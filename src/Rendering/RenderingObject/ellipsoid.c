#include <FC3D/Rendering/RenderingObject/ellipsoid.h>

fc3d_rendering_object_interface const fc3d_Ellipsoid_rendering_interface =
(fc3d_rendering_object_interface)   {
                                        .NearestIntersectionWithRay = &fc3d_Ellipsoid_NearestIntersectionWithRay,
                                        .Rasterization = &fc3d_Ellipsoid_Rasterization,
                                        .Radius = &fc3d_Ellipsoid_Radius,
                                        .InfRadiusWithTransform = &fc3d_Ellipsoid_InfRadiusWithTransform
                                    };

//
//
//
fc3d_Ellipsoid* fc3d_Ellipsoid_Create(float rx, float ry, float rz, wf3d_surface const* surface, wf3d_color const* diffusion_color)
{
    fc3d_Ellipsoid* ellipsoid = malloc(sizeof(*ellipsoid));

    if(ellipsoid != NULL)
    {
        ellipsoid->surface = surface;
        ellipsoid->diffusion_color = *diffusion_color;
        fc3d_Ellipsoid_UpdateAxis(ellipsoid, rx, ry, rz);
    }

    return ellipsoid;
}

//
//
//
void fc3d_Ellipsoid_Destroy(fc3d_Ellipsoid* ellispoid)
{
    if(ellispoid != NULL)
    {
        free(ellispoid);
    }
}

//Updates the three axes
//
//
fc3d_Ellipsoid* fc3d_Ellipsoid_UpdateAxis(fc3d_Ellipsoid* ellipsoid, float rx, float ry, float rz)
{
    ellipsoid->r[0] = rx;
    ellipsoid->r[1] = ry;
    ellipsoid->r[2] = rz;

    owl_q32 q_eigenbasis = owl_q32_from_real(1.0);
    owl_v3f32 a = owl_v3f32_zero();
    owl_v3f32 alpha = owl_v3f32_set(1.0 / (rx*rx), 1.0 / (ry*ry), 1.0 / (rz*rz));

    float const delta = 1.0 - (1.0 / ((float)(1<<7)));
    owl_v3f32 norminf_filter = owl_v3f32_set(delta / rx, delta / ry, delta / rz);
    owl_v3f32 norm2_filter = owl_v3f32_set(delta / rx, delta / ry, delta / rz);

    wf3d_quadratic_curve_set(&ellipsoid->curve, q_eigenbasis, norminf_filter, norm2_filter, -1.0, a, alpha);

    return ellipsoid;
}

//Update one axis
//
//
fc3d_Ellipsoid* fc3d_Ellipsoid_UpdateOneAxis(fc3d_Ellipsoid* ellipsoid, unsigned int axis, float r)
{
    if(axis < 3)
    {
        ellipsoid->r[axis] = r;
        fc3d_Ellipsoid_UpdateAxis(ellipsoid, ellipsoid->r[0], ellipsoid->r[1], ellipsoid->r[2]);
    }

    return ellipsoid;
}

//Reverse the normal
//
//
fc3d_Ellipsoid* fc3d_Ellipsoid_ReverseNormal(fc3d_Ellipsoid* ellipsoid)
{
    ellipsoid->curve.alpha = owl_v3f32_scalar_mul(ellipsoid->curve.alpha, -1.0);
    ellipsoid->curve.c *= -1.0;

    return ellipsoid;
}

//
//
//
bool fc3d_Ellipsoid_NearestIntersectionWithRay(void const* obj, owl_v3f32 v_pos, owl_q32 q_rot, owl_v3f32 ray_origin, owl_v3f32 ray_dir, float t_min, float t_max, float* t_ret, owl_v3f32* normal_ret, wf3d_surface const** surface_ret, wf3d_color* diffusion_color_ret)
{
    fc3d_Ellipsoid const* ellipsoid = obj;

    bool intersection_exists = wf3d_quadratic_curve_NearestIntersectionWithRay(&ellipsoid->curve, v_pos, q_rot, ray_origin, ray_dir, t_min, t_max, t_ret, normal_ret);

    if(intersection_exists)
    {
        if(surface_ret != NULL)
        {
            *surface_ret = ellipsoid->surface;
        }

        if(diffusion_color_ret != NULL)
        {
            *diffusion_color_ret = ellipsoid->diffusion_color;
        }
    }

    return intersection_exists;
}

