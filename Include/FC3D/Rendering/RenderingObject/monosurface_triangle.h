#ifndef FC3D_MONOSURFACE_TRIANGLE_H_INCLUDED
#define FC3D_MONOSURFACE_TRIANGLE_H_INCLUDED

#include <FC3D/fc3d.h>

#include <WF3D/Rendering/Shapes/triangle3d.h>

#include <FC3D/Rendering/RenderingObject/rendering_object.h>

typedef struct
{
    wf3d_triangle3d triangle3d;

    wf3d_surface const* surface;
    wf3d_color diffusion_color;

} fc3d_monosurface_triangle;

//
FC3D_DLL_EXPORT bool fc3d_monosurface_triangle_NearestIntersectionWithRay(void const* obj, owl_v3f32 v_pos, owl_q32 q_rot, owl_v3f32 ray_origin, owl_v3f32 ray_dir, float t_min, float t_max, float* t_ret, owl_v3f32* normal_ret, wf3d_surface const** surface_ret, wf3d_color* diffusion_color_ret);

//
//  RASTERIZATION
//

//
typedef struct
{
    fc3d_monosurface_triangle const* mono_triangle;
    fc3d_Image3d* img3d;

} fc3d_monosurface_triangle_rasterization_callback_arg;
FC3D_DLL_EXPORT void OWL_VECTORCALL fc3d_monosurface_triangle_rasterization_callback(wf3d_rasterization_rectangle const* rect, int x, int y, void const* callback_arg, owl_v3f32 v_intersection, owl_v3f32 normal);

//
FC3D_DLL_EXPORT void fc3d_monosurface_triangle_Rasterization(void const* obj, fc3d_Image3d* img3d, wf3d_rasterization_rectangle const* rect, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam);


//
//  DEPTH RASTERIZATION
//

//
FC3D_DLL_EXPORT void fc3d_monosurface_triangle_DepthRasterization(void const* obj, fc3d_DepthImage* depth_img, wf3d_rasterization_rectangle const* rect, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam);

//
//
//

//
FC3D_DLL_EXPORT float fc3d_monosurface_triangle_Radius(void const* obj);

//
FC3D_DLL_EXPORT float fc3d_monosurface_triangle_InfRadiusWithTransform(void const* obj, owl_v3f32 v_pos, owl_q32 q_rot);

//
FC3D_DLL_EXPORT extern fc3d_rendering_object_interface const fc3d_monosurface_triangle_rendering_interface;

//Create a cube in a list of monosurface_triangle
//cube_face_list[12]
//
FC3D_DLL_EXPORT fc3d_monosurface_triangle* fc3d_monosurface_triangle_FillListWithCube(fc3d_monosurface_triangle* cube_face_list, float side, wf3d_surface const* const* surface_list, wf3d_color const* diffusion_color_list);

#endif // FC3D_MONOSURFACE_TRIANGLE_H_INCLUDED
