#include"slab.h"
#include"cache.h"
#include"buddy.h"

void kmem_init(void *space, int block_num) { buddy_init(space, block_num); }

kmem_cache_t *kmem_cache_create(const char *name, size_t size, void(*ctor)(void *), void(*dtor)(void *)) { create(name, size, ctor, dtor,1); }
int kmem_cache_shrink(kmem_cache_t *cachep) {
	return cache_shrink(cachep);
}
void *kmem_cache_alloc(kmem_cache_t *cachep) { return cache_alloc(cachep); }
void kmem_cache_free(kmem_cache_t *cachep, void *objp) { cache_free(cachep, objp); }
void *kmalloc(size_t size) { return buffer_alloc(size); }
void kfree(const void *objp) { buffer_free(objp); }
void kmem_cache_destroy(kmem_cache_t *cachep) { cache_destroy(cachep); }
void kmem_cache_info(kmem_cache_t *cachep) { info_cache(cachep); }
int kmem_cache_error(kmem_cache_t *cachep) { error_cache(cachep); }