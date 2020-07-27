#include <FC3D/Rendering/RenderingObject/tricolor_triangle.h>

#include <FC3D/Rendering/RenderingObject/monosurface_triangle.h>

#include <math.h>

fc3d_rendering_object_interface const fc3d_tricolor_triangle_rendering_interface =
(fc3d_rendering_object_interface)   {
                                        .NearestIntersectionWithRay = &fc3d_tricolor_triangle_NearestIntersectionWithRay,
                                        .Rasterization = &fc3d_tricolor_triangle_Rasterization,
                                        .Radius = &fc3d_tricolor_triangle_Radius,
                                        .InfRadiusWithTransform = &fc3d_tricolor_triangle_InfRadiusWithTransform
                                    };

//
//
//
bool fc3d_tricolor_triangle_NearestIntersectionWithRay(void const* obj, owl_v3f32 v_pos, owl_q32 q_rot, owl_v3f32 ray_origin, owl_v3f32 ray_dir, float t_min, float t_max, float* t_ret, owl_v3f32* normal_ret, wf3d_surface const** surface_ret, wf3d_color* diffusion_color_ret)
{
    fc3d_tricolor_triangle const* tri_triangle = obj;

    float t;
    bool intersection_exists = wf3d_triangle3d_NearestIntersectionWithRay(&tri_triangle->triangle3d, v_pos, q_rot, ray_origin, ray_dir, t_min, t_max, &t, normal_ret);

    if(intersection_exists)
    {
        if(t_ret != NULL)
        {
            *t_ret = t;
        }

        if(surface_ret != NULL)
        {
            *surface_ret = tri_triangle->surface;
        }

        if(diffusion_color_ret != NULL)
        {
            wf3d_triangle3d const* triangle3d = &tri_triangle->triangle3d;

            owl_v3f32 v_intersection = owl_v3f32_add_scalar_mul(ray_origin, ray_dir, t);
            owl_v3f32 rel_v_intersection = owl_q32_transform_v3f32(
                                                                    owl_q32_conj(q_rot),
                                                                    owl_v3f32_sub(v_intersection, v_pos)
                                                                   );
            float det[3];
            float det_sum = 0.0;
            for(unsigned int vi0 = 0 ; vi0 < 3 ; vi0++)
            {
                unsigned int vi1 = (vi0 + 1) % 3;
                unsigned int vi2 = (vi0 + 2) % 3;

                det[vi0] = owl_v3f32_triple(
                                                triangle3d->normal,
                                                owl_v3f32_sub(triangle3d->vertex_list[vi2], triangle3d->vertex_list[vi1]),
                                                owl_v3f32_sub(rel_v_intersection, triangle3d->vertex_list[vi1])
                                            );
                det_sum += det[vi0];
            }

            float barycentric_coords[3];
            for(unsigned int vi = 0 ; vi < 3 ; vi++)
            {
                barycentric_coords[vi] = det[vi] / det_sum;
            }

            wf3d_color_mix(diffusion_color_ret, tri_triangle->vertex_color, barycentric_coords, 3);
        }
    }

    return intersection_exists;
}

typedef struct
{
    fc3d_tricolor_triangle const* tri_triangle;
    owl_v3f32 opp_side_cross_normal[3];
    owl_v3f32 transformed_next_vertex[3];
    fc3d_Image3d* img3d;

} fc3d_tricolor_triangle_rasterization_callback_arg;

