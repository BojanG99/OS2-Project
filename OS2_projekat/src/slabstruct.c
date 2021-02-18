#include"slab.h"
#include"buddy.h"
#include"slabstruct.h"
#include<stdbool.h>
//#include<math.h>
/*
int find_optimal_slotnum(int slotSize) {
	int slotSizeReal=slotSize;
	int optsn = 0;
	double d = 100.0;
	if (slotSize < 4)slotSize = 4;
	for (int i = 0; i<maxFreeDegree(); i++)
	{
		int size = (1 << i)*BLOCK_SIZE;
		int numOfSlots =( size - sizeof(struct slab))/(slotSize+1);
		int waist = (size - numOfSlots * (slotSize+1) - sizeof(struct slab));
		double procenat = (double)(waist+ sizeof(struct slab)) / (double)size;
		procenat *= 100;
		if (procenat < 4)return numOfSlots;
		int b = 2;
		if (procenat < d)
		{
			optsn = numOfSlots;
			d = procenat;
		}
	}

	return optsn;
}


struct slab* create_Slab(int slotNum, int slotSize,int offset) {
	int realSlotSize = slotSize;
	if (slotSize <= 4)slotSize =4;
	int reqSpace = slotNum * (slotSize+1 ) + sizeof(struct slab);

	struct slab* ret=buddy_alloc(reqSpace);
	if (ret == NULL)return ret;
	
	ret->slotSize = slotSize;
	ret->objectspace = (char*)ret + (sizeof(struct slab) + offset);
	ret->free = ret->objectspace;
	ret->next = NULL;
	ret->slotNum = slotNum;
	ret->tmpObj = 0;
	ret->obj = NULL;
	ret->size = 1 << (logbaza2(reqSpace));
	
	void* ptr = ret->free;
	for (int i = 0; i < slotNum-1; i++)
	{
		*((int*)((char*)ptr+1)) = (char*)ptr + (slotSize + 1);
		*(bool*)(ptr) =true;
		ptr = (char*)ptr + (slotSize+1);
	}

	*((int*)((char*)ptr+1)) = 0;

	return ret;
}
void* getObjSpace(struct slab* slab) {
	if (slab == NULL)return NULL;
	if (slab->tmpObj == slab->slotNum)return NULL;
	void *ret = (char*)(slab->free) + 1;
	 
	
	*(bool*)(slab->free) = false;

		slab->free = *((int*)((char*)(slab->free)+1));
	slab->tmpObj++;

	return ret;

}
void* returnObjSpace(struct slab* slab, void* space) {
	int dif = (char*)space - slab->objectspace-1;
	if ((dif % (slab->slotSize+1)) == 0 && (*(bool*)((char*)space-1))==false) {
		slab->tmpObj--;
		*(int*)space = slab->free;

		slab->free = ((char*)space-1);
		*(bool*)(slab->free) = true;
		return space;
	}
	else {
		printf("objekat je vec vracen ili je losa adresa");
	}
	return NULL;
}

void slab_dealloc(struct slab* slab) {
	printf("Obrisan slab\n");
	buddy_dealloc(slab,slab->size);

}

/*int maisn() {
	buddy_init(malloc(1025 * 4096), 1025 );
	for (int i = 1;i < 8;i++) {
		//find_optimal_slotnumV2(i);
	}
	
void* obj = getObjSpace(slab);

	*(int*)obj = 100;
	*((int*)obj + 2) = 200;

	returnObjSpace(slab, obj);
	returnObjSpace(slab, obj);
	//obj = getObjSpace(slab);
	printf("%d", *((int*)obj + 2));
	system("pause");
	return 0;

}*/