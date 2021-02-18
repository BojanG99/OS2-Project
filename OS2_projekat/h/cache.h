#pragma once
typedef int size_t ;
//#include"stdbool.h"


#define NoMemory (1)
#define InvalidPointer (2)
#define Shrinking_Avoided (3)
struct kmem_cache_s;
struct kmem_cache_s *instanca_kesa;
struct kmem_cache_s *instanca_buff_kesa;

 void * cache_alloc(struct kmem_cache_s *cache);
 void info_cache(struct kmem_cache_s *cache);
 void error_cache(struct kmem_cache_s *cache);

 struct kmem_cache_s *create(const char *name, size_t size, void(*ctor)(void *), void(*dtor)(void *),int tip); // Allocate cache
 int cache_shrink(struct kmem_cache_s *cachep); // Shrink cache

 int cache_free(struct kmem_cache_s *cachep, void *objp); // Deallocate one object from cache
 void *buffer_alloc(size_t size); // Alloacate one small memory buffer
 void buffer_free(const void *objp); // Deallocate one small memory buffer
 void cache_destroy(struct kmem_cache_s *cachep); // Deallocate cache
