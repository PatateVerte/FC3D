#ifndef FC3D_OCTREE_H_INCLUDED
#define FC3D_OCTREE_H_INCLUDED

#include <stdlib.h>

#include <FC3D/Rendering/Octree/octree_node.h>

typedef struct
{
    fc3d_DataPool* octree_children_data_pool;
    fc3d_DataPool* octree_auxiliary_data_pool;

    wf3d_vect3d center;
    float half_size;

    fc3d_rendering_octree_node* node_0;

} fc3d_RenderingOctree;

//Create an octree
fc3d_RenderingOctree* fc3d_RenderingOctree_Create(wf3d_vect3d center, float half_size, size_t children_data_pool_block_len, size_t auxiliary_data_pool_block_len);

//Destroy an octree
void fc3d_RenderingOctree_Destroy(fc3d_RenderingOctree* octree);

//Rewind an octree
//Return a pointer to node_0
//Cannot fail
fc3d_RenderingOctree* fc3d_RenderingOctree_Rewind(fc3d_RenderingOctree* octree);


#endif // FC3D_OCTREE_H_INCLUDED
