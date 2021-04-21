#include <FC3D/Rendering/RenderingObject/polygon_mesh.h>

#include <malloc.h>

fc3d_rendering_object_interface const fc3d_PolygonMesh_rendering_interface =
{
    .NearestIntersectionWithRay = &fc3d_PolygonMesh_NearestIntersectionWithRay,
    .Rasterization = &fc3d_PolygonMesh_Rasterization,
    .DepthRasterization = &fc3d_PolygonMesh_DepthRasterization,
    .Radius = &fc3d_PolygonMesh_Radius,
    .InfRadiusWithTransform = &fc3d_PolygonMesh_InfRadiusWithTransform
};

//
//
//
fc3d_PolygonMesh* fc3d_PolygonMesh_Create(unsigned int nb_faces, fc3d_monosurface_triangle const* face_list)
{
    fc3d_PolygonMesh* mesh = malloc(sizeof(*mesh));

    if(mesh != NULL)
    {
        mesh->nb_faces = nb_faces;
        mesh->face_list = _aligned_malloc((size_t)nb_faces * sizeof(*mesh->face_list), 16);

        if(nb_faces == 0 || mesh->face_list != NULL)
        {
            fc3d_PolygonMesh_ChangeAllFaces(mesh, face_list);
        }
        else
        {
            fc3d_PolygonMesh_Destroy(mesh);
            mesh = NULL;
        }
    }

    return mesh;
}

//
//
//
void fc3d_PolygonMesh_Destroy(fc3d_PolygonMesh* mesh)
{
    if(mesh != NULL)
    {
        _aligned_free(mesh->face_list);

        free(mesh);
    }
}

//
//
//
wf3d_error fc3d_PolygonMesh_ChangeAllFaces(fc3d_PolygonMesh* mesh, fc3d_monosurface_triangle const* new_face_list)
{
    wf3d_error error = WF3D_SUCCESS;

    for(unsigned int k = 0 ; k < mesh->nb_faces ; k++)
    {
        mesh->face_list[k] = new_face_list[k];
    }

    return error;
}

//
//
//
wf3d_error fc3d_PolygonMesh_ChangeOneFace(fc3d_PolygonMesh* mesh, unsigned int i, fc3d_monosurface_triangle const* new_face)
{
    wf3d_error error = WF3D_SUCCESS;

    if(i < mesh->nb_faces)
    {
        mesh->face_list[i] = *new_face;
    }
    else
    {
        error = WF3D_INVALID_PARAMETER;
    }

    return error;
}

//
//
//
bool fc3d_PolygonMesh_NearestIntersectionWithRay(void const* obj, owl_v3f32 v_pos, owl_q32 q_rot, owl_v3f32 ray_origin, owl_v3f32 ray_dir, float t_min, float t_max, float* t_ret, owl_v3f32* normal_ret, wf3d_surface const** surface_ret, wf3d_color* diffusion_color_ret)
{
    fc3d_PolygonMesh const* mesh = obj;

    bool intersection_found = false;
    float t = t_max;

    for(unsigned int k = 0 ; k < mesh->nb_faces ; k++)
    {
        intersection_found = fc3d_monosurface_triangle_NearestIntersectionWithRay(mesh->face_list + k, v_pos, q_rot, ray_origin, ray_dir, t_min, t, &t, normal_ret, surface_ret, diffusion_color_ret) || intersection_found;
    }

    if(intersection_found && t_ret != NULL)
    {
        *t_ret = t;
    }

    return intersection_found;
}

//
//
//
void fc3d_PolygonMesh_Rasterization(void const* obj, fc3d_Image3d* img3d, wf3d_rasterization_rectangle const* rect, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam)
{
    fc3d_PolygonMesh const* mesh = obj;

    for(unsigned int k = 0 ; k < mesh->nb_faces ; k++)
    {
        fc3d_monosurface_triangle_Rasterization(mesh->face_list + k, img3d, rect, v_pos, q_rot, cam);
    }
}

//
//
//
void fc3d_PolygonMesh_DepthRasterization(void const* obj, fc3d_DepthImage* depth_img, wf3d_rasterization_rectangle const* rect, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam)
{
    fc3d_PolygonMesh const* mesh = obj;

    for(unsigned int k = 0 ; k < mesh->nb_faces ; k++)
    {
        fc3d_monosurface_triangle_DepthRasterization(mesh->face_list + k, depth_img, rect, v_pos, q_rot, cam);
    }
}

//
//
//
float fc3d_PolygonMesh_Radius(void const* obj)
{
    fc3d_PolygonMesh const* mesh = obj;
    float square_radius = 0.0;

    for(unsigned int k = 0 ; k < mesh->nb_faces ; k++)
    {
        for(unsigned int vi = 0 ; vi < 3 ; vi++)
        {
            owl_v3f32 vertex = mesh->face_list[k].triangle3d.vertex_list[vi];
            square_radius = fmaxf(
                                    square_radius,
                                    owl_v3f32_dot(vertex, vertex)
                                  );
        }
    }

    return sqrtf(square_radius);
}

//
//
//
float fc3d_PolygonMesh_InfRadiusWithTransform(void const* obj, owl_v3f32 v_pos, owl_q32 q_rot)
{
    fc3d_PolygonMesh const* mesh = obj;
    float inf_radius = 0.0;

    for(unsigned int k = 0 ; k < mesh->nb_faces ; k++)
    {
        for(unsigned int vi = 0 ; vi < 3 ; vi++)
        {
            owl_v3f32 vertex = mesh->face_list[k].triangle3d.vertex_list[vi];
            vertex = owl_v3f32_add(v_pos, owl_q32_transform_v3f32(q_rot, vertex));
            inf_radius = fmaxf(
                                inf_radius,
                                owl_v3f32_norminf(vertex)
                               );
        }
    }

    return inf_radius;
}

