#ifndef FC3D_TRICOLOR_TRIANGLE_H_INCLUDED
#define FC3D_TRICOLOR_TRIANGLE_H_INCLUDED

#include <WF3D/Rendering/Shapes/triangle3d.h>
#include <WF3D/Rendering/Design/surface.h>
#include <WF3D/Rendering/Design/color.h>

#include <FC3D/Rendering/RenderingObject/rendering_object.h>

typedef struct
{
    wf3d_triangle3d triangle3d;

    wf3d_surface const* surface;
    wf3d_color vertex_color[3];

} fc3d_tricolor_triangle;

//
bool fc3d_tricolor_triangle_NearestIntersectionWithRay(void const* obj, owl_v3f32 v_pos, owl_q32 q_rot, owl_v3f32 ray_origin, owl_v3f32 ray_dir, float t_min, float t_max, float* t_ret, owl_v3f32* normal_ret, wf3d_surface const** surface_ret, wf3d_color* diffusion_color_ret);

//
//  RASTERIZATION
//

//
typedef struct
{
    fc3d_tricolor_triangle const* tri_triangle;
    owl_v3f32 opp_side_cross_normal[3];
    owl_v3f32 transformed_next_vertex[3];
    fc3d_Image3d* img3d;

} fc3d_tricolor_triangle_rasterization_callback_arg;
void OWL_VECTORCALL fc3d_tricolor_triangle_rasterization_callback(wf3d_rasterization_rectangle const* rect, int x, int y, void const* callback_arg, owl_v3f32 v_intersection, owl_v3f32 normal);

//
void fc3d_tricolor_triangle_Rasterization(void const* obj, fc3d_Image3d* img3d, wf3d_rasterization_rectangle const* rect, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam);

//
//  DEPTH RASTERIZATION
//

//
void fc3d_tricolor_triangle_DepthRasterization(void const* obj, fc3d_DepthImage* depth_img, wf3d_rasterization_rectangle const* rect, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam);

//
//
//

//
float fc3d_tricolor_triangle_Radius(void const* obj);

//
float fc3d_tricolor_triangle_InfRadiusWithTransform(void const* obj, owl_v3f32 v_pos, owl_q32 q_rot);

//
extern fc3d_rendering_object_interface const fc3d_tricolor_triangle_rendering_interface;

#endif // FC3D_TRICOLOR_TRIANGLE_H_INCLUDED
