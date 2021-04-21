#ifndef FC3D_AMBIENT_LIGHTSOURCE_H_INCLUDED
#define FC3D_AMBIENT_LIGHTSOURCE_H_INCLUDED

#include <FC3D/fc3d.h>

#include <WF3D/Rendering/Design/color.h>

#include <FC3D/Rendering/Lightsource/lightsource.h>

typedef struct
{
    wf3d_color color;

} fc3d_AmbientLightsource;

//
FC3D_DLL_EXPORT fc3d_AmbientLightsource* fc3d_AmbientLightsource_Create(float r, float g, float b);

//
FC3D_DLL_EXPORT void fc3d_AmbientLightsource_Destroy(fc3d_AmbientLightsource* lightsource);

//
FC3D_DLL_EXPORT fc3d_AmbientLightsource* fc3d_AmbientLightsource_SetIntensity(fc3d_AmbientLightsource* lightsource, float new_r, float new_g, float new_b);

//
FC3D_DLL_EXPORT wf3d_color OWL_VECTORCALL fc3d_AmbientLightsource_EnlightSurfacePoint(void* lightsource_obj, wf3d_surface const* surface, wf3d_color diffusion_color, owl_v3f32 v_pos, owl_v3f32 normal, owl_v3f32 vision_ray_dir);

//
FC3D_DLL_EXPORT extern fc3d_lightsource_interface const fc3d_AmbientLightsource_interface;

#endif // FC3D_AMBIENT_LIGHTSOURCE_H_INCLUDED
