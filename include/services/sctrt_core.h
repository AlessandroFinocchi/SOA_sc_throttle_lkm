#ifndef _SCTRT_CORE_H
#define _SCTRT_CORE_H

#include <linux/types.h>

void sctrt_core_init(void);
void sctrt_core_exit(void);

bool sctrt_check_throttling_compatibility(struct pt_regs *regs);
int sctrt_wait_on_weq(void);
void sctrt_wake_up_weq(void);

#endif