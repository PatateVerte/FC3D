#ifndef FC3D_OCTREE_NODE_H_INCLUDED
#define FC3D_OCTREE_NODE_H_INCLUDED

#include <stdbool.h>

#include <FC3D/error.h>
#include <FC3D/Rendering/wolf_object_interface.h>
#include <FC3D/DataStruct/data_pool.h>

#include <WF3D/Geometry/vect3d.h>

#define FC3D_OCTREE_NODE_NB_OBJECTS 8
#define FC3D_OCTREE_MAX_DEPTH 8

typedef struct fc3d_octree_node fc3d_octree_node;
struct fc3d_octree_node
{
    //Center of the node
    wf3d_vect3d center;
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
    fc3d_octree_node* children;

    //Objects contained in the node
    fc3d_wolf_object const* objects[FC3D_OCTREE_NODE_NB_OBJECTS];

    //Secondary storage node if the number of objects exceeds the storage capacity
    fc3d_octree_node* auxiliary_storage_node;
};

//Set up a new node
fc3d_octree_node* fc3d_octree_node_Set(fc3d_octree_node* node, wf3d_vect3d center, float half_size);

//Activates children
//Return a pointer to the first child
fc3d_octree_node* fc3d_octree_node_ChildrenOn(fc3d_octree_node* node, fc3d_DataPool* octree_children_data_pool);

//Activates auxiliary storage
//Return a pointer to the auxiliary node created
fc3d_octree_node* fc3d_octree_node_AuxStorageOn(fc3d_octree_node* node, fc3d_DataPool* octree_auxiliary_data_pool);

//Insert an object into the node
fc3d_wolf_object* fc3d_octree_node_InsertObject(fc3d_octree_node* node, fc3d_wolf_object* obj, fc3d_DataPool* octree_auxiliary_data_pool);

//Add an object to the node or the children
fc3d_wolf_object* fc3d_octree_node_AddObject(fc3d_octree_node* node, fc3d_wolf_object* obj, wf3d_vect3d v_pos, wf3d_quat q_rot, int max_depth, fc3d_DataPool* octree_children_data_pool, fc3d_DataPool* octree_auxiliary_data_pool);

//Add an object with no spatial extension
fc3d_wolf_object* fc3d_octree_node_AddObjectWithoutExtension(fc3d_octree_node* node, fc3d_wolf_object* obj, wf3d_vect3d v_pos, int max_depth, fc3d_DataPool* octree_children_data_pool, fc3d_DataPool* octree_auxiliary_data_pool);

#endif // FC3D_OCTREE_NODE_H_INCLUDED
