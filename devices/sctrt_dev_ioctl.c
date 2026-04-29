#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cred.h>
#include <linux/uidgid.h> /* Necessario per uid_eq e GLOBAL_ROOT_UID */

#include "sctrt.h"
#include "sctrt_state.h"
#include "sctrt_profiler.h"
#include "sctrt_dev_ioctl.h"


long sctrt_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    struct sc_throttle_param param;
    long ret = 0;

    /* Se il comando è una scrittura */
    if (_IOC_DIR(cmd) & _IOC_WRITE) {
        /* 
         * Verifica formale dell'Effective User ID. 
         * Garantisce che solo l'utente root globale possa procedere.
         */
        if (!uid_eq(current_euid(), GLOBAL_ROOT_UID))
            return -EPERM;
        
        if (copy_from_user(&param, (struct sc_throttle_param __user *)arg, sizeof(param))) 
            return -EFAULT;
    }

    /* Se il comando è una lettura*/
    if ((_IOC_DIR(cmd) & _IOC_READ)) 
        memset(&param, 0, sizeof(param)); // Pulizia preventiva

    /* Dispatching comandi */
    switch (cmd) {
        /* =================== Write =================== */
        case SC_THROTTLE_SET_STATE: // Monitor on/off
            if(param.data.new_state == true)
                ret = sctrt_monitor_enable();
            else
                sctrt_monitor_disable();
            break;

        case SC_THROTTLE_SET_RATE: // Configurazione soglia MAX
            sctrt_set_max(param.data.max_rate);
            break;

        case SC_THROTTLE_REG_SYS: // Aggiunta syscall
            ret = sctrt_syscall_register(param.data.syscall_num);
            break;

        case SC_THROTTLE_DEREG_SYS: // Rimozione syscall
            ret = sctrt_syscall_deregister(param.data.syscall_num);
            break;

        case SC_THROTTLE_REG_UID: // Aggiunta nuovo EUID
            ret = sctrt_euid_register(param.data.uid);
            break;

        case SC_THROTTLE_DEREG_UID: // Rimozione di un EUID
            ret = sctrt_euid_deregister(param.data.uid);
            break;

        case SC_THROTTLE_REG_PROG: // Aggiunta nuovo programma
            param.data.prog_name[MAX_PROG_NAME_LEN - 1] = '\0';
            ret = sctrt_prog_register(param.data.prog_name);
            break;

        case SC_THROTTLE_DEREG_PROG: // Rimozione di un programma
            param.data.prog_name[MAX_PROG_NAME_LEN - 1] = '\0';
            ret = sctrt_prog_deregister(param.data.prog_name);
            break;
        
        /* =================== Read =================== */
        case SC_THROTTLE_GET_METRICS: // Estrazione statistiche
            sctrt_profiler_get(&param);
            break;

        /* =================== Debug =================== */
        case SC_THROTTLE_PRINT_STATE: // Log dello stato
            sctrt_print_state();
            break;

        case SC_THROTTLE_PRINT_RATE: // Log della soglia MAX
            sctrt_print_max();
            break;

        case SC_THROTTLE_PRINT_SYSCS: // Log delle syscalls
            sctrt_print_syscalls();
            break;

        case SC_THROTTLE_PRINT_USERS: // Log degli EUID
            sctrt_print_users();
            break;

        case SC_THROTTLE_PRINT_PROGS: // Log dei programmi
            sctrt_print_programs();
            break;

        default: // Comando non riconosciuto
            return -ENOTTY;
    }

    /* Se il comando è una lettura*/
    if ((_IOC_DIR(cmd) & _IOC_READ) && ret == 0) {
        if (copy_to_user((struct sc_throttle_param __user *)arg, &param, sizeof(param))) {
            return -EFAULT;
        }
    }

    return ret;
}