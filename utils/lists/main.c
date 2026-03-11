#include <stdio.h>
#include <stdbool.h>
#include "list.h"

struct node_t *list;

int visit(const char *str, bool first){
	return printf("%s%s", !first? "," : "", str);
}

int main(void){
	struct node_t *curr, *one, *two;
	
	init_list(&list);
	curr = add_after(list, "First string");
	curr = add_after(curr, "Second string");
	one = curr = add_after(curr, "Third string");
	curr = add_after(curr, "Fourth string");
	two = curr = add_after(curr, "Fifth string");
	curr = add_after(curr, "Sisxth string");
	
	list_foreach(list, visit); puts("");
	remove_entry(&list, one);
	remove_entry(&list, two);
	list_foreach(list, visit); puts("");
	
	fini_list(&list);
}