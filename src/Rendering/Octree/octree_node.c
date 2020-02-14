#include <FC3D/Rendering/Octree/octree_node.h>

//Set up a new node
//
//
fc3d_rendering_octree_node* fc3d_rendering_octree_node_Set(fc3d_rendering_octree_node* node, wf3d_vect3d center, float half_size)
{
    if(node == NULL)
    {
        return NULL;
    }

    node->center = center;
    node->half_size = half_size;

    node->children = NULL;

    for(int k = 0 ; k < FC3D_OCTREE_NODE_NB_OBJECTS ; k++)
    {
        node->objects[k] = NULL;
    }

    node->auxiliary_storage_node = NULL;

    return node;
}

//Activates children
//Return a pointer to the first child
//
fc3d_rendering_octree_node* fc3d_rendering_octree_node_ChildrenOn(fc3d_rendering_octree_node* node, fc3d_DataPool* octree_children_data_pool)
{
    if(node == NULL)
    {
        return NULL;
    }

    if(node->children == NULL)
    {
        node->children = fc3d_DataPool_NextData(octree_children_data_pool);

        if(node->children != NULL)
        {
            wf3d_vect3d const node_center = node->center;
            float const half_size = node->half_size;
            float const child_half_size = 0.5f * half_size;

            wf3d_vect3d base_xyz[3];
            wf3d_vect3d_base_xyz(base_xyz, half_size);

            for(int k = 0 ; k < 8 ; k++)
            {
                wf3d_vect3d child_center = wf3d_vect3d_sub(node_center, wf3d_vect3d_broadcast(child_half_size));
                for(int b = k ; b != 0 ; b >>= 1)
                {
                    if((b & 1) != 0)
                    {
                        wf3d_vect3d_sub(child_center, base_xyz[b]);
                    }
                }

                fc3d_rendering_octree_node_Set(node->children + k, child_center, child_half_size);
            }
        }
    }

    return node->children;
}

//Activates auxiliary storage
//Return a pointer to the auxiliary node created
//
fc3d_rendering_octree_node* fc3d_rendering_octree_node_AuxStorageOn(fc3d_rendering_octree_node* node, fc3d_DataPool* octree_auxiliary_data_pool)
{
    if(node == NULL)
    {
        return NULL;
    }

    if(node->auxiliary_storage_node == NULL)
    {
        node->auxiliary_storage_node = fc3d_DataPool_NextData(octree_auxiliary_data_pool);
        fc3d_rendering_octree_node_Set(node->auxiliary_storage_node, node->center, node->half_size);
    }

    return node->auxiliary_storage_node;
}

//Insert an object into the node
//
//
fc3d_rendering_octree_node* fc3d_rendering_octree_node_InsertObject(fc3d_rendering_octree_node* node, fc3d_rendering_object* obj, fc3d_DataPool* octree_auxiliary_data_pool)
{
    if(node == NULL)
    {
        return NULL;
    }

    bool has_been_inserted = false;
    for(int k = 0 ; k < FC3D_OCTREE_NODE_NB_OBJECTS && !has_been_inserted ; k++)
    {
        if(node->objects[k] == NULL)
        {
            node->objects[k] = obj;
            has_been_inserted = true;
        }
    }

    if(has_been_inserted)
    {
        return node;
    }
    else
    {
        fc3d_rendering_octree_node* aux_node = fc3d_rendering_octree_node_AuxStorageOn(node, octree_auxiliary_data_pool);
        return fc3d_rendering_octree_node_InsertObject(aux_node, obj, octree_auxiliary_data_pool);
    }
}

//Add an object to the node or the children
//
//
fc3d_rendering_octree_node* fc3d_rendering_octree_node_AddObject(fc3d_rendering_octree_node* node, fc3d_rendering_object* obj, int max_depth, bool spatial_extension, fc3d_DataPool* octree_children_data_pool, fc3d_DataPool* octree_auxiliary_data_pool)
{
    if(node == NULL)
    {
        return NULL;
    }

    if(max_depth <= 0)
    {
        return fc3d_rendering_octree_node_InsertObject(node, obj, octree_auxiliary_data_pool);
    }
    else
    {
        int i_node = wf3d_vect3d_sign_mask( wf3d_vect3d_sub(obj->v_pos, node->center) );

        //Does it fit completely into i_node
        bool fit_into_i_node = false;

        if(spatial_extension)
        {
            wf3d_vect3d const obj_radius_vect = wf3d_vect3d_broadcast( obj->wolf_obj_interface->Radius(obj->wolf_obj) );
            wf3d_vect3d const rel_center = wf3d_vect3d_sub(obj->v_pos, node->center);
            float quick_test_result = fmaxf(
                                                wf3d_vect3d_inf_norm( wf3d_vect3d_add( rel_center, obj_radius_vect ) ),
                                                wf3d_vect3d_inf_norm( wf3d_vect3d_sub( rel_center, obj_radius_vect ) )
                                            );
            if(quick_test_result <= node->half_size)
            {
                fit_into_i_node = true;
            }
            else
            {
                //More advanced test
                float advanced_test_result = obj->wolf_obj_interface->InfRadiusWithRot(obj->wolf_obj, obj->v_pos, obj->q_rot);
                if(advanced_test_result <= node->half_size)
                {
                    fit_into_i_node = true;
                }
            }
        }
        else
        {
            fit_into_i_node = true;
        }

        if(fit_into_i_node)
        {
            fc3d_rendering_octree_node* children = fc3d_rendering_octree_node_ChildrenOn(node, octree_children_data_pool);

            if(children != NULL)
            {
                return fc3d_rendering_octree_node_AddObject(children + i_node, obj, max_depth - 1, spatial_extension, octree_children_data_pool, octree_auxiliary_data_pool);
            }
            else
            {
                return NULL;
            }
        }
        else
        {
            return fc3d_rendering_octree_node_InsertObject(node, obj, octree_auxiliary_data_pool);
        }
    }
}

