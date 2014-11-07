#include<stdio.h>
#include<stdlib.h>

#include "dll.h"

// Doubly linked-list -----------------------------------------------------------------------------------

// ---------------------------------------Insertion at the end-------------------------------------------
// It takes arguments as reference of head of the list and reference of the memory address
	//It returns reference to new head
DLL* insert_end(DLL** head,void * ptr)
{
	DLL *n;
	n = (DLL*)ptr;
	int size = sizeof(n);			// any pointer size for current machine & compiler
	n->value_ptr = ptr+(2*size);		// after two poiters next & prev
	n->next = NULL;
	// if empty list start with head
	if((*head) == NULL)
	{
		n->prev =NULL;
		(*head) = n;
		return (*head);
	}
	DLL *temp;
	temp = (*head);
	// check if second
	if((*head)->next == NULL)
	{
		(*head)->next = n;
		n->prev = (*head);
		return (*head);
	}
	// travel till end
	while(temp->next != NULL)
	{
		temp = temp->next;
	}
	temp->next = n;
	n->prev = temp;
	return (*head);

}
// ---------------------------------------Insertion at front----------------------------------------------
// It takes arguments as reference of head of the list and reference of the memory address
	//It returns reference to new head
DLL* insert_front(DLL* *head,void * ptr){
	DLL *n;
	n = (DLL*)ptr;
	int size = sizeof(n);			// any pointer size for current machine & compiler
	n->value_ptr = ptr+(2*size);		// after two poiters next & prev
	n->prev = NULL;
	// check if empty list
	if ((*head) == NULL){
		n->next = NULL;
		(*head) = n;		
		return (*head);
	}
	n->next = (*head);
	(*head)->prev = n;
	(*head) = n;
	return (*head);
}
// ---------------------------------------Insertion at front----------------------------------------------
// It takes arguments as reference of head of the list 
	//It returns reference of the memory address
void* delete_front(DLL* *head)
{
	void * return_ptr;
	DLL *temp;
	// check if empty list
	if((*head) == NULL)
	{
		//perror("empty, cannot delete");
		return NULL;
	}
	return_ptr = (void*)(*head);
	// check for single node
	if ((*head)->next == NULL){
		//free((*head));	
		(*head) = NULL;
		return return_ptr;
	}
	
	temp = (*head)->next;
	//free((*head));
	(*head) = NULL;
	temp->prev = NULL;
	(*head) = temp;		
	return return_ptr;
}





