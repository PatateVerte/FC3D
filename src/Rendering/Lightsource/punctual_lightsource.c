#include <FC3D/Rendering/Lightsource/punctual_lightsource.h>

//
//
//
fc3d_lightsource_interface const fc3d_PunctualLightsource_interface =
(fc3d_lightsource_interface)    {
                                    .EnlightSurfacePoint = &fc3d_PunctualLightsource_EnlightSurfacePoint
                                };

//
//
//
fc3d_PunctualLightsource* fc3d_PunctualLightsource_Create(float r, float g, float b, bool shadow_mapping, int map_width, int map_height, float bias, float cam_near_clipping_distance)
{
    if(map_width > 0 && map_height > 0)
    {
        fc3d_PunctualLightsource* lightsource = malloc(sizeof(*lightsource));

        if(lightsource != NULL)
        {
            fc3d_PunctualLightsource_SetIntensity(lightsource, r, g, b);

            lightsource->shadow_mapping = shadow_mapping;

            bool error = false;

            if(shadow_mapping)
            {
                lightsource->map_width = map_width;
                lightsource->map_height = map_height;

                lightsource->bias = bias;
                lightsource->cam_near_clipping_distance = cam_near_clipping_distance;

                for(unsigned int k = 0 ; k < 6 ; k++)
                {
                    lightsource->depth_face[k] = fc3d_DepthImage_Create(map_width, map_height);
                    error = error || (lightsource->depth_face[k] == NULL);
                }
            }
            else
            {
                lightsource->map_width = 0;
                lightsource->map_height = 0;

                lightsource->bias = 0.0;
                lightsource->cam_near_clipping_distance = 0.0;

                for(unsigned int k = 0 ; k < 6 ; k++)
                {
                    lightsource->depth_face[k] = NULL;
                }
            }

            if(error)
            {
                fc3d_PunctualLightsource_Destroy(lightsource);
            }
        }

        return lightsource;
    }
    else
    {
        return NULL;
    }
}

//
//
//
void fc3d_PunctualLightsource_Destroy(fc3d_PunctualLightsource* lightsource)
{
    if(lightsource != NULL)
    {
        for(unsigned int k = 0 ; k < 6 ; k++)
        {
            fc3d_DepthImage_Destroy(lightsource->depth_face[k]);
        }

        free(lightsource);
    }
}

//
//
//
fc3d_PunctualLightsource* fc3d_PunctualLightsource_SetIntensity(fc3d_PunctualLightsource* lightsource, float new_r, float new_g, float new_b)
{
    lightsource->color = wf3d_color_set(new_r, new_g, new_b);

    return lightsource;
}

//From face coordinates to lightsource coordinates
//
//
static float face_q_rot_coords_tab[6][4] OWL_ALIGN16 =
{
    {(float)OWL_SQRT2 / 2.0f, 0.0f, -(float)OWL_SQRT2 / 2.0f, 0.0f},
    {(float)OWL_SQRT2 / 2.0f, 0.0f, (float)OWL_SQRT2 / 2.0f, 0.0f},
    {(float)OWL_SQRT2 / 2.0f, (float)OWL_SQRT2 / 2.0f, 0.0f, 0.0f},
    {(float)OWL_SQRT2 / 2.0f, -(float)OWL_SQRT2 / 2.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 0.0f},
    {1.0f, 0.0f, 0.0f, 0.0f}
};

//+x,-x,+y,-y,+z,-z
/*static float face_normal_coords[6][4] OWL_ALIGN16 =
{
    {1.0f, 0.0f, 0.0f, 0.0f},
    {-1.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f, 0.0f},
    {0.0f, -1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, -1.0f, 0.0f}
};*/

