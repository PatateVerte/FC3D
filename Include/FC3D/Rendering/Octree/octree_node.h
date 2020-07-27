#ifndef FC3D_OCTREE_NODE_H_INCLUDED
#define FC3D_OCTREE_NODE_H_INCLUDED

#include <stdbool.h>

#include <OWL/Optimized3d/quaternion/q32.h>
#include <OWL/Optimized3d/vector/v3f32.h>

#include <FC3D/Rendering/RenderingObject/rendering_object.h>
#include <FC3D/DataStruct/data_pool.h>

#include <FC3D/Rendering/Image/image3d.h>

#include <WF3D/Rendering/camera3d.h>

#define FC3D_OCTREE_NODE_NB_OBJECTS 8
#define FC3D_OCTREE_MAX_DEPTH 8

typedef struct fc3d_rendering_octree_node fc3d_rendering_octree_node;
struct fc3d_rendering_octree_node
{
    //Center of the node
    owl_v3f32 center;

    //Half Size of the node (as a square)
    float half_size;

    /*
        i   x   y   z
        =============
        0   -   -   -
        1   +   -   -
        2   -   +   -
        3   +   +   -
        4   -   -   +
        5   +   -   +
        6   -   +   +
        7   +   +   +
    */
    fc3d_rendering_octree_node* children[8];

    //Objects contained in the node
    fc3d_rendering_object const* objects[FC3D_OCTREE_NODE_NB_OBJECTS];

    //Secondary storage node if the number of objects exceeds the storage capacity
    fc3d_rendering_octree_node* auxiliary_storage_node;
};

//Set up a new node
fc3d_rendering_octree_node* fc3d_rendering_octree_node_Set(fc3d_rendering_octree_node* node, owl_v3f32 center, float half_size);

//Activates child i (0 <= i <= 7)
//Return a pointer to the activated child
fc3d_rendering_octree_node* fc3d_rendering_octree_node_ChildrenOn(fc3d_rendering_octree_node* node, int i, fc3d_DataPool* octree_children_data_pool);

//Activates auxiliary storage
//Return a pointer to the auxiliary node created
fc3d_rendering_octree_node* fc3d_rendering_octree_node_AuxStorageOn(fc3d_rendering_octree_node* node, fc3d_DataPool* octree_auxiliary_data_pool);

//Insert an object into the node
fc3d_rendering_octree_node* fc3d_rendering_octree_node_InsertObject(fc3d_rendering_octree_node* node, fc3d_rendering_object* obj, fc3d_DataPool* octree_auxiliary_data_pool);

//Add an object to the node or a child if it fits
//Return NULL if the object wasn't added
//Return a pointer to the node where it was added otherwise
fc3d_rendering_octree_node* fc3d_rendering_octree_node_AddObject(fc3d_rendering_octree_node* node, fc3d_rendering_object* obj, int max_depth, bool spatial_extension, fc3d_DataPool* octree_children_data_pool, fc3d_DataPool* octree_auxiliary_data_pool);



//The intersection between a ray and the objects of the node
//Return true if the intersection exists, false otherwise
//Optional parameters (modified only if an intersection has been found) :
//t to return the parameter for the nearest intersection (v_intersection = ray_origin + t*ray_dir)
//normal_ret to return the normal of the intersection
//surface_ret to return the surface of the intersection
bool fc3d_rendering_octree_node_NearestIntersectionWithRay(fc3d_rendering_octree_node* node, owl_v3f32 octree_v_pos, owl_q32 octree_q_rot, owl_v3f32 ray_origin, owl_v3f32 ray_dir, float t_min, float t_max, float* t_ret, owl_v3f32* normal_ret, wf3d_surface const** surface_ret, wf3d_color* diffusion_color_ret);

//Rasterization
void fc3d_rendering_octree_node_Rasterization(fc3d_rendering_octree_node* node, fc3d_Image3d* img3d, wf3d_rasterization_rectangle const* rect, owl_v3f32 octree_v_pos, owl_q32 octree_q_rot, wf3d_camera3d const* cam);

#endif // FC3D_OCTREE_NODE_H_INCLUDED
