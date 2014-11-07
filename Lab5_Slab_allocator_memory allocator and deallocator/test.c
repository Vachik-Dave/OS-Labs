#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "slab_allocator.h"
#define M 10000
#define POOL_SIZE 128

typedef struct linked_list ll;

struct linked_list{
	ll *fwptr;
	ll *bwptr;
	int value[M];
};

ll* insert_at_front(Slab_Allocator* alloc,ll* head,int i)
{
	ll *n;
	n = (ll*)alloc_mem(alloc,sizeof(ll));
	if(n == NULL)
	{	perror("error");
		return head;
	}
	size_t try = (size_t) n % 8;
	if(try != 0)
	{	perror("-------------------------not divisible by 8");
		return head;
	}
	n->value[0] = i;
	n->value[M-1] = i*i;
	n->bwptr = NULL;		
	if (head == NULL){
		n->fwptr = NULL;
		head = n;		
		return head;
	}
	n->fwptr = head;
	head->bwptr = n;
	head = n;
	return head;
}

ll* delete_from_front(Slab_Allocator* alloc,ll* head)
{
	int r;
	ll *temp;
	if(head == NULL)
	{
		printf("empty, cannot delete");
		return head;
	}
	if (head->fwptr == NULL){
		r = free_mem(alloc,head);	
		if(r != 0)
			perror("freeing error");
		head = NULL;
		return head;
	}
	
	temp = head->fwptr;
	r = free_mem(alloc,head);
	if(r != 0)
		perror("freeing error");
	temp->bwptr = NULL;
	head = temp;		
	return head;
}

int traverse(ll* head){

	//printf("hi");	
	ll *t;
	t = head;
	if(head == NULL)
		printf("empty \n");
	while(t != NULL){
		printf("\n %d-%d---",t->value[0],t->value[M-1]);
		t = t->fwptr;
	}
	return 0;
}
///*
int main()
{
	struct slab_allocator my_alloc;
	int i;
	ll *head = NULL;

	init_slab_allocator(&my_alloc, POOL_SIZE);
	for(i=0;i<=200;i++){
		head = insert_at_front(&my_alloc,head,i);
	}
	//printf("hi");
	traverse(head);
	printf("\n");

	for(i=0;i<=5;i++){
		head = delete_from_front(&my_alloc,head);
	}
	traverse(head);
	printf("\n");

//
/*
	for(i=0;i<=6;i++){
		head = insert_at_front(&my_alloc,head,i);
	}
	//printf("hi");
	traverse(head);

	printf("\n");
	for(i=0;i<=5;i++){
		head = delete_from_front(&my_alloc,head);
	}
	traverse(head);
	printf("\n");

	for(i=0;i<=7;i++){
		head = insert_at_front(&my_alloc,head,i);
	}
	//printf("hi");
	traverse(head);

	printf("\n");
//*/

	return 0;
}
//*

/*

int main()
{
	Slab_Allocator my_alloc;
	init_slab_allocator(&my_alloc, POOL_SIZE);
	int * list;
	int i;
	list = (int *)alloc_mem(&my_alloc, sizeof(int) * 35);
	if(list == NULL)
		perror("error-main");
	for (i=0;i<35;i++)
	{	list[i] = i +1;
		printf("\n--%d-",list[i]);
	}
	free_mem(&my_alloc, list);
	return 0;
}
//*/
