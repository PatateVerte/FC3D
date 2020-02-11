#ifndef FC3D_OCTREE_DATA_POOL_H_INCLUDED
#define FC3D_OCTREE_DATA_POOL_H_INCLUDED

#include <FC3D/Rendering/rendering_node.h>

#define FC3D_OCTREE_DATA_PIECE_LEN 8

typedef struct fc3d_OctreeDataPiece fc3d_OctreeDataPiece;
struct fc3d_OctreeDataPiece
{
    //Place in the memory pool
    fc3d_OctreeDataPiece* previous_data_piece;
    fc3d_OctreeDataPiece* next_data_piece;

    //Place as a container
    size_t nb_max_owned_objects;
    size_t nb_virtual_objects;
    fc3d_OctreeDataPiece* virtual_next;
    fc3d_RenderingNode* nodes[FC3D_OCTREE_DATA_PIECE_LEN];

};

//Create an OctreeDataPiece and notifies the previous and the next
fc3d_OctreeDataPiece* fc3d_OctreeDataPiece_Create(fc3d_OctreeDataPiece* previous_data_piece, fc3d_OctreeDataPiece* next_data_piece);

//Destroy an OctreeDataPiece and stick the next and previous together
void fc3d_OctreeDataPiece_Destroy(fc3d_OctreeDataPiece* data_piece);

//Destroy an OctreeDataPiece and all the following
void fc3d_OctreeDataPiece_DestroyRecursively(fc3d_OctreeDataPiece* data_piece);

//Get the next OctreeDataPiece (Return NULL if there is none)
fc3d_OctreeDataPiece* fc3d_OctreeDataPiece_GetNext(fc3d_OctreeDataPiece* data_piece);

//Access node i
fc3d_RenderingNode* fc3d_OctreeDataPiece_GetNode(fc3d_OctreeDataPiece* data_piece, size_t i);

typedef struct
{
    fc3d_OctreeDataPiece* first_data_piece;
    fc3d_OctreeDataPiece* free_data_piece;

} fc3d_OctreeDataPool;

//Create an OctreeDataPool
fc3d_OctreeDataPool* fc3d_OctreeDataPool_Create();

//Rewind an OctreeDataPiece
fc3d_OctreeDataPool* fc3d_OctreeDataPool_Rewind(fc3d_OctreeDataPool* data_pool);

//Destroy an OctreeDataPool
void fc3d_OctreeDataPool_Destroy(fc3d_OctreeDataPool* data_pool);

//Erase all unused data
fc3d_OctreeDataPool* fc3d_OctreeDataPool_CleanUnused(fc3d_OctreeDataPool* data_pool);

//Get a new OctreeDataPiece
fc3d_OctreeDataPiece* fc3d_OctreeDataPool_GetDataPiece(fc3d_OctreeDataPool* data_pool);

#endif // FC3D_OCTREE_DATA_POOL_H_INCLUDED