//
//
//
fc3d_PunctualLightsource* fc3d_PunctualLightsource_UpdateShadowArea(fc3d_PunctualLightsource* lightsource, fc3d_RenderingOctree const* octree, owl_v3f32 octree_v_pos, owl_q32 octree_q_rot, owl_v3f32 area_center, float area_radius)
{
    if(lightsource->shadow_mapping)
    {
        float area_center_distance = owl_v3f32_norm(area_center);

        wf3d_camera3d cam;
        cam.blackface_culling_enabled = false;
        cam.max_nb_reflections = 0;
        cam.near_clipping_distance = lightsource->cam_near_clipping_distance;
        cam.tan_h_half_opening_angle = 1.0f;
        cam.tan_v_half_opening_angle = 1.0f;

        if(area_center_distance > area_radius)
        {
            for(unsigned int k = 0 ; k < 6 ; k++)
            {
                wf3d_rasterization_rectangle rect;
                rect.width = lightsource->map_width;
                rect.height = lightsource->map_height;

                owl_v3f32 c = owl_q32_transform_v3f32(
                                                        owl_q32_conj(owl_q32_load4(face_q_rot_coords_tab[k])),
                                                        area_center
                                                    );
                float norm_c = area_center_distance;
                owl_v3f32 u = owl_v3f32_scalar_div(c, norm_c);
                float R = area_radius;
                float c_2 = owl_v3f32_unsafe_get_component(c, 2);

                if(c_2 < R)
                {
                    float cos_angle_c = -c_2 / norm_c;
                    float angle_c = acosf(cos_angle_c);
                    float sin_opening_angle = R / norm_c;
                    float cos_opening_angle = sqrtf(1.0f - sin_opening_angle * sin_opening_angle);
                    float opening_angle = asinf(sin_opening_angle);

                    float max_angle = angle_c + opening_angle;
                    float min_angle = angle_c - opening_angle;
                    float const lim_angle = atanf((float)OWL_SQRT2);
                    float const whole_screen_lim_angle = (float)OWL_PI / 4.0f;

                    if(min_angle < lim_angle)
                    {
                        owl_v3f32 face_octree_v_pos = owl_q32_transform_v3f32(
                                                                                owl_q32_conj(owl_q32_load4(face_q_rot_coords_tab[k])),
                                                                                octree_v_pos
                                                                              );
                        owl_q32 face_octree_q_rot = owl_q32_mul(
                                                                    owl_q32_conj(owl_q32_load4(face_q_rot_coords_tab[k])),
                                                                    octree_q_rot
                                                                );

                        //The area covers the whole screen
                        if(min_angle < -whole_screen_lim_angle)
                        {
                            rect.x_min = 0;
                            rect.x_max = rect.width;
                            rect.y_min = 0;
                            rect.y_max = rect.height;


                            fc3d_RenderingOctree_DepthRasterization(octree, lightsource->depth_face[k], &rect, face_octree_v_pos, face_octree_q_rot, &cam);
                        }
                        //Projection is a conic
                        else
                        {

                            float coord_min[2] = {INFINITY, INFINITY};
                            float coord_max[2] = {-INFINITY, -INFINITY};

                            //Projection is an hyperbola or a big ellipse
                            if(max_angle > lim_angle)
                            {
                                owl_v3f32 a = owl_v3f32_scalar_div(c, R - c_2);
                                a = owl_v3f32_unsafe_set_component(a, 2, -1.0f);

                                float c_0 = owl_v3f32_unsafe_get_component(c, 0);
                                float c_1 = owl_v3f32_unsafe_get_component(c, 1);
                                float t = R / sqrtf(c_0 * c_0 + c_1 * c_1);
                                owl_v3f32 v = owl_v3f32_set(
                                                                -owl_v3f32_unsafe_get_component(a, 1),
                                                                owl_v3f32_unsafe_get_component(a, 0),
                                                                0.0f
                                                            );

                                owl_v3f32 tang_point[2];
                                tang_point[0] = owl_v3f32_add_scalar_mul(a, v, t);
                                tang_point[1] = owl_v3f32_add_scalar_mul(a, v, -t);

                                //Tangent vector in p_i that both point to their intersection point or both point to infinity
                                owl_v3f32 tang_vect[2];
                                for(unsigned int p = 0 ; p < 2 ; p++)
                                {
                                    owl_v3f32 half_grad_f = owl_v3f32_add_scalar_mul(
                                                                                        owl_v3f32_scalar_mul(u, owl_v3f32_dot(u, tang_point[p])),
                                                                                        tang_point[p], -cos_opening_angle * cos_opening_angle
                                                                                      );
                                    tang_vect[p] = owl_v3f32_normalize(owl_v3f32_cross(owl_v3f32_set(0.0f, 0.0f, 1.0f), half_grad_f));
                                }
                                tang_vect[1] = owl_v3f32_negate(tang_vect[1]);

                                //Intersection of tangents
                                float tang_intersection_param = - owl_v3f32_norm(owl_v3f32_sub(tang_point[1], tang_point[0])) / owl_v3f32_norm(owl_v3f32_sub(tang_vect[1], tang_vect[0]));
                                owl_v3f32 tang_intersection = owl_v3f32_add_scalar_mul(
                                                                                        tang_point[0], tang_vect[0],
                                                                                        tang_intersection_param
                                                                                      );

                                //
                                float tang_intersection_coords[2] =
                                {
                                    owl_v3f32_unsafe_get_component(tang_intersection, 0),
                                    owl_v3f32_unsafe_get_component(tang_intersection, 1)
                                };
                                float tang_vect_coords[2][2];
                                for(unsigned int p = 0 ; p < 2 ; p++)
                                {
                                    tang_vect_coords[p][0] = owl_v3f32_unsafe_get_component(tang_vect[p], 0);
                                    tang_vect_coords[p][1] = owl_v3f32_unsafe_get_component(tang_vect[p], 1);
                                }

                                for(unsigned int p = 0 ; p < 2 ; p++)
                                {
                                    for(unsigned int i = 0 ; i < 2 ; i++)
                                    {
                                        unsigned int j = (i + 1) % 2;
                                        if(tang_intersection_coords[j] <= 1.0f && tang_intersection_coords[j] >= -1.0f)
                                        {
                                            coord_min[i] = fminf(coord_min[i], tang_intersection_coords[i]);
                                            coord_max[i] = fmaxf(coord_max[i], tang_intersection_coords[i]);
                                        }

                                        for(float sign = -1.0f ; sign <= 1.0f ; sign += 2.0f)
                                        {
                                            float s = (sign - tang_intersection_coords[j]) / tang_vect_coords[p][j];
                                            float sign_coord = tang_intersection_coords[i] + tang_vect_coords[p][i] * s;
                                            if(isfinite(sign_coord) && s > 0.0f)
                                            {
                                                coord_min[i] = fminf(coord_min[i], sign_coord);
                                                coord_max[i] = fmaxf(coord_max[i], sign_coord);
                                            }
                                        }
                                    }
                                }
                            }
                            //Projection is an ellipse
                            else
                            {
                                float cos2_alpha = cos_opening_angle * cos_opening_angle;
                                float u_coords[4] OWL_ALIGN16;
                                owl_v3f32_store4(u_coords, u);

                                for(unsigned i = 0 ; i < 2 ; i++)
                                {
                                    unsigned int j = (i + 1) % 2;

                                    float o_coords[4] OWL_ALIGN16 = {0.0f, 0.0f, 0.0f, 0.0f};
                                    o_coords[j] = u_coords[j] * u_coords[2] / (u_coords[j] * u_coords[j] - cos2_alpha);
                                    o_coords[2] = -1.0f;
                                    owl_v3f32 o = owl_v3f32_load4(o_coords);

                                    float w_coords[4] OWL_ALIGN16 = {0.0f, 0.0f, 0.0f, 0.0f};
                                    w_coords[i] = 1.0f;
                                    w_coords[j] = - u_coords[i] * u_coords[j] / (u_coords[j] * u_coords[j] - cos2_alpha);
                                    owl_v3f32 w = owl_v3f32_load4(w_coords);

                                    float a_ = owl_v3f32_dot(w, w) * cos2_alpha - owl_v3f32_dot(w, u) * owl_v3f32_dot(w, u);
                                    float b_red = owl_v3f32_dot(o, w) * cos2_alpha - owl_v3f32_dot(o, u) * owl_v3f32_dot(w, u);
                                    float c_ = owl_v3f32_dot(o, o) * cos2_alpha - owl_v3f32_dot(o, u) * owl_v3f32_dot(o, u);
                                    if(a_ < 0.0f)
                                    {
                                        a_ = -a_;
                                        b_red = -b_red;
                                        c_ = -c_;
                                    }

                                    float sqrt_delta_red = sqrtf(b_red*b_red - a_*c_);
                                    coord_min[i] = fminf(
                                                            coord_min[i],
                                                            (-b_red - sqrt_delta_red) / a_
                                                         );
                                    coord_max[i] = fmaxf(
                                                            coord_max[i],
                                                            (-b_red + sqrt_delta_red) / a_
                                                         );
                                }
                            }

                            if(coord_min[0] < 1.0f && coord_max[0] > -1.0f && coord_min[1] < 1.0f && coord_max[1] > -1.0f)
                            {
                                int x_min = (int)(roundf(fminf(fmaxf(0.5f * (coord_min[0] + 1.0f), 0.0f), 1.0f) * (float)rect.width));
                                if(x_min < 0)
                                {
                                    x_min = 0;
                                }
                                if(x_min >= rect.width)
                                {
                                    x_min = rect.width;
                                }

                                int x_max = (int)(roundf(fminf(fmaxf(0.5f * (coord_max[0] + 1.0f), 0.0f), 1.0f) * (float)rect.width));
                                if(x_max < 0)
                                {
                                    x_max = 0;
                                }
                                if(x_max >= rect.width)
                                {
                                    x_max = rect.width;
                                }

                                int y_min = (int)(roundf(fminf(fmaxf(0.5f * (coord_min[1] + 1.0f), 0.0f), 1.0f) * (float)rect.height));
                                if(y_min < 0)
                                {
                                    y_min = 0;
                                }
                                if(y_min >= rect.height)
                                {
                                    y_min = rect.height;
                                }

                                int y_max = (int)(roundf(fminf(fmaxf(0.5f * (coord_max[1] + 1.0f), 0.0f), 1.0f) * (float)rect.height));
                                if(y_max < 0)
                                {
                                    y_max = 0;
                                }
                                if(y_max >= rect.height)
                                {
                                    y_max = rect.height;
                                }

                                rect.x_min = x_min;
                                rect.x_max = x_max;
                                rect.y_min = y_min;
                                rect.y_max = y_max;


                                fc3d_RenderingOctree_DepthRasterization(octree, lightsource->depth_face[k], &rect, face_octree_v_pos, face_octree_q_rot, &cam);
                            }
                        }
                    }
                }
            }
        }
        else
        {
            wf3d_rasterization_rectangle rect;
            rect.width = lightsource->map_width;
            rect.height = lightsource->map_height;
            rect.x_min = 0;
            rect.x_max = rect.width;
            rect.y_min = 0;
            rect.y_max = rect.height;

            for(unsigned int k = 0 ; k < 6 ; k++)
            {
                owl_v3f32 face_octree_v_pos = owl_q32_transform_v3f32(
                                                                        owl_q32_conj(owl_q32_load4(face_q_rot_coords_tab[k])),
                                                                        octree_v_pos
                                                                      );
                owl_q32 face_octree_q_rot = owl_q32_mul(
                                                            owl_q32_conj(owl_q32_load4(face_q_rot_coords_tab[k])),
                                                            octree_q_rot
                                                        );
                fc3d_RenderingOctree_DepthRasterization(octree, lightsource->depth_face[k], &rect, face_octree_v_pos, face_octree_q_rot, &cam);
            }
        }
    }

    return lightsource;
}