//
//
//
typedef struct
{
    fc3d_Ellipsoid const* ellipsoid;
    fc3d_Image3d* img3d;

} fc3d_Ellipsoid_rasterization_callback_arg;

static void fc3d_Ellipsoid_rasterization_callback(wf3d_rasterization_rectangle const* rect, int x, int y, void const* callback_arg, owl_v3f32 v_intersection, owl_v3f32 normal)
{
    fc3d_Ellipsoid_rasterization_callback_arg const* arg = callback_arg;
    fc3d_Ellipsoid const* ellipsoid = arg->ellipsoid;

    float depth = -owl_v3f32_unsafe_get_component(v_intersection, 2);
    int x3d = x - rect->x_min;
    int y3d = y - rect->y_min;

    if(depth < fc3d_Image3d_unsafe_Depth(arg->img3d, x3d, y3d))
    {
        fc3d_Image3d_unsafe_SetPixel(arg->img3d, x3d, y3d, ellipsoid->surface, &ellipsoid->diffusion_color, depth, v_intersection, normal);
    }
}

//
//
//
void fc3d_Ellipsoid_Rasterization(void const* obj, fc3d_Image3d* img3d, wf3d_rasterization_rectangle const* rect, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam)
{
    fc3d_Ellipsoid const* ellipsoid = obj;

    fc3d_Ellipsoid_rasterization_callback_arg callback_arg;
    callback_arg.img3d = img3d;
    callback_arg.ellipsoid = ellipsoid;

    wf3d_rasterization_callback callback;
    callback.callback_arg = &callback_arg;
    callback.callback_fct = &fc3d_Ellipsoid_rasterization_callback;

    wf3d_quadratic_curve_Rasterization(&ellipsoid->curve, &callback, rect, v_pos, q_rot, cam);
}

//
//
//
float fc3d_Ellipsoid_Radius(void const* obj)
{
    fc3d_Ellipsoid const* ellipsoid = obj;
    return fmaxf(ellipsoid->r[2], fmaxf(ellipsoid->r[1], ellipsoid->r[0]));
}

//
//
//
float fc3d_Ellipsoid_InfRadiusWithTransform(void const* obj, owl_v3f32 v_pos, owl_q32 q_rot)
{
    fc3d_Ellipsoid const* ellipsoid = obj;

    owl_v3f32 base_xyz[3];
    owl_v3f32_setbase_xyz(base_xyz, ellipsoid->r[0], ellipsoid->r[1], ellipsoid->r[2]);

    for(unsigned int j = 0 ; j < 3 ; j++)
    {
        base_xyz[j] = owl_q32_transform_v3f32(q_rot, base_xyz[j]);
    }

    float inf_radius = 0.0;
    for(float sign_z = -1.0 ; sign_z <= 1.0 ; sign_z += 2.0)
    {
        owl_v3f32 center_z = owl_v3f32_add_scalar_mul(v_pos, base_xyz[2], sign_z);
        for(float sign_y = -1.0 ; sign_y <= 1.0 ; sign_y += 2.0)
        {
            owl_v3f32 center_y = owl_v3f32_add_scalar_mul(center_z, base_xyz[1], sign_y);
            for(float sign_x = -1.0 ; sign_x <= 1.0 ; sign_x += 2.0)
            {
                owl_v3f32 vertex = owl_v3f32_add_scalar_mul(center_y, base_xyz[0], sign_x);
                inf_radius = fmaxf(inf_radius, owl_v3f32_norminf(vertex));
            }
        }
    }

    return inf_radius;
}

