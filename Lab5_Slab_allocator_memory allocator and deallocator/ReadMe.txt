Here, I have provided four file to provide my memory allocator.

dll.h & dll.c :- doubly linked list used to implement slab allocator functions
slab_allocator.h & slab_allocator.c :- memomry allocator files provids basic allocation and freeing functionality.

This four file are required to work with this memory allocator functionalities.

I also provide additional test.c file , which I used for testing my allocator functionality.

--------------------------

I've commented line no: 148, 183, 213 from slab_allocator.c file.
These lines are printing the class_id of the chunk being allocated or freeing.
