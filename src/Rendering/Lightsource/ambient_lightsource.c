#include <FC3D/Rendering/Lightsource/ambient_lightsource.h>

//
//
//
fc3d_lightsource_interface const fc3d_AmbientLightsource_interface =
(fc3d_lightsource_interface)    {
                                    .EnlightSurfacePoint = &fc3d_AmbientLightsource_EnlightSurfacePoint
                                };

//
//
//
fc3d_AmbientLightsource* fc3d_AmbientLightsource_Create(float r, float g, float b)
{
    fc3d_AmbientLightsource* lightsource = malloc(sizeof(*lightsource));

    if(lightsource != NULL)
    {
        fc3d_AmbientLightsource_SetIntensity(lightsource, r, g, b);
    }

    return lightsource;
}

//
//
//
void fc3d_AmbientLightsource_Destroy(fc3d_AmbientLightsource* lightsource)
{
    if(lightsource != NULL)
    {
        free(lightsource);
    }
}

//
//
//
fc3d_AmbientLightsource* fc3d_AmbientLightsource_SetIntensity(fc3d_AmbientLightsource* lightsource, float new_r, float new_g, float new_b)
{
    lightsource->color = wf3d_color_set(new_r, new_g, new_b);

    return lightsource;
}

//
//
//
wf3d_color OWL_VECTORCALL fc3d_AmbientLightsource_EnlightSurfacePoint(void* lightsource_obj, wf3d_surface const* surface, wf3d_color diffusion_color, owl_v3f32 v_pos, owl_v3f32 normal, owl_v3f32 vision_ray_dir)
{
    (void)surface;
    (void)v_pos;
    (void)normal;
    (void)vision_ray_dir;

    fc3d_AmbientLightsource const* lightsource = lightsource_obj;

    return wf3d_color_mul(diffusion_color, lightsource->color);
}

