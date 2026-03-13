#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>

#define DEVICE_NAME "sctrt_dev"  // Nome device in /dev/

static int Major; // Major number assigned to device driver
static struct class *sctrt_class = NULL;
static struct device *sctrt_device = NULL;

static struct file_operations fops = {
  .owner = THIS_MODULE,
  .unlocked_ioctl = sc_throttle_ioctl
};

int sctrt_dev_init(void) {
    Major = register_chrdev(0, DEVICE_NAME, &fops);

	if (Major < 0) {
	  printk("Registering device failed\n");
	  return Major;
	}

	/* Creazione automatica del device node /dev/sctrt_dev */
	sctrt_class = class_create(DEVICE_NAME);
	if (IS_ERR(sctrt_class)) {
		unregister_chrdev(Major, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class\n");
		return PTR_ERR(sctrt_class);
	}

	sctrt_device = device_create(sctrt_class, NULL, MKDEV(Major, 0), NULL, DEVICE_NAME);
	if (IS_ERR(sctrt_device)) {
		class_destroy(sctrt_class);
		unregister_chrdev(Major, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(sctrt_device);
	}

	printk(KERN_INFO "Device registered, it is assigned major number %d\n", Major);

	return 0;
}


void sctrt_dev_cleanup(void){
	device_destroy(sctrt_class, MKDEV(Major, 0));
	class_destroy(sctrt_class);
	unregister_chrdev(Major, DEVICE_NAME);

	printk(KERN_INFO "Broadcast device unregistered, it was assigned major number %d\n", Major);
}