#ifndef FC3D_PUNCTUAL_LIGHTSOURCE_H_INCLUDED
#define FC3D_PUNCTUAL_LIGHTSOURCE_H_INCLUDED

#include <FC3D/fc3d.h>

#include <OWL/Optimized3d/vector/v3f32.h>

#include <FC3D/Rendering/Octree/octree.h>
#include <FC3D/Rendering/Lightsource/lightsource.h>
#include <FC3D/Rendering/Image/depth_image.h>

typedef struct
{
    wf3d_color color;

    float bias;
    float cam_near_clipping_distance;

    bool shadow_mapping;
    int map_width;
    int map_height;
    fc3d_DepthImage* depth_face[6];

} fc3d_PunctualLightsource;

//Create a punctual lightsource
//shadow_mapping=false, all the following parameters will be ignored and the lightsource won't allow any shadow mapping
FC3D_DLL_EXPORT fc3d_PunctualLightsource* fc3d_PunctualLightsource_Create(float r, float g, float b, bool shadow_mapping, int map_width, int map_height, float bias, float cam_near_clipping_distance);

//
FC3D_DLL_EXPORT void fc3d_PunctualLightsource_Destroy(fc3d_PunctualLightsource* lightsource);

//
FC3D_DLL_EXPORT fc3d_PunctualLightsource* fc3d_PunctualLightsource_SetIntensity(fc3d_PunctualLightsource* lightsource, float new_r, float new_g, float new_b);

//
FC3D_DLL_EXPORT fc3d_PunctualLightsource* fc3d_PunctualLightsource_UpdateShadowArea(fc3d_PunctualLightsource* lightsource, fc3d_RenderingOctree const* octree, owl_v3f32 octree_v_pos, owl_q32 octree_q_rot, owl_v3f32 area_center, float area_radius);

//v_pos = position of the object relatively to the camera
FC3D_DLL_EXPORT wf3d_color OWL_VECTORCALL fc3d_PunctualLightsource_EnlightSurfacePoint(void* lightsource_obj, wf3d_surface const* surface, wf3d_color diffusion_color, owl_v3f32 v_pos, owl_v3f32 normal, owl_v3f32 vision_ray_dir);

//
FC3D_DLL_EXPORT extern fc3d_lightsource_interface const fc3d_PunctualLightsource_interface;

#endif // FC3D_PUNCTUAL_LIGHTSOURCE_H_INCLUDED
