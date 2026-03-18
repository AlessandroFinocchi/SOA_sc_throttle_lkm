#ifndef SCTRT_DEV_IOCTL_H
#define SCTRT_DEV_IOCTL_H

#include <linux/ioctl.h>
#include <linux/types.h>

#define SC_THROTTLE_MAGIC 'a'
#define MAX_PROG_NAME_LEN 256

/* Enum per la numerazione sequenziale dei comandi */
enum sc_throttle_ops {
    // Write
    OP_SET_MONITOR_STATE = 0,
    OP_SET_MAX_RATE,
    OP_REG_SYSCALL,
    OP_DEREG_SYSCALL,
    OP_REG_UID,
    OP_DEREG_UID,
    OP_REG_PROG,
    OP_DEREG_PROG,

    // Read
    OP_GET_TELEMETRY,

    // Debug
    OP_PRINT_MONITOR_STATE,
    OP_PRINT_MAX_RATE,
    OP_PRINT_SYSCALLS,
    OP_PRINT_USERS,
    OP_PRINT_PROGRAMS
};

/* Struttura unificata per il passaggio dei parametri */
struct sc_throttle_param {
    union {
        bool new_state;                    /* Monitor acceso/spento */
        unsigned int max_rate;             /* Valore MAX di invocazioni al secondo */
        int syscall_num;                   /* Numero della system call x86-64 */
        uid_t uid;                         /* User ID effettivo */
        char prog_name[MAX_PROG_NAME_LEN]; /* Nome dell'eseguibile */
        
        /* Struttura annidata per la telemetria (direzione kernel -> user) */
        struct {
            unsigned long peak_delay_ns;
            char peak_prog_name[MAX_PROG_NAME_LEN];
            uid_t peak_uid;
            unsigned int avg_blocked_threads;
            unsigned int peak_blocked_threads;
        } telemetry;
    } data;
};

/* Definizione formale dei comandi ioctl */
// Write
#define SC_THROTTLE_SET_STATE   _IOW(SC_THROTTLE_MAGIC, OP_SET_MONITOR_STATE, struct sc_throttle_param)
#define SC_THROTTLE_SET_RATE    _IOW(SC_THROTTLE_MAGIC, OP_SET_MAX_RATE,      struct sc_throttle_param)
#define SC_THROTTLE_REG_SYS     _IOW(SC_THROTTLE_MAGIC, OP_REG_SYSCALL,       struct sc_throttle_param)
#define SC_THROTTLE_DEREG_SYS   _IOW(SC_THROTTLE_MAGIC, OP_DEREG_SYSCALL,     struct sc_throttle_param)
#define SC_THROTTLE_REG_UID     _IOW(SC_THROTTLE_MAGIC, OP_REG_UID,           struct sc_throttle_param)
#define SC_THROTTLE_DEREG_UID   _IOW(SC_THROTTLE_MAGIC, OP_DEREG_UID,         struct sc_throttle_param)
#define SC_THROTTLE_REG_PROG    _IOW(SC_THROTTLE_MAGIC, OP_REG_PROG,          struct sc_throttle_param)
#define SC_THROTTLE_DEREG_PROG  _IOW(SC_THROTTLE_MAGIC, OP_DEREG_PROG,        struct sc_throttle_param)

// Read
#define SC_THROTTLE_GET_TELEM   _IOR(SC_THROTTLE_MAGIC, OP_GET_TELEMETRY,     struct sc_throttle_param)

// Debug
#define SC_THROTTLE_PRINT_STATE _IO(SC_THROTTLE_MAGIC, OP_PRINT_MONITOR_STATE)
#define SC_THROTTLE_PRINT_RATE  _IO(SC_THROTTLE_MAGIC, OP_PRINT_MAX_RATE)
#define SC_THROTTLE_PRINT_SYSCS _IO(SC_THROTTLE_MAGIC, OP_PRINT_SYSCALLS)
#define SC_THROTTLE_PRINT_USERS _IO(SC_THROTTLE_MAGIC, OP_PRINT_USERS)
#define SC_THROTTLE_PRINT_PROGS _IO(SC_THROTTLE_MAGIC, OP_PRINT_PROGRAMS)

long sctrt_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

#endif