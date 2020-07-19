#ifndef FC3D_OCTREE_H_INCLUDED
#define FC3D_OCTREE_H_INCLUDED

#include <stdlib.h>

#include <WF3D/Rendering/Design/image3d.h>

#include <FC3D/Rendering/Octree/octree_node.h>

typedef struct
{
    fc3d_DataPool* octree_children_data_pool;
    fc3d_DataPool* octree_auxiliary_data_pool;

    float half_size;

    int max_depth;

    fc3d_rendering_octree_node* node_0;

} fc3d_RenderingOctree;

//Create an octree
fc3d_RenderingOctree* fc3d_RenderingOctree_Create(float half_size, int max_depth, size_t children_data_pool_block_len, size_t auxiliary_data_pool_block_len);

//Destroy an octree
void fc3d_RenderingOctree_Destroy(fc3d_RenderingOctree* octree);

//Rewind an octree
//Return a pointer to node_0
//Cannot fail
fc3d_rendering_octree_node* fc3d_RenderingOctree_Rewind(fc3d_RenderingOctree* octree);

//Add an object into the octree
fc3d_rendering_octree_node* fc3d_RenderingOctree_AddObject(fc3d_RenderingOctree* octree, fc3d_rendering_object* obj, bool spatial_extension);

//Rasterization
wf3d_error fc3d_RenderingOctree_Rasterization(fc3d_RenderingOctree* octree, wf3d_image2d_rectangle* img_out, wf3d_rasterization_env const* env, owl_v3f32 octree_v_pos, owl_q32 octree_q_rot);

//Rasterization with multithreading
wf3d_error fc3d_RenderingOctree_MultiThreadRasterization(fc3d_RenderingOctree* octree, wf3d_image2d_rectangle* img_out, unsigned short nb_threads, wf3d_rasterization_env const* env, owl_v3f32 octree_v_pos, owl_q32 octree_q_rot);

//Multithread ray tracing
//img3d_thread_buffer_list must contain nb_threads elements of the same size (width and height)
wf3d_error fc3d_RenderingOctree_MultiThreadRayTracing(fc3d_RenderingOctree* octree, wf3d_image2d_rectangle* img_out, wf3d_Image3d** img3d_thread_buffer_list, unsigned short nb_threads, wf3d_lightsource const* const* lightsource_list, unsigned int nb_lightsources, owl_v3f32 octree_v_pos, owl_q32 octree_q_rot, wf3d_camera3d const* cam);

#endif // FC3D_OCTREE_H_INCLUDED
