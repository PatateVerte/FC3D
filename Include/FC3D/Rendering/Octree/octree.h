#ifndef FC3D_OCTREE_H_INCLUDED
#define FC3D_OCTREE_H_INCLUDED

#include <stdlib.h>

#include <WF3D/Rendering/Design/color.h>

#include <FC3D/Rendering/Image/image2d.h>
#include <FC3D/Rendering/Image/image3d.h>
#include <FC3D/Rendering/Octree/octree_node.h>
#include <FC3D/Rendering/Lightsource/lightsource.h>

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


//
//
typedef struct
{
    owl_v3f32 octree_v_pos;
    owl_q32 octree_q_rot;

    fc3d_lightsource const* const* lightsource_list;
    unsigned int nb_lightsources;

    float near_clipping_distance;

} fc3d_octree_point_color_attr;
//
wf3d_color OWL_VECTORCALL fc3d_RenderingOctree_PointColor(fc3d_RenderingOctree const* octree, fc3d_octree_point_color_attr const* attr, owl_v3f32 cam_ray_dir, owl_v3f32 v_pos, owl_v3f32 normal, wf3d_surface const* surface, wf3d_color diffusion_color, int max_nb_reflections);

//Ray tracing
wf3d_error fc3d_RenderingOctree_MultiThreadRayTracing(fc3d_RenderingOctree* octree, fc3d_image2d_rectangle* img_out, fc3d_Image3d** img3d_thread_buffer_list, unsigned short nb_threads, fc3d_lightsource const* const* lightsource_list, unsigned int nb_lightsources, owl_v3f32 octree_v_pos, owl_q32 octree_q_rot, wf3d_camera3d const* cam);

//Depth rasterization
void fc3d_RenderingOctree_DepthRasterization(void const* obj, fc3d_DepthImage* depth_img, wf3d_rasterization_rectangle const* rect, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam);


#endif // FC3D_OCTREE_H_INCLUDED