//
//
//
wf3d_color OWL_VECTORCALL fc3d_PunctualLightsource_EnlightSurfacePoint(void* lightsource_obj, wf3d_surface const* surface, wf3d_color diffusion_color, owl_v3f32 v_pos, owl_v3f32 normal, owl_v3f32 vision_ray_dir)
{
    fc3d_PunctualLightsource const* lightsource = lightsource_obj;
    wf3d_color final_color = wf3d_color_black();

    bool is_shadowed = false;

    if(lightsource->shadow_mapping)
    {
        unsigned int shadow_face_i = 0;
        {
            float v_pos_coords[4] OWL_ALIGN16;
            owl_v3f32_store4(v_pos_coords, v_pos);

            float f_max = 0.0f;

            for(unsigned int k = 0 ; k < 3 ; k++)
            {
                if(v_pos_coords[k] > f_max)
                {
                    f_max = v_pos_coords[k];
                    shadow_face_i = 2*k;
                }
                else if(-v_pos_coords[k] > f_max)
                {
                    f_max = -v_pos_coords[k];
                    shadow_face_i = 2*k+1;
                }
            }
        }

        owl_v3f32 rel_v_pos = owl_q32_transform_v3f32(
                                                        owl_q32_conj(owl_q32_load4(face_q_rot_coords_tab[shadow_face_i])),
                                                        v_pos
                                                      );
        float depth = - owl_v3f32_unsafe_get_component(rel_v_pos, 2);
        float xf = owl_v3f32_unsafe_get_component(rel_v_pos, 0) / depth;
        float yf = owl_v3f32_unsafe_get_component(rel_v_pos, 1) / depth;

        int x = (int)(floorf( (xf * 0.5f * (float)lightsource->map_width) +  0.5f * (float)lightsource->map_width));
        if(x < 0)
        {
            x = 0;
        }
        else if(x >= lightsource->map_width)
        {
            x = lightsource->map_width - 1;
        }

        int y = (int)(floorf( (yf * 0.5f * (float)lightsource->map_height) +  0.5f * (float)lightsource->map_height));
        if(y < 0)
        {
            y = 0;
        }
        else if(y >= lightsource->map_height)
        {
            y = lightsource->map_height - 1;
        }

        is_shadowed = (depth >= fc3d_DepthImage_unsafe_Depth(lightsource->depth_face[shadow_face_i], x, y) + lightsource->bias);
    }

    if(!is_shadowed)
    {
        float square_norm = owl_v3f32_dot(v_pos, v_pos);

        switch(surface->shading_model)
        {
        case WF3D_DEFAULT_SHADING:
            {
                float diffusion_intensity = fmaxf(0.0, -owl_v3f32_dot(v_pos, normal)) / (square_norm * sqrtf(square_norm));

                final_color = wf3d_color_scalar_mul(wf3d_color_mul(diffusion_color, lightsource->color), diffusion_intensity);
            }
            break;

        case WF3D_PHONG_SHADING:
            {
                float diffusion_intensity = fmaxf(0.0f, -owl_v3f32_dot(v_pos, normal)) / (square_norm * sqrtf(square_norm));

                final_color = wf3d_color_scalar_mul(wf3d_color_mul(diffusion_color, lightsource->color), diffusion_intensity);

                owl_v3f32 r = owl_v3f32_add_scalar_mul(v_pos, normal, -2.0f * owl_v3f32_dot(v_pos, normal));
                float specular_factor = fmaxf(
                                                0.0f,
                                                owl_v3f32_dot(vision_ray_dir, r) / sqrtf(square_norm)
                                              );
                float specular_intensity = powf(specular_factor, surface->shininess) / square_norm;

                final_color = wf3d_color_add_scalar_mul(final_color, wf3d_color_mul(surface->specular_color, lightsource->color), specular_intensity);
            }
            break;
        }
    }

    return final_color;
}

