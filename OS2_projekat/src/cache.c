#include "cache.h"
#include"buddy.h"
#include"stdlib.h"
#include"slabstruct.h"
#include<stdbool.h>
#include<Windows.h>
#include"slab.h"
#include<math.h>

HANDLE globmtx = NULL;
HANDLE Buffglobmtx = NULL;
struct slab;
int napravljeni = 0;
int unisti = 0;

struct kmem_cache_s {
	struct kmem_cache_s *next;
	struct slab * free;
	struct slab * ful;
	struct slab * haveSpace;
	int numOfSlots;
	int sizeOfSlot;
	void(*ctor)(void *);
	void(*dctor)(void *);
	int errorType;
	char*opis;
	char*ime;
	int intfragm;
	int tmpOff;
	bool vrsenaAlokacija;
	bool smanjeno;
	HANDLE mutex;
	
};
struct kmem_cache_s *instanca_kesa = NULL;
struct kmem_cache_s *instanca_buff_kesa = NULL;
bool isCache(struct kmem_cache_s *cache) {
	struct kmem_cache_s *pom = instanca_kesa;
	while (pom != NULL) {
		if (pom == cache)return true;
		pom = pom->next;
	}
	pom = instanca_buff_kesa;
	while (pom != NULL) {
		if (pom == cache)return true;
		pom = pom->next;
	}

	return false;
}
void * cache_alloc(struct kmem_cache_s *cache) {
	if (isCache(cache) == false)return 0;
	WaitForSingleObject(cache->mutex, INFINITE);
	napravljeni++;
	void *ret = NULL;
	if (cache->haveSpace) {

		 ret =getObjSpace(cache->haveSpace);
		if (cache->haveSpace->tmpObj==cache->haveSpace->slotNum) {
			struct slab* s = cache->haveSpace;
			cache->haveSpace = s->next;
			s->next = cache->ful;
			cache->ful = s;
		}
	}
	else if (cache->free) {
		ret = getObjSpace(cache->free);
		struct slab* s = cache->free;
		cache->free = s->next;
		if (cache->haveSpace->tmpObj != cache->haveSpace->slotNum) {
			s->next = cache->haveSpace;
			cache->haveSpace = s;
		}
		else {

			s->next = cache->ful;
			cache->ful = s;

		}
	}
	else {
		
		struct slab* slb = create_Slab(cache->numOfSlots, cache->sizeOfSlot, cache->tmpOff);
		
		if(slb==NULL){
			cache->errorType = NoMemory;
			ReleaseMutex(cache->mutex);
			return NULL;
		}
		if (cache->smanjeno)cache->vrsenaAlokacija = true;
		cache->tmpOff += CACHE_L1_LINE_SIZE;
		if (cache->tmpOff >= cache->intfragm)cache->tmpOff = 0;
		ret = getObjSpace(slb);

		if (slb->tmpObj != slb->slotNum) {
			slb->next = cache->haveSpace;
			cache->haveSpace = slb;
		}
		else {

			slb->next = cache->ful;
			cache->ful = slb;

		}

	}
	if (ret == NULL)	cache->errorType = NoMemory;
	else {
		if (cache->ctor)
		cache->ctor(ret);
	}
	ReleaseMutex(cache->mutex);
	return ret;
}
void info_cache(struct kmem_cache_s *cache) { 
	if (isCache(cache) == false)
	{
		printf("Ovo nije kes\n");
		return ;
	}
	WaitForSingleObject(cache->mutex, INFINITE);
	printf("\n%s\n", cache->ime);
	printf("Velicina podatka:%dB\n", cache->sizeOfSlot);
	printf("Broj podatka:%d obj/slab\n", cache->numOfSlots);
	int slots_occupied = 0;
	int total_slots = 0;
	struct slab* slb;
	slb = cache->ful;
	int numOfSlabs = 0;
	int sizeOfSlab = 0;
	while (slb) {
		numOfSlabs++;
		sizeOfSlab = slb->size;
	slots_occupied+=slb->tmpObj;
	total_slots += slb->slotNum;
	slb = slb->next;
	}


	slb = cache->haveSpace;
	while (slb) {
		sizeOfSlab = slb->size;
		numOfSlabs++;
		slots_occupied += slb->tmpObj;
		total_slots += slb->slotNum;
		slb = slb->next;
	}

	slb =cache->free; 
	while (slb) {
		sizeOfSlab = slb->size;
		numOfSlabs++;
		total_slots += slb->slotNum;
		slb = slb->next;
	}
	printf("Velicina slaba(u blokovima) : %d\n", sizeOfSlab/BLOCK_SIZE);
	printf("Broj slabova : %d\n", numOfSlabs);
	if (total_slots != 0) {
		double procenat = (double)(slots_occupied) / (double)(total_slots);
		procenat *= 100;
		printf("Procenat iskoriscenosti je %lf\%\n\n", procenat);
	}
	else
		printf("kes nema slotove \n\n",slb);
	ReleaseMutex(cache->mutex);

}
void error_cache(struct kmem_cache_s *cache) {
	if (isCache(cache) == false)
	{
		printf("Ovo nije kes\n");
		return;
	}
	switch (cache->errorType) {
	case 1:
		printf("Nema memorije za slab\n");
		break;
	case 2:
		printf("Los pokazivac\n");
		break;
	case 3:
		printf("Nije moguce izvrsiti shrink\n");
		break;

	default: 
		printf("Nema greske");
}
}

