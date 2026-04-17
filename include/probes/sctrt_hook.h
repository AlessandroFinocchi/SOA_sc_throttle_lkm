#ifndef SCTRT_HOOK_H
#define SCTRT_HOOK_H
    
extern struct kprobe** kprobe_ctx_offset;

int sctrt_hook_init(void);
void sctrt_hook_exit(void);

#endif