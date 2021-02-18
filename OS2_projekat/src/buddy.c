#include"buddy.h"
#include"slab.h"
#include<stdio.h>
#include"stdbool.h"
#include<Windows.h>

struct buddy_allocator_s {
	void* mem;
	int size;
	void * free_space[26];
	bool initialized;
	HANDLE mutex;
};


int logbaza2(int a) {
	for (int i = 0; i < 25; i++)
	{
		if (a <= (1 << i))return i;
	}
	return 25;
}

struct buddy_allocator_s* instanca = NULL;
void buddy_init(void*memory, int bloks) {//popunjava listu buddy-ija
	if (instanca && instanca->initialized) {
		printf("Allocator is initialize\n");
		return;
	}
	instanca = (struct buddy_allocator_s*)memory;//inicijalizujemo 1 instancu buddy_allocator
	instanca->mem = (char*)memory + sizeof(struct buddy_allocator_s);
	memory = (char*)memory + sizeof(struct buddy_allocator_s);
	instanca->size = bloks;
	instanca->initialized = true;
	long long size = bloks * 4096 - sizeof(struct buddy_allocator_s);
	int a = 0;
	for (int i = 25; i >= 0; i--)
	{
		if ((1 << i) <= size) {
			instanca->free_space[i] = memory;
			*((int*)(memory)) = 0;
			size -= (1 << i);

			memory = ((char*)memory + (1 << i));


		}
		else {
			instanca->free_space[i] = NULL;
			//	printf("i%d\n", i);
		}


	}
	instanca->mutex = CreateMutex(NULL, FALSE, NULL);


}
void *buddy_alloc(int size) {
	WaitForSingleObject(instanca->mutex,INFINITE);
	//printf("Uzeto%d\n", size);
	int power = logbaza2(size);
	void *ret = NULL;
	int a = power;//broj ulaza za listu
	if (instanca->free_space[a]) {//ako imamo neku memoriju te velicine rezervisemo nju
		ret = instanca->free_space[a];

		instanca->free_space[a] = *((int *)(instanca->free_space[a]));

	}
	else {

		int b = a + 1;

		while (b < 26) {//trazimo memoriju vece velicine
			if (instanca->free_space[b])break;
			b++;

		}
		if (b == 26) {
			printf("Nema vise memorije");
			ReleaseMutex(instanca->mutex);
			return NULL;
		}
		ret = instanca->free_space[b];

		instanca->free_space[b] = *((int *)(instanca->free_space[b]));


		while (b > a) {
			instanca->free_space[b - 1] = (void *)((char *)(ret)+(1 << (b - 1)));
			*(int*)(instanca->free_space[b - 1]) = 0;

			b--;
		}


	}
	ReleaseMutex(instanca->mutex);
	return ret;
}
void * buddy_merge(void* target, int size) {//trazi parnjaka sa velicinom adresom target i size ulazom u tabeli. Ako nadje parnjaka vraca njegovu adresu ako ne onda NULL
	void * prev = NULL;
	void *ret = instanca->free_space[size];

	while (true) {//prolazi kroz sve buddy-je u size ulazu 
		if (ret == target) {//ako nadje prevezuje listu i izbacuje ga
			if (prev) {
				*(int *)prev = *(int*)ret;

			}
			else {

				instanca->free_space[size] = *((int *)(instanca->free_space[size]));


			}
			return ret;
		}

		else {
			prev = ret;


			ret = *(int*)ret;//prelazi na sledeci u listi
			if (ret == NULL)return NULL;//nije nasao
		}
	}

}
void buddy_dealloc(void *ptr, int size) {
	//WaitForSingleObject(instanca->mutex, INFINITE);
	double power = logbaza2(size);
	//printf("vraceno%d\n", size);
	void *ret = NULL;
	int a = power;//nalazenje ulaza u listi
	if (instanca->free_space[a] == NULL) {//ako je prazna lista samo ubacimo
		instanca->free_space[a] = ptr;
		*(int*)ptr = 0;
//		ReleaseMutex(instanca->mutex);
		return;

	}
	size = 1 << a;//velicina bloka
	int broj = ((char*)ptr - (instanca->mem));//broj je offset od pocetka memorije
	broj /= size;//redni broj bloka velicine size u buddy memoriji
	if (broj % 2 == 0) {//ako je on paran onda je njegov parnjak za size adresa veci
		if (buddy_merge((char*)instanca->mem + (broj + 1)*size, a))buddy_dealloc(ptr, size * 2);//ako nadje parnjaka onda se spajaju i traze tog parnjaka
		else {//ako ne nadjemo onda stavljamo na pocetak liste i stavljamo da pokazuje na prosli pocetak liste
			*(int*)ptr = instanca->free_space[a];
			instanca->free_space[a] = ptr;
		}
	}
	else {//ako je neparan onda je on za size adresa veci
		if (ret = buddy_merge((char*)instanca->mem + (broj - 1)*size, a))buddy_dealloc(ret, size * 2);//ako nadje parnjaka onda se spajaju i traze tog parnjaka
		else {//ako ne nadjemo onda stavljamo na pocetak liste i stavljamo da pokazuje na prosli pocetak liste
			*(int*)ptr = instanca->free_space[a];
			instanca->free_space[a] = ptr;
		}
	}


//	ReleaseMutex(instanca->mutex);
}

int maxFreeDegree() {
//	WaitForSingleObject(instanca->mutex, INFINITE);
	for (int i = 25; i >= 12; i--)
	{
		if (instanca->free_space[i])return i - 12;
	}
	return -1;
//	ReleaseMutex(instanca->mutex);
}