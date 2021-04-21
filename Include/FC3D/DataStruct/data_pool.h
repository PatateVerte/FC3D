#ifndef FC3D_DATA_POOL_H_INCLUDED
#define FC3D_DATA_POOL_H_INCLUDED

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <FC3D/fc3d.h>

#include <FC3D/DataStruct/linked_list.h>

typedef struct
{
    //Fixed data
    size_t data_block_len;
    size_t data_size;

    //Data storage
    fc3d_LinkedList* storage;

    //Current state
    size_t current_block_ind;

} fc3d_DataPool;

//Create a DataPool
//data_block_len must be at least 1
FC3D_DLL_EXPORT fc3d_DataPool* fc3d_DataPool_Create(size_t data_block_len, size_t data_size);

//Destroy a DataPool
FC3D_DLL_EXPORT void fc3d_DataPool_Destroy(fc3d_DataPool* data_pool);

//Rewind a DataPool
//Return a pointer to the first data
//Cannot fail
FC3D_DLL_EXPORT void* fc3d_DataPool_Rewind(fc3d_DataPool* data_pool);

//Get the current data
FC3D_DLL_EXPORT void* fc3d_DataPool_GetCurrentData(fc3d_DataPool* data_pool);

//Next data
FC3D_DLL_EXPORT void* fc3d_DataPool_NextData(fc3d_DataPool* data_pool);

#endif // FC3D_DATA_POOL_H_INCLUDED
