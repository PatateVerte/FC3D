#include <FC3D/Rendering/Lightsource/directional_lightsource.h>

//
//
//
fc3d_lightsource_interface const fc3d_DirectionalLightsource_interface =
(fc3d_lightsource_interface)    {
                                    .EnlightSurfacePoint = &fc3d_DirectionalLightsource_EnlightSurfacePoint
                                };

//
//
//
fc3d_DirectionalLightsource* fc3d_DirectionalLightsource_Create(owl_v3f32 dir_vect, float r, float g, float b)
{
    fc3d_DirectionalLightsource* lightsource = malloc(sizeof(*lightsource));

    if(lightsource != NULL)
    {
        fc3d_DirectionalLightsource_SetDirVect(lightsource, dir_vect);
        fc3d_DirectionalLightsource_SetIntensity(lightsource, r, g, b);
    }

    return lightsource;
}

//
//
//
void fc3d_DirectionalLightsource_Destroy(fc3d_DirectionalLightsource* lightsource)
{
    if(lightsource != NULL)
    {
        free(lightsource);
    }
}

//
//
//
fc3d_DirectionalLightsource* fc3d_DirectionalLightsource_SetDirVect(fc3d_DirectionalLightsource* lightsource,owl_v3f32 new_dir_vect)
{
    lightsource->dir_vect = new_dir_vect;
    return lightsource;
}


//
//
//
fc3d_DirectionalLightsource* fc3d_DirectionalLightsource_SetIntensity(fc3d_DirectionalLightsource* lightsource, float new_r, float new_g, float new_b)
{
    lightsource->color = wf3d_color_set(new_r, new_g, new_b);

    return lightsource;
}

//
//
//
wf3d_color OWL_VECTORCALL fc3d_DirectionalLightsource_EnlightSurfacePoint(void* lightsource_obj, wf3d_surface const* surface, wf3d_color diffusion_color, owl_v3f32 v_pos, owl_v3f32 normal, owl_v3f32 vision_ray_dir)
{
    fc3d_DirectionalLightsource const* lightsource = lightsource_obj;
    wf3d_color final_color = wf3d_color_black();

    (void)v_pos;

    switch(surface->shading_model)
    {
    case WF3D_DEFAULT_SHADING:
        {
            float diffusion_intensity = fmaxf(
                                                0.0,
                                                -owl_v3f32_dot(normal, lightsource->dir_vect)
                                             );
            final_color = wf3d_color_scalar_mul(wf3d_color_mul(diffusion_color, lightsource->color), diffusion_intensity);
        }
        break;

    case WF3D_PHONG_SHADING:
        {
            //Diffusion
            float diffusion_intensity = fmaxf(
                                                0.0,
                                                -owl_v3f32_dot(normal, lightsource->dir_vect)
                                             );
            final_color = wf3d_color_scalar_mul(wf3d_color_mul(diffusion_color, lightsource->color), diffusion_intensity);

            //Specular light
            owl_v3f32 r = owl_v3f32_add_scalar_mul(
                                                    lightsource->dir_vect,
                                                    normal,
                                                    -2.0f * owl_v3f32_dot(lightsource->dir_vect, normal)
                                                   );
            float specular_factor = fmaxf(0.0f, -owl_v3f32_dot(r, vision_ray_dir));
            float specular_intensity = powf(specular_factor, surface->shininess);

            final_color = wf3d_color_add_scalar_mul(final_color, wf3d_color_mul(surface->specular_color, lightsource->color), specular_intensity);
        }
        break;
    }

    return final_color;
}

