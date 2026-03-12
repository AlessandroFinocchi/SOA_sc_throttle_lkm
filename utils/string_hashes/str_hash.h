#pragma once
#include <linux/types.h>

int str_hash_add(char *str);

bool str_hash_lookup(char *str);

void str_hash_del(char *str);

void str_hash_cleanup(void);
