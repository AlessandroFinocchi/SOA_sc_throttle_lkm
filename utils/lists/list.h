#pragma once
#include <stdbool.h>
struct node_t;

typedef int (*visitor_t)(const char *str, bool first);

void remove_entry(struct node_t **head, struct node_t *const entry);

struct node_t *add_after(struct node_t *prev, char *str);

void init_list(struct node_t ** const head);

void fini_list(struct node_t ** head);

void list_foreach(struct node_t *head, visitor_t eval);