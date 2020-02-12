#include <FC3D/DataStruct/data_pool.h>

//Create a DataPoolBlock
//
//
fc3d_DataPoolBlock* fc3d_DataPoolBlock_Create(size_t data_size)
{
    fc3d_DataPoolBlock* data_block = malloc(sizeof(*data_block));

    if(data_block != NULL)
    {
        data_block->data = malloc(data_size);
        data_block->data_size = data_size;

        //If an error occured
        if(data_block->data == NULL && data_size != 0)
        {
            fc3d_DataPoolBlock_Destroy(data_block);
            data_block = NULL;
        }
    }

    return data_block;
}

//Destroy a DataPoolBlock
//
//
void fc3d_DataPoolBlock_Destroy(fc3d_DataPoolBlock* data_block)
{
    if(data_block != NULL)
    {
        free(data_block->data);
        free(data_block);
    }
}

//Destroy recursively an DataPoolBlock
//
//
void fc3d_DataPoolBlock_DestroyRecursively(fc3d_DataPoolBlock* data_block)
{
    if(data_block != NULL)
    {
        fc3d_DataPoolBlock* next_data_block = data_block->next_data_block;
        fc3d_DataPoolBlock_Destroy(data_block);
        fc3d_DataPoolBlock_DestroyRecursively(next_data_block);
    }
}

//Get the next DataPoolBlock (Create one if there is none)
//
//
fc3d_DataPoolBlock* fc3d_DataPoolBlock_GetNext(fc3d_DataPoolBlock* data_block)
{
    if(data_block->next_data_block != NULL)
    {
        return data_block->next_data_block;
    }
    else
    {
        data_block->next_data_block = fc3d_DataPoolBlock_Create(data_block->data_size);
        return data_block->next_data_block;
    }
}
