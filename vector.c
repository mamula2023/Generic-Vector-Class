#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

void VectorNew(vector *v, int elemSize, VectorFreeFunction freeFn, int initialAllocation)
{
    assert(elemSize > 0);
    if(initialAllocation == 0)
        initialAllocation= 16;
    v->elementSize = elemSize;
    v->logicalSize = 0;
    v->physicalSize = initialAllocation;
    v->sizeIncrement = initialAllocation;
    v->firstElement = malloc(initialAllocation*elemSize);
    v->freeFunction = freeFn;
}

void VectorDispose(vector *v)
{
    if(v->freeFunction!=NULL){
        for(int i = 0; i<v->physicalSize; i++){   
            v->freeFunction(v->firstElement + i*v->elementSize);
        }
    }
    free(v->firstElement);
}

int VectorLength(const vector *v)
{
     return v->logicalSize; 
}

void *VectorNth(const vector *v, int position)
{ 
    assert(position >= 0 && position < VectorLength(v));
    void *result = (void*)(v->firstElement + v->elementSize * position);
    return result;
}   

void VectorReplace(vector *v, const void *elemAddr, int position)
{
    assert(position >= 0 && position < VectorLength(v));
    if(v->freeFunction!=NULL)
        v->freeFunction(v->firstElement+position*v->elementSize);
    memcpy(v->firstElement+position*v->elementSize, elemAddr, v->elementSize);

}

void VectorInsert(vector *v, const void *elemAddr, int position)
{
    assert(position >= 0 && position <= VectorLength(v));
   
    if(v->logicalSize == v->physicalSize){
        expandMemoryForVector(v);
    }

    for(int i = v->logicalSize-1; i>=position; i--){
        void* source = v->firstElement + i*v->elementSize;
        memcpy(source+v->elementSize, source, v->elementSize);        
    }

    memcpy(v->firstElement+position*v->elementSize, elemAddr, v->elementSize);

    v->logicalSize++;
}

void VectorAppend(vector *v, const void *elemAddr)
{

    if(v->logicalSize == v->physicalSize)
        expandMemoryForVector(v);
    
    void* tempToBeWritten = v->firstElement + v->logicalSize*v->elementSize;
    memcpy(tempToBeWritten, elemAddr, v->elementSize);
    
    v->logicalSize++;   
}

void VectorDelete(vector *v, int position)
{
    assert(position >= 0 && position < VectorLength(v));
  
    for(int i = position; i<VectorLength(v)-1; i++){
        if(v->freeFunction!=NULL){
            v->freeFunction(v->firstElement+i*v->elementSize);
        }
        memcpy(v->firstElement+i*v->elementSize, v->firstElement+(i+1)*v->elementSize, v->elementSize);
    }
    
    v->logicalSize--;
    if(VectorLength(v) == 0){
        if(v->freeFunction != NULL)
            v->freeFunction(v->firstElement);
    }else{
        if(v->freeFunction!=NULL)
            v->freeFunction(v->firstElement+VectorLength(v)*v->elementSize);
    }
}

void VectorSort(vector *v, VectorCompareFunction compare)
{
    assert(compare!=NULL);
    qsort(v->firstElement, VectorLength(v), v->elementSize, compare);
}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData)
{
    assert(mapFn!=NULL);
    for(int i = 0;i<VectorLength(v); i++){
        mapFn(v->firstElement+i*v->elementSize, auxData);
    }
}
static const int kNotFound = -1;
int performLinearSearch(const vector *v,const void *key, VectorCompareFunction searchFn, int startIndex){
    for(int i = startIndex; i<VectorLength(v); i++){
        if(searchFn(key, v->firstElement + i*v->elementSize)==0)
            return i;
    }
    return kNotFound;
}

int performBinarySearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex){
    int left = startIndex;
    int right = VectorLength(v)-1;
    int mid = 0;

    while(left <= right){
        mid = (left+right)/2;
        void* midPointer = v->firstElement+mid*v->elementSize;
        if(searchFn(midPointer, key) == 0) return mid;
        if(searchFn(midPointer, key) < 0) left = mid+1;
        if(searchFn(midPointer, key) > 0) right = mid-1;
    }
    return kNotFound;
}


int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted)
{
    int i = 0;
    if(VectorLength(v) == 0)
        return kNotFound;
    assert(startIndex >= 0 && startIndex < VectorLength(v));
    assert(searchFn != NULL && key != NULL);
    if(!isSorted){
        return performLinearSearch(v, key, searchFn, startIndex);
    }else
        return performBinarySearch(v, key, searchFn, startIndex);
    return kNotFound; 
} 




void expandMemoryForVector(vector *v){
    v->physicalSize *= 2;
    
    void *temp;
    temp = malloc(v->physicalSize*v->elementSize);
    memcpy(temp,  v->firstElement, v->elementSize * VectorLength(v));
    if(v->freeFunction != NULL){
        for(int i = 0; i<VectorLength(v); i++){       
                v->freeFunction(v->firstElement + i*v->elementSize);   
        }
    }else{
        free(v->firstElement);
    }

    v->firstElement = temp;
}