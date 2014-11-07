#include<stdio.h>
#include<stdlib.h>

typedef struct linkedlist
{
	struct linkedlist *next;
	struct linkedlist *prev;
	void* value_ptr;
}DLL;


DLL* insert_front(DLL**,void *);
DLL* insert_end(DLL**,void * );
void* delete_front(DLL**);

