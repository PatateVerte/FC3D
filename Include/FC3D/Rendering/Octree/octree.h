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

    int max_depth;

    fc3d_rendering_octree_node* node_0;

} fc3d_RenderingOctree;

//Create an octree
fc3d_RenderingOctree* fc3d_RenderingOctree_Create(wf3d_vect3d center, float half_size, int max_depth, size_t children_data_pool_block_len, size_t auxiliary_data_pool_block_len);

//Destroy an octree
void fc3d_RenderingOctree_Destroy(fc3d_RenderingOctree* octree);

//Rewind an octree
//Return a pointer to node_0
//Cannot fail
fc3d_rendering_octree_node* fc3d_RenderingOctree_Rewind(fc3d_RenderingOctree* octree);

//Add an object into the octree
fc3d_rendering_octree_node* fc3d_RenderingOctree_AddObject(fc3d_RenderingOctree* octree, fc3d_rendering_object* obj, bool spatial_extension);

//Rasterization
fc3d_error fc3d_RenderingOctree_Rasterization(fc3d_RenderingOctree* octree, wf3d_img_gen_interface* img_out, float* depth_buffer, wf3d_vect3d cam_v_pos, wf3d_quat cam_q_rot, wf3d_camera3d const* cam);

#endif // FC3D_OCTREE_H_INCLUDED
