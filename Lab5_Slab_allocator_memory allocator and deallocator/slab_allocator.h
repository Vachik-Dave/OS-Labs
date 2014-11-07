#include <stdio.h>
#include <stdlib.h>

#include "dll.h"

# define no_classes 43
// CHUNK structure strores class ID it belong & pointer to free memory
typedef struct chunk
{
	int cls_ID;
	void * value_ptr;
}Chunk;
// Slab_allocator structure
typedef struct slab_allocator
{
	DLL* memory_pool;					// memory pool containing number of slabs as nodes
	DLL* classes[no_classes];				// classes as linked list of slabs with specific chunk size
	int class_chunk_size[no_classes];			// Chunk size of a specific class
}Slab_Allocator;

// Memory allocator function
void * alloc_mem(struct slab_allocator* alloc, int num_bytes);

// Memory freeing function
int free_mem(struct slab_allocator* alloc, void* ptr);

//Initialization of the slab allocator
void init_slab_allocator(struct slab_allocator* alloc, size_t mem_pool_size);

// Assign a specific slab to given class
int slab_to_class(DLL**,DLL**,int);

// Find suitable class to assign chunk to user
int find_class(struct slab_allocator*, int,int*);


