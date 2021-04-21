#ifndef FC3D_LIGHTSOURCE_H_INCLUDED
#define FC3D_LIGHTSOURCE_H_INCLUDED

#include <FC3D/fc3d.h>

#include <OWL/Optimized3d/vector/v3f32.h>
#include <OWL/Optimized3d/quaternion/q32.h>

#include <WF3D/Rendering/Design/color.h>
#include <WF3D/Rendering/Design/surface.h>

typedef struct
{
    // ||vision_ray_dir|| == 1
    //v_pos, normal and vision_ray_dir are in the lightsource space
    wf3d_color (OWL_VECTORCALL *EnlightSurfacePoint)(void* lightsource_obj, wf3d_surface const* surface, wf3d_color diffusion_color, owl_v3f32 v_pos, owl_v3f32 normal, owl_v3f32 vision_ray_dir);

} fc3d_lightsource_interface;

typedef struct
{
    void* lightsource_obj;
    owl_v3f32 v_pos;
    owl_q32 q_rot;

    fc3d_lightsource_interface const* lightsource_interface;

} fc3d_lightsource;

#endif // FC3D_LIGHTSOURCE_H_INCLUDED