struct kmem_cache_s *create(const char *name, size_t size, void(*ctor)(void *), void(*dtor)(void *),int tip) {
	if (globmtx == NULL) {
		globmtx = CreateMutex(NULL, FALSE, NULL);
		Buffglobmtx = CreateMutex(NULL, FALSE, NULL);
	}

	struct kmem_cache_s* kes=buddy_alloc(sizeof(struct kmem_cache_s));
	if (kes == NULL) {
		printf("Nema memorije za kes\n");
		return kes;
	}
	kes->ctor = ctor;
	kes->sizeOfSlot = size;
	kes->ime = name;
	kes->free = NULL;
	kes->errorType = 0;
	kes->haveSpace = NULL;

	kes->ful = NULL;
	kes->vrsenaAlokacija = false;
	kes->smanjeno = false;
	kes->tmpOff = 0;


	
	kes->numOfSlots = find_optimal_slotnum(size);
	  kes->intfragm = (1 << logbaza2(kes->numOfSlots*(size)+ceil(1.0*kes->numOfSlots/8.0)+sizeof(struct slab)))- (kes->numOfSlots*(size)+ceil(1.0*kes->numOfSlots / 8.0) + sizeof(struct slab));
//	kes->intfragm = (1 << logbaza2(kes->numOfSlots*((size<4?4:size)+1) + sizeof(struct slab))) - (kes->numOfSlots*((size < 4 ? 4 : size) +1)+ + sizeof(struct slab));
	//printf("Interna fregmentacija je %d\n", kes->intfragm);//dopuna od verzije
	//kes.int
	if (tip == 1) {
		kes->next = instanca_kesa;
		instanca_kesa = kes;
	}
	else {
		kes->next = instanca_buff_kesa;
		instanca_buff_kesa = kes;
	}
//	kes->mutex = CreateMutex(NULL, FALSE, NULL);
	kes->mutex = globmtx;
	return kes;
}
int cache_shrink(struct kmem_cache_s *cachep) {
	if (isCache(cachep) == false)
	{
		printf("Ovo nije kes\n");
		return;
	}
	WaitForSingleObject(cachep->mutex, INFINITE);
	if (cachep == NULL)exit(3);

	if (cachep->vrsenaAlokacija && cachep->smanjeno)cachep->errorType = Shrinking_Avoided;
	else {
		while (cachep->free) {
			cachep->smanjeno = true;
			slab_dealloc(cachep->free);
			cachep->free = cachep->free->next;
		}
	}
	ReleaseMutex(cachep->mutex);
}// Shrink cache

