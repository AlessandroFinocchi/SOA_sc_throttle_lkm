#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "list.h"

struct node_t{
	struct node_t *next;
	unsigned char payload[];
};

void remove_entry(struct node_t **head, struct node_t *const entry){
	while((*head) != entry){
		head = &(*head) -> next;
	}
	*head = entry -> next;
	free(entry);
}

struct node_t *add_after(struct node_t *prev, char *str){
	size_t len = strlen(str) + 1;
	struct node_t *node = malloc(sizeof(*node) + len);
	
	if(node == NULL)//se la malloc da errore: programmazione difensiva
		return NULL;
	
	memcpy(node -> payload, str, len);
	node -> next = prev -> next;
	prev -> next = node;
	return node;
}

void init_list(struct node_t ** const head){
	*head = malloc(sizeof(**head) + 1);
	(*head) -> payload[0] = '\0';
	(*head) -> next = NULL;
}

void fini_list(struct node_t **head){
	while(*head)
		remove_entry(head, *head);
}

void list_foreach(struct node_t *head, visitor_t eval){
	struct node_t *curr = head -> next; //iteriamo su tutti gli elementi tranne il primo nodo
	bool first = true;
	
	while(curr){
		eval((char *)curr -> payload, first);
		curr = curr -> next;
		first &= false;
	}
}