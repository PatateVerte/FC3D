#ifndef FC3D_POLYGON_MESH_H_INCLUDED
#define FC3D_POLYGON_MESH_H_INCLUDED

#include <FC3D/fc3d.h>

#include <FC3D/Rendering/RenderingObject/rendering_object.h>

#include <FC3D/Rendering/RenderingObject/monosurface_triangle.h>

typedef struct
{
    unsigned int nb_faces;
    fc3d_monosurface_triangle* face_list;

} fc3d_PolygonMesh;

//
FC3D_DLL_EXPORT fc3d_PolygonMesh* fc3d_PolygonMesh_Create(unsigned int nb_faces, fc3d_monosurface_triangle const* face_list);

//
FC3D_DLL_EXPORT void fc3d_PolygonMesh_Destroy(fc3d_PolygonMesh* mesh);

//
FC3D_DLL_EXPORT wf3d_error fc3d_PolygonMesh_ChangeAllFaces(fc3d_PolygonMesh* mesh, fc3d_monosurface_triangle const* new_face_list);

//
FC3D_DLL_EXPORT wf3d_error fc3d_PolygonMesh_ChangeOneFace(fc3d_PolygonMesh* mesh, unsigned int i, fc3d_monosurface_triangle const* new_face);

//
FC3D_DLL_EXPORT bool fc3d_PolygonMesh_NearestIntersectionWithRay(void const* obj, owl_v3f32 v_pos, owl_q32 q_rot, owl_v3f32 ray_origin, owl_v3f32 ray_dir, float t_min, float t_max, float* t_ret, owl_v3f32* normal_ret, wf3d_surface const** surface_ret, wf3d_color* diffusion_color_ret);

//
FC3D_DLL_EXPORT void fc3d_PolygonMesh_Rasterization(void const* obj, fc3d_Image3d* img3d, wf3d_rasterization_rectangle const* rect, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam);

//
FC3D_DLL_EXPORT void fc3d_PolygonMesh_DepthRasterization(void const* obj, fc3d_DepthImage* depth_img, wf3d_rasterization_rectangle const* rect, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam);

//
FC3D_DLL_EXPORT float fc3d_PolygonMesh_Radius(void const* obj);

//
FC3D_DLL_EXPORT float fc3d_PolygonMesh_InfRadiusWithTransform(void const* obj, owl_v3f32 v_pos, owl_q32 q_rot);

//
FC3D_DLL_EXPORT extern fc3d_rendering_object_interface const fc3d_PolygonMesh_rendering_interface;

#endif // FC3D_POLYGON_MESH_H_INCLUDED