static void fc3d_tricolor_triangle_rasterization_callback(wf3d_rasterization_rectangle const* rect, int x, int y, void const* callback_arg, owl_v3f32 v_intersection, owl_v3f32 normal)
{
    fc3d_tricolor_triangle_rasterization_callback_arg const* arg = callback_arg;
    fc3d_tricolor_triangle const* tri_triangle = arg->tri_triangle;

    float depth = -owl_v3f32_unsafe_get_component(v_intersection, 2);
    int x3d = x - rect->x_min;
    int y3d = y - rect->y_min;

    if(depth < fc3d_Image3d_unsafe_Depth(arg->img3d, x3d, y3d))
    {
        wf3d_color diffusion_color;
        float det[3];
        float det_sum = 0.0;
        for(unsigned int vi0 = 0 ; vi0 < 3 ; vi0++)
        {
            det[vi0] = owl_v3f32_dot(arg->opp_side_cross_normal[vi0], owl_v3f32_sub(v_intersection, arg->transformed_next_vertex[vi0]));
            det_sum += det[vi0];
        }

        float barycentric_coords[3];
        for(unsigned int vi = 0 ; vi < 3 ; vi++)
        {
            barycentric_coords[vi] = det[vi] / det_sum;
        }

        wf3d_color_mix(&diffusion_color, tri_triangle->vertex_color, barycentric_coords, 3);

        fc3d_Image3d_unsafe_SetPixel(arg->img3d, x3d, y3d, tri_triangle->surface, &diffusion_color, depth, v_intersection, normal);
    }
}

//
//
//
void fc3d_tricolor_triangle_Rasterization(void const* obj, fc3d_Image3d* img3d, wf3d_rasterization_rectangle const* rect, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam)
{
    fc3d_tricolor_triangle const* tri_triangle = obj;

    fc3d_tricolor_triangle_rasterization_callback_arg callback_arg;
    callback_arg.img3d = img3d;
    callback_arg.tri_triangle = tri_triangle;

    wf3d_triangle3d transformed_triangle;
    wf3d_triangle3d_Transform(&transformed_triangle, &tri_triangle->triangle3d, v_pos, q_rot);
    for(unsigned int vi0 = 0 ; vi0 < 3 ; vi0++)
    {
        unsigned int vi1 = (vi0 + 1) % 3;
        unsigned int vi2 = (vi0 + 2) % 3;

        callback_arg.transformed_next_vertex[vi0] = transformed_triangle.vertex_list[vi1];
        callback_arg.opp_side_cross_normal[vi0] = owl_v3f32_cross(
                                                                    transformed_triangle.normal,
                                                                    owl_v3f32_sub(transformed_triangle.vertex_list[vi2], transformed_triangle.vertex_list[vi1])
                                                                );
    }

    wf3d_rasterization_callback callback;
    callback.callback_arg = &callback_arg;
    callback.callback_fct = &fc3d_tricolor_triangle_rasterization_callback;

    wf3d_triangle3d_Rasterization(&tri_triangle->triangle3d, &callback, rect, v_pos, q_rot, cam);
}

//
//
//
float fc3d_tricolor_triangle_Radius(void const* obj)
{
    fc3d_tricolor_triangle const* tri_triangle = obj;
    wf3d_triangle3d const* triangle = &tri_triangle->triangle3d;

    float square_radius = 0.0;

    for(unsigned int vi = 0 ; vi < 3 ; vi++)
    {
        square_radius = fmaxf(
                                square_radius,
                                owl_v3f32_dot(triangle->vertex_list[vi], triangle->vertex_list[vi])
                              );
    }

    return sqrtf(square_radius);
}

//
//
//
float fc3d_tricolor_triangle_InfRadiusWithTransform(void const* obj, owl_v3f32 v_pos, owl_q32 q_rot)
{
    fc3d_tricolor_triangle const* tri_triangle = obj;
    wf3d_triangle3d const* triangle = &tri_triangle->triangle3d;

    float inf_radius = 0.0;

    for(unsigned int vi = 0 ; vi < 3 ; vi++)
    {
        owl_v3f32 v = owl_v3f32_add(
                                        v_pos,
                                        owl_q32_transform_v3f32(q_rot, triangle->vertex_list[vi])
                                    );
        inf_radius = fmaxf(inf_radius, owl_v3f32_norminf(v));
    }

    return inf_radius;
}


