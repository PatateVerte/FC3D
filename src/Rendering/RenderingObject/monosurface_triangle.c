#include <FC3D/Rendering/RenderingObject/monosurface_triangle.h>

#include <math.h>

fc3d_rendering_object_interface const fc3d_monosurface_triangle_rendering_interface =
(fc3d_rendering_object_interface)   {
                                        .NearestIntersectionWithRay = &fc3d_monosurface_triangle_NearestIntersectionWithRay,
                                        .Rasterization = &fc3d_monosurface_triangle_Rasterization,
                                        .Radius = &fc3d_monosurface_triangle_Radius,
                                        .InfRadiusWithTransform = &fc3d_monosurface_triangle_InfRadiusWithTransform
                                    };

//
//
//
bool fc3d_monosurface_triangle_NearestIntersectionWithRay(void const* obj, owl_v3f32 v_pos, owl_q32 q_rot, owl_v3f32 ray_origin, owl_v3f32 ray_dir, float t_min, float t_max, float* t_ret, owl_v3f32* normal_ret, wf3d_surface* surface_ret)
{
    fc3d_monosurface_triangle const* mono_triangle = obj;

    bool intersection_exists = wf3d_triangle3d_NearestIntersectionWithRay(&mono_triangle->triangle3d, v_pos, q_rot, ray_origin, ray_dir, t_min, t_max, t_ret, normal_ret);

    if(intersection_exists && surface_ret != NULL)
    {
        *surface_ret = *mono_triangle->surface;
    }

    return intersection_exists;
}

typedef struct
{
    wf3d_surface const* surface;
    fc3d_Image3d* img3d;

} fc3d_monosurface_triangle_rasterization_callback_arg;

static void fc3d_monosurface_triangle_rasterization_callback(wf3d_rasterization_rectangle const* rect, int x, int y, void const* callback_arg, owl_v3f32 v_intersection, owl_v3f32 normal)
{
    fc3d_monosurface_triangle_rasterization_callback_arg const* arg = callback_arg;

    float depth = owl_v3f32_unsafe_get_component(v_intersection, 2);
    int x3d = x - rect->x_min;
    int y3d = y - rect->y_min;

    if(depth < fc3d_Image3d_unsafe_Depth(arg->img3d, x3d, y3d))
    {
        fc3d_Image3d_unsafe_SetPixel(arg->img3d, x3d, y3d, arg->surface, depth, v_intersection, normal);
    }
}

//
//
//
void fc3d_monosurface_triangle_Rasterization(void const* obj, fc3d_Image3d* img3d, wf3d_rasterization_rectangle const* rect, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam)
{
    fc3d_monosurface_triangle const* mono_triangle = obj;

    fc3d_monosurface_triangle_rasterization_callback_arg callback_arg;
    callback_arg.img3d = img3d;
    callback_arg.surface = mono_triangle->surface;

    wf3d_rasterization_callback callback;
    callback.callback_arg = &callback_arg;
    callback.callback_fct = &fc3d_monosurface_triangle_rasterization_callback;

    wf3d_triangle3d_Rasterization(&mono_triangle->triangle3d, &callback, rect, v_pos, q_rot, cam);
}

//
//
//
float fc3d_monosurface_triangle_Radius(void const* obj)
{
    fc3d_monosurface_triangle const* mono_triangle = obj;
    wf3d_triangle3d const* triangle = &mono_triangle->triangle3d;

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
float fc3d_monosurface_triangle_InfRadiusWithTransform(void const* obj, owl_v3f32 v_pos, owl_q32 q_rot)
{
    fc3d_monosurface_triangle const* mono_triangle = obj;
    wf3d_triangle3d const* triangle = &mono_triangle->triangle3d;

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
