#pragma once
struct buddy_allocator_s;
 struct buddy_allocator_s* instanca;

 int logbaza2(int);
void buddy_init(void*memory, int bloks); 
void *buddy_alloc(int size);
void * buddy_merge(void* target, int size);
void buddy_dealloc(void *ptr, int size);
