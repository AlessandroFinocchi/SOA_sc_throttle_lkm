#ifndef _SCTRT_CORE_H
#define _SCTRT_CORE_H

#include <linux/types.h>

bool sctrt_check_throttling_compatibility(struct pt_regs *regs);

#endif