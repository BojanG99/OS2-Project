#pragma once

/*struct slab {

	void *objectspace;
	struct slab* next;
	void*free;
	void*obj;
	int slotSize;
	int size;
	int slotNum;
	int tmpObj;
};*/  //V1
struct slab {
	void *objectspace;
	struct slab* next;
	//void*free;
	//void*obj;
	int slotSize;
	int size;
	int slotNum;
	int tmpObj;
	unsigned char* maske;
};  //V2
int find_optimal_slotnum(int slotSize);
struct slab* create_Slab(int slotNumber, int slotSize, int offset);
void* getObjSpace(struct slab* slab);
void slab_dealloc(struct slab* slab);
void* returnObjSpace(struct slab* slab, void* space);