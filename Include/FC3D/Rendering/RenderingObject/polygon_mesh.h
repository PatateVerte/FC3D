#ifndef FC3D_POLYGON_MESH_H_INCLUDED
#define FC3D_POLYGON_MESH_H_INCLUDED

#include <FC3D/Rendering/RenderingObject/rendering_object.h>

#include <FC3D/Rendering/RenderingObject/monosurface_triangle.h>

typedef struct
{
    unsigned int nb_faces;
    fc3d_monosurface_triangle* face_list;

} fc3d_PolygonMesh;

//
fc3d_PolygonMesh* fc3d_PolygonMesh_Create(unsigned int nb_faces, fc3d_monosurface_triangle const* face_list);

//
void fc3d_PolygonMesh_Destroy(fc3d_PolygonMesh* mesh);

//
wf3d_error fc3d_PolygonMesh_ChangeAllFaces(fc3d_PolygonMesh* mesh, fc3d_monosurface_triangle const* new_face_list);

//
wf3d_error fc3d_PolygonMesh_ChangeOneFace(fc3d_PolygonMesh* mesh, unsigned int i, fc3d_monosurface_triangle const* new_face);

//
bool fc3d_PolygonMesh_NearestIntersectionWithRay(void const* obj, owl_v3f32 v_pos, owl_q32 q_rot, owl_v3f32 ray_origin, owl_v3f32 ray_dir, float t_min, float t_max, float* t_ret, owl_v3f32* normal_ret, wf3d_surface* surface_ret);

//
void fc3d_PolygonMesh_Rasterization(void const* obj, fc3d_Image3d* img3d, wf3d_rasterization_rectangle const* rect, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam);

//
float fc3d_PolygonMesh_Radius(void const* obj);

//
float fc3d_PolygonMesh_InfRadiusWithTransform(void const* obj, owl_v3f32 v_pos, owl_q32 q_rot);

//
extern fc3d_rendering_object_interface const fc3d_PolygonMesh_rendering_interface;

#endif // FC3D_POLYGON_MESH_H_INCLUDED
