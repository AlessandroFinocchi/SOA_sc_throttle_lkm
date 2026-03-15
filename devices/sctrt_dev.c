#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/cdev.h>

#include "sctrt_dev.h"
#include "sctrt_dev_ioctl.h"

#define DEVICE_NAME  "sctrt_dev"
#define DEVICE_CLASS "sctrt_dev_cls"

struct sctrt_cdev {
    dev_t           dev;        // Major number
    struct cdev     cdev;		// Character device
    struct class   *class;		// Device class
};

static struct sctrt_cdev device;

static struct file_operations fops = {
  .owner = THIS_MODULE,
  .unlocked_ioctl = sctrt_dev_ioctl
};

int sctrt_dev_init(void) {
    int status;
#ifdef STATIC_DEVNR
	status = register_chrdev_region(device.dev, 1, DEVICE_NAME);
#else
	status = alloc_chrdev_region(&device.dev, 0, 1, DEVICE_NAME);
#endif
	if (status) {
		pr_err("%s - Error reserving the region of device numbers\n", DEVICE_NAME);
		return status;
	}

	cdev_init(&device.cdev, &fops);
	device.cdev.owner = THIS_MODULE;

	if ((status = cdev_add(&device.cdev, device.dev, 1))) {
		pr_err("%s - Error adding cdev\n", DEVICE_NAME);
		goto free_dev;
	}

	pr_info("%s - Registered a character device for Major %d starting with Minor %d\n", DEVICE_NAME, MAJOR(device.dev), MINOR(device.dev));

	if (!(device.class = class_create(DEVICE_CLASS))) {
		pr_err("%s - Could not create class %s\n", DEVICE_NAME, DEVICE_CLASS);
		status = ENOMEM;
		goto delete_cdev;
	}

	if (!(device_create(device.class, NULL, device.dev, NULL, "%s%d", DEVICE_NAME, 0))) {
		pr_err("%s - Could not create device %s%d\n", DEVICE_NAME, 0);
		status = ENOMEM;
		goto delete_class;
	}

	pr_info("%s - Created device\n", DEVICE_NAME);

	return 0;

delete_class:
	class_unregister(device.class);
	class_destroy(device.class);
delete_cdev:
	cdev_del(&device.cdev);
free_dev:
	unregister_chrdev_region(device.dev, 1);
	return status;
}


void sctrt_dev_cleanup(void){
	device_destroy(device.class, device.dev);
	class_unregister(device.class);
	class_destroy(device.class);
	cdev_del(&device.cdev);
	unregister_chrdev_region(device.dev, 1);
}