int cache_free(struct kmem_cache_s *cachep, void *objp) {
	if (isCache(cachep) == false)
	{
		printf("Ovo nije kes\n");
		return -1;
	}
	if (objp == NULL) {
		printf("Objekat je NULL\n");
		cachep->errorType = InvalidPointer;
		return -1;
	}
	
	WaitForSingleObject(cachep->mutex, INFINITE);
	unisti++;
	struct slab* b = cachep->ful;
	struct slab* prev = NULL;
	while (b != NULL) {
		if (((int*)objp > (int*)b) && (int*)objp < ((char*)b + b->size))break;
		prev = b;
		b = b->next;
	}

	if (b) {
		returnObjSpace(b, objp);
		if (prev) {
			prev->next = b->next;
		}
		else cachep->ful = b->next;
		if (b->tmpObj == 0) {
			b->next = cachep->free;
			cachep->free=b;

		}
		else {
			b->next = cachep->haveSpace;
			cachep->haveSpace = b;
		}
		ReleaseMutex(cachep->mutex);
		return 0;
	}
	b = cachep->haveSpace;
	prev = NULL;
	while (b != NULL) {
		if (((int*)objp > (int*)b) && (int*)objp < ((char*)b + b->size))break;
		prev = b;
		b = b->next;
	}

	if (b) {
		if (b->tmpObj == 0) {
			if (prev) {
				prev->next = b->next;
			}
			else cachep->haveSpace = b->next;

			b->next = cachep->free;
			cachep->free = b;

		}
		ReleaseMutex(cachep->mutex);
		return 0;
	}

	ReleaseMutex(cachep->mutex);
	cachep->errorType = InvalidPointer;
	return -1;
}
void *buffer_alloc(size_t size){
	WaitForSingleObject(Buffglobmtx, INFINITE);
//	size = 1 << logbaza2(size);
	struct kmem_cache_s* kesevi = instanca_buff_kesa;
	while (kesevi != NULL) {
		if (kesevi->sizeOfSlot == size)break;

		kesevi = kesevi->next;
	}

	if (kesevi) {
		ReleaseMutex(Buffglobmtx);
		return cache_alloc(kesevi);

	}
	else {

		struct kmem_cache_s* kes=create("size-"  , size, NULL, NULL,2);
		ReleaseMutex(Buffglobmtx);
		return cache_alloc(kes);
	}

}
void buffer_free(const void *objp){
	WaitForSingleObject(Buffglobmtx, INFINITE);
	struct kmem_cache_s* kesevi = instanca_buff_kesa;
	while (kesevi != NULL) {
		info_cache(kesevi);
		if (cache_free(kesevi, objp) == 0) {
		
			return;
		}
		kesevi = kesevi->next;
	}

	ReleaseMutex(Buffglobmtx);
	
}
void cache_destroy(struct kmem_cache_s *cachep) {
	if (isCache(cachep) == false)
	{
		printf("Ovo nije kes\n");
		return -1;
	}
	
	WaitForSingleObject(cachep->mutex, INFINITE);
	while (cachep->free) {

		slab_dealloc(cachep->free);
		cachep->free = cachep->free->next;
	}

	while (cachep->ful) {

		slab_dealloc(cachep->ful);
		cachep->ful = cachep->ful->next;
	}

	while (cachep->haveSpace) {

		slab_dealloc(cachep->haveSpace);
		cachep->haveSpace = cachep->haveSpace->next;
	}
	struct kmem_cache_s* pom = instanca_kesa;
	struct kmem_cache_s* prev = NULL;
	while (pom != NULL) {
		if (pom == cachep)break;
		prev = pom;
		pom = pom->next;
	}
	if (pom) {
		if (prev) {
			prev->next = pom->next;
		}
		else
			instanca_kesa = pom->next;
	}
	else
	{
		pom = instanca_buff_kesa;
		prev = NULL;
		while (pom != NULL) {
			if (pom == cachep)break;
			prev = pom;
			pom = pom->next;
		}
		if (pom) {
			if (prev) {
				prev->next = pom->next;
			}
			else
				instanca_kesa = pom->next;
		}

	}
	
	buddy_dealloc(cachep, sizeof(struct kmem_cache_s));
	
	ReleaseMutex(cachep->mutex);
}