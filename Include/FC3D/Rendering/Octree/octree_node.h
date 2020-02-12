#ifndef FC3D_OCTREE_NODE_H_INCLUDED
#define FC3D_OCTREE_NODE_H_INCLUDED

#include <FC3D/Rendering/wolf_object_interface.h>

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
    size_t nb_objects;
    fc3d_wolf_object const* objects[FC3D_OCTREE_NODE_NB_OBJECTS];

    //Secondary storage node if the number of objects exceeds the storage capacity
    fc3d_octree_node* auxiliary_storage_node;
};

//


#endif // FC3D_OCTREE_NODE_H_INCLUDED
