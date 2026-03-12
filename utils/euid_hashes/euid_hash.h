#pragma once
#include <linux/types.h>
#include <linux/uidgid.h>

int euid_hash_add(kuid_t euid);

bool euid_hash_lookup(kuid_t euid);

void euid_hash_del(kuid_t euid);

void euid_hash_cleanup(void);
