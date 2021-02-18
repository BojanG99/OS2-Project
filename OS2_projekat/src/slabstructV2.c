#include"slab.h"
#include<math.h>
#include"slabstruct.h"

int find_optimal_slotnum(int slotSize) {

	int slotSizeReal = slotSize;
	int optsn = 0;
	double d = 100.0;

	for (int i = 0; i <= maxFreeDegree(); i++)
	{
		int size = (1 << i)*BLOCK_SIZE;
		int numOfSlots = (8 * (size - sizeof(struct slab))) / (8 * slotSize + 1);
		int waist = (size - numOfSlots * (slotSize)-numOfSlots / 8 - sizeof(struct slab));
		double procenat = (double)(waist + sizeof(struct slab)) / (double)size;
		procenat *= 100;
		if (procenat < 10)return numOfSlots;
		int b = 2;
		if (procenat < d)
		{
			optsn = numOfSlots;
			d = procenat;
		}
	}

	return optsn;
}



struct slab* create_Slab(int slotNumber, int slotSize, int offset) {

	
	int reqSpace = slotNumber * (slotSize )+(int)ceil(1.0*slotNumber/8.0) + sizeof(struct slab);

	struct slab* ret = buddy_alloc(reqSpace);
	if (ret == NULL)return ret;

	ret->slotSize = slotSize;
	ret->objectspace = (char*)ret + (sizeof(struct slab) + offset)+ (int)ceil(1.0*slotNumber / 8.0);

	ret->next = NULL;
	ret->slotNum = slotNumber;
	ret->tmpObj = 0;
	
	ret->size = 1 << (logbaza2(reqSpace));


	ret->maske = (char*)ret + (sizeof(struct slab) + offset);
	for (int i = 0; i < (int)ceil(1.0*slotNumber / 8.0); i++)
	{
		ret->maske[i] = 0;
	}
	return ret;
}
void* getObjSpace(struct slab* slab1) {
	if (slab1->tmpObj == slab1->slotNum)return NULL;
	int j = 0;
	unsigned char c = slab1->maske[0];
	while(c==255){
		
		c = slab1->maske[++j];
	}
	int i;
	for ( i = 0; i < 8; i++)
	{
		if ((c&(1 << i))==0)break;
	}
	int numOfObj = j * 8 + i;
	c = c | (1 << i);
	slab1->maske[j] = c;
	slab1->tmpObj++;

	return ((char*)slab1->objectspace + numOfObj*slab1->slotSize );
}
void slab_dealloc(struct slab* slab) {

	buddy_dealloc(slab, slab->size);

}
void* returnObjSpace(struct slab* slab, void* space) {
	int numOfObj = (char*)slab->objectspace - (char*)space;
	if ((numOfObj % (slab->slotSize) != 0)||((numOfObj/slab->slotSize)>(slab->slotNum-1)))return NULL;
	numOfObj /= slab->slotSize;
	int entry = numOfObj / 8;
	int div = numOfObj % 8;
	if (slab->maske[entry] & (1 << div)) {
		slab->maske[entry] & (~(1 << div));
	}
	slab->tmpObj--;
}