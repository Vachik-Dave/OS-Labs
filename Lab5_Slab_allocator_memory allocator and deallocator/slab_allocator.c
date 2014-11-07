#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "slab_allocator.h"

#define Slab_size 1048576		// 1MB = 1048576 bytes


//########################################################################################################

// Assign a specific slab to given class------------------------------------------------------------------
//It takes reference of memory pool, reference of class and size of chunks to the class.
	// It returns +1 for sucess and -1 in failure
int slab_to_class(DLL** mem_pool,DLL** class,int chunk_size)
{
	int i=0;
	void * ptr = NULL;
	//Slab * new_slab = NULL;
	ptr = delete_front(&(*mem_pool));
	if(ptr == NULL)
		return -1;
	
	size_t location = (size_t)ptr % 8;
	if(location != 0)
		ptr = ptr+location;

	int len = (int)((double)Slab_size / (double)chunk_size);
	
	//new_slab = (Slab*)ptr;
	
	for(i =0; i < len; i++)
	{
		int t = i*chunk_size;
		(*class) = insert_end(&(*class),ptr+t);
	}
	//class = insert_end(class,new_slab);
	return 1;

}
//###############################################################################################################
//Initialization of the slab allocator---------------------------------------------------------------------------
// It takes reference of the slab_allocator structure and memmory pool size as input
void init_slab_allocator(struct slab_allocator* alloc, size_t mem_pool_size)
{
	int i =0;
	void * ptr = NULL;

	// Initialize a Null pointer for head of the memory pool linked list
	alloc->memory_pool = NULL;

	//get memory for memory pool using malloc in group of 1MB
	for(i =0;i<mem_pool_size;i++)
	{
		ptr = (void*)malloc(Slab_size);	
		if(ptr == NULL)
		{	perror("Malloc failed!!!");
			return;
		}	
		alloc->memory_pool = insert_front(&alloc->memory_pool,ptr);
	}
	// Create separate classes with 1 slab each from memory pool
	for(i =0; i < no_classes; i++)
	{	
		//calculate chunk size for the class
		if(i>0)
		{	double t = 1.25 * alloc->class_chunk_size[i-1];
			double tt = t / 8.0 + 0.5;
			int ttt = (int)tt;
			alloc->class_chunk_size[i] = ttt*8;
		}
		else
			alloc->class_chunk_size[i] = 80;

//error cheking:printf("%d -> %d \n-",i,alloc->class_chunk_size[i]);
		
		// Initialize the current class linked-list head 
		alloc->classes[i] = NULL;
		// Allocate 1 slab to the current class
		int r =	slab_to_class(&alloc->memory_pool,&alloc->classes[i],alloc->class_chunk_size[i]);

		if(r < 0)
			perror("Memory_pool empty");
	}
	return;	
}
//##########################################################################################################
// Find suitable class to assign chunk to user--------------------------------------------------------------
// It takes reference of the slab_allocator structure, number of bytes required from the class, reference of 8 multiplier 
	// it returns the id of the suitable size class
int find_class(struct slab_allocator* alloc, int num_bytes, int * multiplier)
{
	int i =0;
	size_t size_int = sizeof(int);
	// find multiplier of 8 for sizeof integer in current machine
	while(size_int > 8 * (*multiplier))
	{	(*multiplier)++;
	}
	// find class
	for(i = 0;i<no_classes;i++)
	{
		if(alloc->class_chunk_size[i] - (8 * (*multiplier)) >= num_bytes)			//fisrt size_int is used as class ID
			return i;
	}
	// if couldnt find suitable class from given range
	return -1;
}
//##########################################################################################################
// Memory allocator function--------------------------------------------------------------------------------
// It takes reference of the slab_allocator structure and number of byte required as input
	// It returns reference of the memory
void * alloc_mem(struct slab_allocator* alloc, int num_bytes)
{
	int multiplier =1;
	void * return_ptr = NULL;
	Chunk * ret_chunk = NULL;
	void * t = NULL;

	int class_id = find_class(alloc,num_bytes,&multiplier);
	// not any suitable class found
	if(class_id == -1)
	{
			perror("Required memory size is too large, cannot allocate:");
			return NULL;
	}
	return_ptr = delete_front(&alloc->classes[class_id]);

	if(return_ptr != NULL)
	{
		ret_chunk = (Chunk*)return_ptr;
		ret_chunk->cls_ID = class_id;
		t = (void*)ret_chunk;
		ret_chunk->value_ptr = t + (multiplier*8);
		ret_chunk->value_ptr = (void*)ret_chunk->value_ptr;

		// Returning address shuld be divisible by 8
		size_t location = (size_t)ret_chunk->value_ptr % 8;
		if(location != 0)
		{
			perror("Here, this chunk not sarting with 8 multiplier: why?");
			if(location <= (8-sizeof(int)))
				ret_chunk->value_ptr = ret_chunk->value_ptr-location;
			else
				ret_chunk->value_ptr = ret_chunk->value_ptr+location;	
		}			

		// Printing class for checking
//		printf("\nclass_assigned -%d",class_id);


		return ret_chunk->value_ptr;
	}
	else
	{
		int r =	slab_to_class(&alloc->memory_pool,&alloc->classes[class_id],alloc->class_chunk_size[class_id]);
		if(r < 0)
		{
			perror("~Memory_pool empty");
			return NULL;
		}
		else
		{
			return_ptr = delete_front(&alloc->classes[class_id]);
			if(return_ptr != NULL)
			{
				ret_chunk = (Chunk*)return_ptr;
				ret_chunk->cls_ID = class_id;
				t = (void*)ret_chunk;
				ret_chunk->value_ptr = t + (multiplier*8);
				ret_chunk->value_ptr = (void*)ret_chunk->value_ptr;
				// Returning address shuld be divisible by 8
				size_t location = (size_t)ret_chunk->value_ptr % 8;
				if(location != 0)
				{
					perror("Here, this chunk not sarting with 8 multiplier: why?");
					if(location <= (8-sizeof(int)))
						ret_chunk->value_ptr = ret_chunk->value_ptr-location;
					else
						ret_chunk->value_ptr = ret_chunk->value_ptr+location;	
				}			

				// Printing class  after new slab is assigned for checking
//				printf("\nNew slab: class_assigned -%d",class_id);
				return ret_chunk->value_ptr;
			}
			else
			{
				perror("Memory_pool empty");
				return NULL;
			}
		}
	}	

}
//##########################################################################################################
// Memory freeing function--------------------------------------------------------------------------------
// It takes reference of the slab_allocator structure and reference of the memory as input
	// It returns 0 on sucess and -1 on failure
int free_mem(struct slab_allocator* alloc, void* ptr)
{
	int multiplier =1,id;
	Chunk * remove_chunk = NULL;
	size_t size_int = sizeof(int);
	while(size_int > 8 * (multiplier))
	{	(multiplier)++;
	}	
	void * p = (void*) ptr;
	p = p - (multiplier*8);
	remove_chunk = (Chunk*)p;
	id = remove_chunk->cls_ID;

	// Printing class for checking
//	printf("\nclass_freed -%d\n",id);
	
	if(id < 0  || id >= 43)
	{
		perror("class id couldnt found!!!");
		return -1;
	}
	p = (void*)remove_chunk;
	alloc->classes[id] = insert_front(&alloc->classes[id],p);
	if(alloc->classes[id] == NULL)
	{
		perror("cannot return chunk to the class!!! ");
		return -1;
	}
	return 0;
}
