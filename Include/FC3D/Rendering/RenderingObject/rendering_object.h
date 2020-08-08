#ifndef FC3D_RENDERING_OBJECT_H_INCLUDED
#define FC3D_RENDERING_OBJECT_H_INCLUDED

#include <OWL/Optimized3d/quaternion/q32.h>
#include <OWL/Optimized3d/vector/v3f32.h>

#include <WF3D/Rendering/camera3d.h>
#include <WF3D/Rendering/Shapes/rasterization_attr.h>

#include <FC3D/Rendering/Image/image3d.h>
#include <FC3D/Rendering/Image/depth_image.h>

typedef struct
{
    bool (*NearestIntersectionWithRay)(void const* obj, owl_v3f32 v_pos, owl_q32 q_rot, owl_v3f32 ray_origin, owl_v3f32 ray_dir, float t_min, float t_max, float* t_ret, owl_v3f32* normal_ret, wf3d_surface const** surface_ret, wf3d_color* diffusion_color_ret);

    void (*Rasterization)(void const* obj, fc3d_Image3d* img3d, wf3d_rasterization_rectangle const* rect, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam);

    void (*DepthRasterization)(void const* obj, fc3d_DepthImage* depth_img, wf3d_rasterization_rectangle const* rect, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam);

    float (*Radius)(void const* obj);

    float (*InfRadiusWithTransform)(void const* obj, owl_v3f32 v_pos, owl_q32 q_rot);

} fc3d_rendering_object_interface;

typedef struct
{
    owl_v3f32 v_pos;
    owl_q32 q_rot;

    void* obj;
    fc3d_rendering_object_interface const* rendering_obj_interface;

} fc3d_rendering_object;

#endif // FC3D_RENDERING_OBJECT_H_INCLUDED
