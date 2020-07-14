#ifndef FC3D_OBJECT_INTERFACE_H_INCLUDED
#define FC3D_OBJECT_INTERFACE_H_INCLUDED

#include <OWL/q32.h>
#include <OWL/v3f32.h>

#include <WF3D/error.h>
#include <WF3D/Rendering/camera3d.h>
#include <WF3D/Rendering/lightsource.h>
#include <WF3D/Rendering/Design/image2d.h>
#include <WF3D/Rendering/Design/image3d.h>

typedef struct
{
    //Destroy() parameter
    void (*Destroy)(void*);

    //wf3d_error wf3d_PolygonMesh_Rasterization(wf3d_PolygonMesh const* obj, wf3d_image2d_rectangle* img_out, wf3d_lightsource const* lightsource_list, unsigned int nb_lightsources, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam)
    wf3d_error (*Rasterization)(void const*, wf3d_image2d_rectangle*, wf3d_lightsource const*, unsigned int, owl_v3f32, owl_q32, wf3d_camera3d const*);

    //float Radius(void const* obj)
    float (*Radius)(void const*);

    //float wf3d_PolygonMesh_InfRadius(void const* obj, owl_v3f32 v_pos)
    float (*InfRadius)(void const*, owl_v3f32);

    //float wf3d_PolygonMesh_InfRadiusWithRot(void const* obj, owl_v3f32 v_pos, owl_q32 q_rot)
    float (*InfRadiusWithRot)(void const*, owl_v3f32, owl_q32);

} fc3d_wolf_object_interface;

#endif // FC3D_OBJECT_INTERFACE_H_INCLUDED
