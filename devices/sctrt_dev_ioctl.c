#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cred.h>

#include "sctrt_dev_ioctl.h"

static long sc_throttle_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    struct sc_throttle_param param;
    long ret = 0;

    // Controllo dei privilegi root
    if (current_euid().val != 0) {
        return -EPERM;
    }

    /* * Se il comando prevede il passaggio di parametri da user a kernel (Scrittura),
     * copiamo in modo sicuro la struttura nello spazio kernel */
    if (_IOC_DIR(cmd) & _IOC_WRITE) {
        if (copy_from_user(&param, (struct sc_throttle_param __user *)arg, sizeof(param))) 
            return -EFAULT;
    }

    /* Dispatching comandi */
    switch (cmd) {
        case SC_THROTTLE_SET_STATE: // Accensione/spegnimento monitor
            // set_monitor_active(param.data.state);
            break;

        case SC_THROTTLE_SET_RATE: // Configurazione soglia MAX
            // set_max_syscall_rate(param.data.max_rate);
            break;

        case SC_THROTTLE_REG_SYS: // Aggiunta syscall
            // register_syscall(param.data.syscall_num);
            break;

        case SC_THROTTLE_DEREG_SYS: // Rimozione syscall
            // deregister_syscall(param.data.syscall_num);
            break;

        case SC_THROTTLE_REG_UID: // Aggiunta nuovo EUID
            // register_uid(param.data.uid);
            break;

        case SC_THROTTLE_DEREG_UID: // Rimozione di un EUID
            // deregister_uid(param.data.uid);
            break;

        case SC_THROTTLE_REG_PROG: // Aggiunta nuovo programma
            param.data.prog_name[MAX_PROG_NAME_LEN - 1] = '\0';
            // register_prog_name(param.data.prog_name);
            break;

        case SC_THROTTLE_DEREG_PROG: // Rimozione di un programma
            param.data.prog_name[MAX_PROG_NAME_LEN - 1] = '\0';
            // deregister_prog_name(param.data.prog_name);
            break;

        case SC_THROTTLE_GET_TELEM: // Estrazione statistiche
            memset(&param, 0, sizeof(param)); // Pulizia preventiva
            // get_current_telemetry(&param.data.telemetry);
            break;

        default: // Comando non riconosciuto
            return -ENOTTY;
    }

    /* * Se il comando prevede di restituire dati allo user-space (Lettura),
     * copiamo la struttura popolata indietro.
     */
    if ((_IOC_DIR(cmd) & _IOC_READ) && ret == 0) {
        if (copy_to_user((struct sc_throttle_param __user *)arg, &param, sizeof(param))) {
            return -EFAULT;
        }
    }

    return ret;
}