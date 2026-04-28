#ifndef _SCTRT_PROFILER_H
#define _SCTRT_PROFILER_H

#include <linux/types.h>

#include "sctrt_dev_ioctl.h"

void sctrt_profiler_init(void);
void sctrt_profiler_reset(void);

/* Interfacce per il tracciamento del ciclo di vita dei thread bloccati */
void sctrt_profiler_thread_sleep(void);
void sctrt_profiler_thread_wakeup(ktime_t start_time);

/* Campionamento periodico invocato dal LAPIC timer */
void sctrt_profiler_sample_window(void);

/* Estrazione sicura dei dati verso l'user-space */
void sctrt_profiler_get(struct sc_throttle_param *param);

#endif