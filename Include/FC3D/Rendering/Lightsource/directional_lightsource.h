#ifndef FC3D_DIRECTIONAL_LIGHTSOURCE_H_INCLUDED
#define FC3D_DIRECTIONAL_LIGHTSOURCE_H_INCLUDED

#include <FC3D/Rendering/Lightsource/lightsource.h>

typedef struct
{
    // ||dir_vect|| == 1
    owl_v3f32 dir_vect;

    wf3d_color color;

} fc3d_DirectionalLightsource;

//
fc3d_DirectionalLightsource* fc3d_DirectionalLightsource_Create(owl_v3f32 dir_vect, float r, float g, float b);

//
void fc3d_DirectionalLightsource_Destroy(fc3d_DirectionalLightsource* lightsource);

//
fc3d_DirectionalLightsource* fc3d_DirectionalLightsource_SetDirVect(fc3d_DirectionalLightsource* lightsource,owl_v3f32 new_dir_vect);

//
fc3d_DirectionalLightsource* fc3d_DirectionalLightsource_SetIntensity(fc3d_DirectionalLightsource* lightsource, float new_r, float new_g, float new_b);

//
wf3d_color OWL_VECTORCALL fc3d_DirectionalLightsource_EnlightSurfacePoint(void* lightsource_obj, wf3d_surface const* surface, wf3d_color diffusion_color, owl_v3f32 v_pos, owl_v3f32 normal, owl_v3f32 vision_ray_dir);

//
extern fc3d_lightsource_interface const fc3d_DirectionalLightsource_interface;

#endif // FC3D_DIRECTIONAL_LIGHTSOURCE_H_INCLUDED
