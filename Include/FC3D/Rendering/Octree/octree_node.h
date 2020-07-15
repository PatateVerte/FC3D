#ifndef FC3D_OCTREE_NODE_H_INCLUDED
#define FC3D_OCTREE_NODE_H_INCLUDED

#include <stdbool.h>

#include <OWL/q32.h>
#include <OWL/v3f32.h>

#include <FC3D/Rendering/rendering_object.h>
#include <FC3D/DataStruct/data_pool.h>

#include <WF3D/Rendering/camera3d.h>
#include <WF3D/Rendering/Design/image2d.h>
#include <WF3D/Rendering/Design/image3d.h>
#include <WF3D/Rendering/Design/image_gen_interface.h>

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
    fc3d_rendering_octree_node* children;

    //Objects contained in the node
    fc3d_rendering_object const* objects[FC3D_OCTREE_NODE_NB_OBJECTS];

    //Secondary storage node if the number of objects exceeds the storage capacity
    fc3d_rendering_octree_node* auxiliary_storage_node;
};

//Set up a new node
fc3d_rendering_octree_node* fc3d_rendering_octree_node_Set(fc3d_rendering_octree_node* node, owl_v3f32 center, float half_size);

//Activates children
//Return a pointer to the first child
fc3d_rendering_octree_node* fc3d_rendering_octree_node_ChildrenOn(fc3d_rendering_octree_node* node, fc3d_DataPool* octree_children_data_pool);

//Activates auxiliary storage
//Return a pointer to the auxiliary node created
fc3d_rendering_octree_node* fc3d_rendering_octree_node_AuxStorageOn(fc3d_rendering_octree_node* node, fc3d_DataPool* octree_auxiliary_data_pool);

//Insert an object into the node
fc3d_rendering_octree_node* fc3d_rendering_octree_node_InsertObject(fc3d_rendering_octree_node* node, fc3d_rendering_object* obj, fc3d_DataPool* octree_auxiliary_data_pool);

//Add an object to the node or a child if it fits
//Return NULL if the object wasn't added
//Return a pointer to the node where it was added otherwise
fc3d_rendering_octree_node* fc3d_rendering_octree_node_AddObject(fc3d_rendering_octree_node* node, fc3d_rendering_object* obj, int max_depth, bool spatial_extension, fc3d_DataPool* octree_children_data_pool, fc3d_DataPool* octree_auxiliary_data_pool);

//Rasterization
wf3d_error fc3d_rendering_octree_node_Rasterization(fc3d_rendering_octree_node* node, wf3d_image2d_rectangle* img_out, wf3d_lightsource const* cam_lightsource_list, unsigned int nb_lightsources, owl_v3f32 cam_v_pos, owl_q32 cam_q_rot, wf3d_camera3d const* cam);

//Rasterization2
wf3d_error fc3d_rendering_octree_node_Rasterization2(fc3d_rendering_octree_node* node, wf3d_image3d_image_piece* img_out, owl_v3f32 cam_v_pos, owl_q32 cam_q_rot, wf3d_camera3d const* cam);

#endif // FC3D_OCTREE_NODE_H_INCLUDED
