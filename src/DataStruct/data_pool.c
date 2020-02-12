#include <FC3D/DataStruct/data_pool.h>

//Create a DataPool
//
//
fc3d_DataPool* fc3d_DataPool_Create(size_t data_block_len, size_t data_size)
{
    fc3d_DataPool* data_pool = malloc(sizeof(*data_pool));

    if(data_pool != NULL)
    {
        data_pool->data_block_len = data_block_len;
        data_pool->data_size = data_size;
        data_pool->current_block_ind = 0;

        data_pool->storage = fc3d_LinkedList_Create(data_size * data_block_len);

        if(data_pool->storage == NULL)
        {
            fc3d_DataPool_Destroy(data_pool);
            data_pool = NULL;
        }
    }

    return data_pool;
}

//Destroy a DataPool
//
//
void fc3d_DataPool_Destroy(fc3d_DataPool* data_pool)
{
    if(data_pool != NULL)
    {
        fc3d_LinkedList_Destroy(data_pool->storage);
        free(data_pool);
    }
}

//Rewind a DataPool
//Return a pointer to the first data
//
void* fc3d_DataPool_Rewind(fc3d_DataPool* data_pool)
{
    data_pool->current_block_ind = 0;
    return fc3d_LinkedList_Rewind(data_pool->storage);
}

//Get the current data
//
//
void* fc3d_DataPool_GetCurrentData(fc3d_DataPool* data_pool)
{
    void* current_data_block = fc3d_LinkedList_GetCurrentElement(data_pool->storage);
    return (void*)( (uintptr_t)(data_pool->current_block_ind * data_pool->data_size) + (uintptr_t)current_data_block );
}

//Next data
//
//
void* fc3d_DataPool_NextData(fc3d_DataPool* data_pool)
{
    void* next_data = fc3d_LinkedList_NextElement(data_pool->storage, true);

    if(next_data != NULL)
    {
        data_pool->current_block_ind = 0;
    }

    return next_data;
}

