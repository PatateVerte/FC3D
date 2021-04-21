#include <FC3D/Rendering/RenderingObject/monosurface_triangle.h>

#include <math.h>

fc3d_rendering_object_interface const fc3d_monosurface_triangle_rendering_interface =
{
    .NearestIntersectionWithRay = &fc3d_monosurface_triangle_NearestIntersectionWithRay,
    .Rasterization = &fc3d_monosurface_triangle_Rasterization,
    .DepthRasterization = &fc3d_monosurface_triangle_DepthRasterization,
    .Radius = &fc3d_monosurface_triangle_Radius,
    .InfRadiusWithTransform = &fc3d_monosurface_triangle_InfRadiusWithTransform
};

//
//
//
bool fc3d_monosurface_triangle_NearestIntersectionWithRay(void const* obj, owl_v3f32 v_pos, owl_q32 q_rot, owl_v3f32 ray_origin, owl_v3f32 ray_dir, float t_min, float t_max, float* t_ret, owl_v3f32* normal_ret, wf3d_surface const** surface_ret, wf3d_color* diffusion_color_ret)
{
    fc3d_monosurface_triangle const* mono_triangle = obj;

    bool intersection_exists = wf3d_triangle3d_NearestIntersectionWithRay(&mono_triangle->triangle3d, v_pos, q_rot, ray_origin, ray_dir, t_min, t_max, t_ret, normal_ret);

    if(intersection_exists)
    {
        if(surface_ret != NULL)
        {
            *surface_ret = mono_triangle->surface;
        }

        if(diffusion_color_ret != NULL)
        {
            *diffusion_color_ret = mono_triangle->diffusion_color;
        }
    }

    return intersection_exists;
}

//
//
//
void OWL_VECTORCALL fc3d_monosurface_triangle_rasterization_callback(wf3d_rasterization_rectangle const* rect, int x, int y, void const* callback_arg, owl_v3f32 v_intersection, owl_v3f32 normal)
{
    fc3d_monosurface_triangle_rasterization_callback_arg const* arg = callback_arg;
    fc3d_monosurface_triangle const* mono_triangle = arg->mono_triangle;

    float depth = -owl_v3f32_unsafe_get_component(v_intersection, 2);
    int x3d = x - rect->x_min;
    int y3d = y - rect->y_min;

    if(depth < fc3d_Image3d_unsafe_Depth(arg->img3d, x3d, y3d))
    {
        fc3d_Image3d_unsafe_SetPixel(arg->img3d, x3d, y3d, mono_triangle->surface, &mono_triangle->diffusion_color, depth, normal);
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
    callback_arg.mono_triangle = mono_triangle;

    wf3d_rasterization_callback callback;
    callback.callback_arg = &callback_arg;
    callback.callback_fct = &fc3d_monosurface_triangle_rasterization_callback;

    wf3d_triangle3d_Rasterization(&mono_triangle->triangle3d, &callback, rect, v_pos, q_rot, cam);
}

//
//
//
void fc3d_monosurface_triangle_DepthRasterization(void const* obj, fc3d_DepthImage* depth_img, wf3d_rasterization_rectangle const* rect, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam)
{
    fc3d_monosurface_triangle const* mono_triangle = obj;

    wf3d_rasterization_callback callback;
    callback.callback_arg = depth_img;
    callback.callback_fct = &fc3d_DepthImage_rasterization_callback;

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

//Create a cube in a list of monosurface_triangle
//cube_face_list[12]
//
fc3d_monosurface_triangle* fc3d_monosurface_triangle_FillListWithCube(fc3d_monosurface_triangle* cube_face_list, float side, wf3d_surface const* const* surface_list, wf3d_color const* diffusion_color_list)
{
    owl_v3f32 base_xyz[3];
    owl_v3f32_base_xyz(base_xyz, 1.0f);

    owl_v3f32 adapted_base_xyz[3];
    owl_v3f32_base_xyz(adapted_base_xyz, 0.5f * side);

    for(int bk0 = 0 ; bk0 < 3 ; bk0++)
    {
        int bk1 = (bk0 + 1) % 3;
        int bk2 = (bk0 + 2) % 3;

        for(float sign_face = -1.0f ; sign_face <= 1.0f ; sign_face += 2.0f)
        {
            owl_v3f32 face_center = owl_v3f32_scalar_mul(adapted_base_xyz[bk0], sign_face);
            owl_v3f32 normal = owl_v3f32_scalar_mul(base_xyz[bk0], sign_face);

            owl_v3f32 corner_list[3];
            corner_list[0] = owl_v3f32_add(
                                            face_center,
                                            owl_v3f32_add(adapted_base_xyz[bk1], adapted_base_xyz[bk2])
                                           );
            corner_list[1] = owl_v3f32_sub(
                                            face_center,
                                            owl_v3f32_add(adapted_base_xyz[bk1], adapted_base_xyz[bk2])
                                           );

            for(float sign_corner = -1.0f ; sign_corner <= 1.0f ; sign_corner += 2.0f)
            {
                corner_list[2] = owl_v3f32_add_scalar_mul(
                                                            face_center,
                                                            owl_v3f32_sub(adapted_base_xyz[bk1], adapted_base_xyz[bk2]),
                                                            sign_corner
                                                          );

                unsigned int face_i = (unsigned int)(4 * bk0 + ((int)sign_face + 1) + ((int)sign_corner + 1) / 2);

                cube_face_list[face_i].surface = surface_list[3 * ((1 - (int)sign_face) / 2) + bk0];
                cube_face_list[face_i].diffusion_color = diffusion_color_list[3 * ((1 - (int)sign_face) / 2) + bk0];

                cube_face_list[face_i].triangle3d.normal = normal;
                for(unsigned int vi = 0 ; vi < 3 ; vi++)
                {
                    cube_face_list[face_i].triangle3d.vertex_list[vi] = corner_list[vi];
                }
            }
        }
    }

    return cube_face_list;
}
