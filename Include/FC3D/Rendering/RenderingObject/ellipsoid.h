#ifndef FC3D_ELLIPSOID_H_INCLUDED
#define FC3D_ELLIPSOID_H_INCLUDED

#include <WF3D/Rendering/Shapes/quadratic_curve.h>
#include <WF3D/Rendering/Design/surface.h>

#include <FC3D/Rendering/RenderingObject/rendering_object.h>

typedef struct
{
    wf3d_quadratic_curve curve;

    wf3d_surface const* surface;
    wf3d_color diffusion_color;

    float r[3];

} fc3d_Ellipsoid;

//
fc3d_Ellipsoid* fc3d_Ellipsoid_Create(float rx, float ry, float rz, wf3d_surface const* surface, wf3d_color const* diffusion_color);

//
void fc3d_Ellipsoid_Destroy(fc3d_Ellipsoid* ellispoid);

//Updates the three axes
fc3d_Ellipsoid* fc3d_Ellipsoid_UpdateAxis(fc3d_Ellipsoid* ellipsoid, float rx, float ry, float rz);

//Update one axis
fc3d_Ellipsoid* fc3d_Ellipsoid_UpdateOneAxis(fc3d_Ellipsoid* ellipsoid, unsigned int axis, float r);

//Reverse the normal
fc3d_Ellipsoid* fc3d_Ellipsoid_ReverseNormal(fc3d_Ellipsoid* ellipsoid);

//
bool fc3d_Ellipsoid_NearestIntersectionWithRay(void const* obj, owl_v3f32 v_pos, owl_q32 q_rot, owl_v3f32 ray_origin, owl_v3f32 ray_dir, float t_min, float t_max, float* t_ret, owl_v3f32* normal_ret, wf3d_surface const** surface_ret, wf3d_color* diffusion_color_ret);

//
void fc3d_Ellipsoid_Rasterization(void const* obj, fc3d_Image3d* img3d, wf3d_rasterization_rectangle const* rect, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam);

//
float fc3d_Ellipsoid_Radius(void const* obj);

//
float fc3d_Ellipsoid_InfRadiusWithTransform(void const* obj, owl_v3f32 v_pos, owl_q32 q_rot);

//
extern fc3d_rendering_object_interface const fc3d_Ellipsoid_rendering_interface;

#endif // FC3D_ELLIPSOID_H_INCLUDED
