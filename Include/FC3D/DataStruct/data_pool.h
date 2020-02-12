#ifndef FC3D_DATA_POOL_H_INCLUDED
#define FC3D_DATA_POOL_H_INCLUDED

#include <stdlib.h>

typedef struct fc3d_DataPoolBlock fc3d_DataPoolBlock;
struct fc3d_DataPoolBlock
{
    fc3d_DataPoolBlock* next_data_block;

    size_t data_size;
    void* data;
};

//Create a DataPoolBlock
fc3d_DataPoolBlock* fc3d_DataPoolBlock_Create(size_t data_size);

//Destroy a DataPoolBlock
void fc3d_DataPoolBlock_Destroy(fc3d_DataPoolBlock* data_block);

//Destroy recursively an DataPoolBlock
void fc3d_DataPoolBlock_DestroyRecursively(fc3d_DataPoolBlock* data_block);

//Get the next DataPoolBlock (Create one if there is none)
fc3d_DataPoolBlock* fc3d_DataPoolBlock_GetNext(fc3d_DataPoolBlock* data_block);

#endif // FC3D_DATA_POOL_H_INCLUDED
