#include <FC3D/Rendering/Octree/octree_data_pool.h>

//Create an OctreeDataPiece and notifies the previous and the next
//
//
fc3d_OctreeDataPiece* fc3d_OctreeDataPiece_Create(fc3d_OctreeDataPiece* previous_data_piece, fc3d_OctreeDataPiece* next_data_piece)
{
    fc3d_OctreeDataPiece* data_piece = malloc(sizeof(*data_piece));

    if(data_piece != NULL)
    {
        data_piece->previous_data_piece = previous_data_piece;
        data_piece->next_data_piece = next_data_piece;

        if(previous_data_piece != NULL)
        {
            previous_data_piece->next_data_piece = data_piece;
        }
        if(next_data_piece != NULL)
        {
            next_data_piece->previous_data_piece = data_piece;
        }
    }

    return data_piece;
}

//Destroy an OctreeDataPiece and stick the next and previous together
//
//
void fc3d_OctreeDataPiece_Destroy(fc3d_OctreeDataPiece* data_piece)
{
    if(data_piece != NULL)
    {
        if(data_piece->previous_data_piece != NULL)
        {
            data_piece->previous_data_piece->next_data_piece = data_piece->next_data_piece;
        }
        if(data_piece->next_data_piece != NULL)
        {
            data_piece->next_data_piece->previous_data_piece = data_piece->previous_data_piece;
        }

        free(data_piece);
    }
}

//Destroy an OctreeDataPiece and all the following
//
//
void fc3d_OctreeDataPiece_DestroyRecursively(fc3d_OctreeDataPiece* data_piece)
{
    if(data_piece != NULL)
    {
        fc3d_OctreeDataPiece* next_data_piece = data_piece->next_data_piece;
        fc3d_OctreeDataPiece_Destroy(data_piece);
        fc3d_OctreeDataPiece_DestroyRecursively(next_data_piece);
    }
}

//Get the next OctreeDataPiece (Return NULL if there is none)
//
//
fc3d_OctreeDataPiece* fc3d_OctreeDataPiece_GetNext(fc3d_OctreeDataPiece* data_piece)
{
    return data_piece->next_data_piece;
}

//Create an OctreeDataPool
//
//
fc3d_OctreeDataPool* fc3d_OctreeDataPool_Create()
{
    fc3d_OctreeDataPool* data_pool = malloc(sizeof(*data_pool));

    if(data_pool != NULL)
    {
        fc3d_OctreeDataPiece* first_data_piece = fc3d_OctreeDataPiece_Create(NULL, NULL);
        data_pool->first_data_piece = first_data_piece;
        data_pool->free_data_piece = first_data_piece;

        if(first_data_piece == NULL)
        {
            free(data_pool);
            data_pool = NULL;
        }
    }

    return data_pool;
}

//Rewind an OctreeDataPiece
//
//
fc3d_OctreeDataPool* fc3d_OctreeDataPool_Rewind(fc3d_OctreeDataPool* data_pool)
{
    data_pool->free_data_piece = data_pool->first_data_piece;
    return data_pool;
}

//Destroy an OctreeDataPool
//
//
void fc3d_OctreeDataPool_Destroy(fc3d_OctreeDataPool* data_pool)
{
    if(data_pool != NULL)
    {
        fc3d_OctreeDataPiece_DestroyRecursively(data_pool->first_data_piece);
        free(data_pool);
    }
}

//Erase all unused data
//
//
fc3d_OctreeDataPool* fc3d_OctreeDataPool_CleanUnused(fc3d_OctreeDataPool* data_pool)
{
    fc3d_OctreeDataPiece_DestroyRecursively( fc3d_OctreeDataPiece_GetNext(data_pool->free_data_piece) );

    return data_pool;
}

//Get a new OctreeDataPiece
//
//
fc3d_OctreeDataPiece* fc3d_OctreeDataPool_GetDataPiece(fc3d_OctreeDataPool* data_pool)
{
    fc3d_OctreeDataPiece* ret_data_piece = data_pool->free_data_piece;

    fc3d_OctreeDataPiece* new_free_data_piece = fc3d_OctreeDataPiece_GetNext(ret_data_piece);
    if(new_free_data_piece == NULL)
    {
        new_free_data_piece = fc3d_OctreeDataPiece_Create(ret_data_piece, NULL);
    }

    //If no error occured
    if(new_free_data_piece != NULL)
    {
        data_pool->free_data_piece = new_free_data_piece;
        return ret_data_piece;
    }
    else
    {
        return NULL;
    }
}

