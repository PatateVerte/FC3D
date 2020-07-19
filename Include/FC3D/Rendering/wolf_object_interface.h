#ifndef FC3D_OBJECT_INTERFACE_H_INCLUDED
#define FC3D_OBJECT_INTERFACE_H_INCLUDED

#include <OWL/q32.h>
#include <OWL/v3f32.h>

#include <WF3D/error.h>
#include <WF3D/Rendering/camera3d.h>
#include <WF3D/Rendering/Shapes/rasterization_attr.h>
#include <WF3D/Rendering/lightsource.h>
#include <WF3D/Rendering/Design/image2d.h>
#include <WF3D/Rendering/Design/image3d.h>

typedef struct
{
    //Destroy() parameter
    void (*Destroy)(void*);

    //bool NearestIntersectionWithRay(void const* obj, owl_v3f32 v_pos, owl_q32 q_rot, owl_v3f32 ray_origin, owl_v3f32 ray_dir, float t_min, float t_max, float* t_ret, owl_v3f32* normal_ret, wf3d_surface* surface_ret);
    bool (*NearestIntersectionWithRay)(void const*, owl_v3f32, owl_q32, owl_v3f32, owl_v3f32, float, float, float*, owl_v3f32*, wf3d_surface*);

    //wf3d_error Rasterization(void const* obj, wf3d_image2d_rectangle* img_out, wf3d_rasterization_env const* env, owl_v3f32 v_pos, owl_q32 q_rot)
    wf3d_error (*Rasterization)(void const*, wf3d_image2d_rectangle*, wf3d_rasterization_env const*, owl_v3f32, owl_q32);

    //wf3d_error Rasterization2(void const* obj, wf3d_image3d_image_piece* img_out, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam)
    wf3d_error (*Rasterization2)(void const*, wf3d_image3d_image_piece*, owl_v3f32, owl_q32, wf3d_camera3d const*);

    //float Radius(void const* obj)
    float (*Radius)(void const*);

    //float InfRadius(void const* obj, owl_v3f32 v_pos)
    float (*InfRadius)(void const*, owl_v3f32);

    //float InfRadiusWithRot(void const* obj, owl_v3f32 v_pos, owl_q32 q_rot)
    float (*InfRadiusWithRot)(void const*, owl_v3f32, owl_q32);

} fc3d_wolf_object_interface;

#endif // FC3D_OBJECT_INTERFACE_H_INCLUDED
