#include <FC3D/Rendering/Octree/octree_node.h>

//Set up a new node
//
//
fc3d_rendering_octree_node* fc3d_rendering_octree_node_Set(fc3d_rendering_octree_node* node, owl_v3f32 center, float half_size)
{
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
    if(node->children == NULL)
    {
        node->children = fc3d_DataPool_NextData(octree_children_data_pool);

        if(node->children != NULL)
        {
            owl_v3f32 const node_center = node->center;
            float const half_size = node->half_size;
            float const children_half_size = 0.5 * half_size;

            owl_v3f32 base_xyz[3];
            owl_v3f32_base_xyz(base_xyz, children_half_size);

            unsigned int k = 0;
            for(float sign_z = -1.0 ; sign_z <= 1.0 ; sign_z += 2.0)
            {
                owl_v3f32 center_z = owl_v3f32_add_scalar_mul(node_center, base_xyz[2], sign_z);
                for(float sign_y = -1.0 ; sign_y <= 1.0 ; sign_y += 2.0)
                {
                    owl_v3f32 center_y = owl_v3f32_add_scalar_mul(center_z, base_xyz[1], sign_y);
                    for(float sign_x = -1.0 ; sign_x <= 1.0 ; sign_x += 2.0)
                    {
                        owl_v3f32 center = owl_v3f32_add_scalar_mul(center_y, base_xyz[0], sign_x);

                        fc3d_rendering_octree_node_Set(node->children + k, center, children_half_size);

                        k += 1;
                    }
                }
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
        if(aux_node != NULL)
        {
            return fc3d_rendering_octree_node_InsertObject(aux_node, obj, octree_auxiliary_data_pool);
        }
        else
        {
            return NULL;
        }
    }
}

//Add an object to the node or the children
//
//
fc3d_rendering_octree_node* fc3d_rendering_octree_node_AddObject(fc3d_rendering_octree_node* node, fc3d_rendering_object* obj, int max_depth, bool spatial_extension, fc3d_DataPool* octree_children_data_pool, fc3d_DataPool* octree_auxiliary_data_pool)
{
    if(max_depth > 0)
    {
        //Does it fit completely into the node
        bool fit_into_node = false;
        owl_v3f32 rel_center = owl_v3f32_sub(obj->v_pos, node->center);

        if(spatial_extension)
        {
            owl_v3f32 obj_radius_vect = owl_v3f32_broadcast( obj->wolf_obj_interface->Radius(obj->wolf_obj) );
            float quick_test_result = fmaxf(
                                                owl_v3f32_norminf( owl_v3f32_add( rel_center, obj_radius_vect ) ),
                                                owl_v3f32_norminf( owl_v3f32_sub( rel_center, obj_radius_vect ) )
                                            );

            if(quick_test_result <= node->half_size)
            {
                fit_into_node = true;
            }
            else
            {
                //Precise test
                float advanced_test_result = obj->wolf_obj_interface->InfRadiusWithRot(obj->wolf_obj, rel_center, obj->q_rot);
                if(advanced_test_result <= node->half_size)
                {
                    fit_into_node = true;
                }
            }
        }
        else
        {
            fit_into_node = true;
        }

        if(fit_into_node)
        {
            fc3d_rendering_octree_node* children = fc3d_rendering_octree_node_ChildrenOn(node, octree_children_data_pool);

            if(children != NULL)
            {
                int i_node = owl_v3f32_sign_mask(rel_center);
                fc3d_rendering_octree_node* obj_node = fc3d_rendering_octree_node_AddObject(children + i_node, obj, max_depth - 1, spatial_extension, octree_children_data_pool, octree_auxiliary_data_pool);
                if(obj_node != NULL)
                {
                    return obj_node;
                }
                else
                {
                    return fc3d_rendering_octree_node_InsertObject(node, obj, octree_auxiliary_data_pool);
                }
            }
            else
            {
                return fc3d_rendering_octree_node_InsertObject(node, obj, octree_auxiliary_data_pool);
            }
        }
        else
        {
            return fc3d_rendering_octree_node_InsertObject(node, obj, octree_auxiliary_data_pool);
        }
    }
    else
    {
        return fc3d_rendering_octree_node_InsertObject(node, obj, octree_auxiliary_data_pool);
    }
}

//Rasterization
//
//
wf3d_error fc3d_rendering_octree_node_Rasterization(fc3d_rendering_octree_node* node, wf3d_Image2d* img_out, wf3d_lightsource const* cam_lightsource_list, unsigned int nb_lightsources, owl_v3f32 cam_v_pos, owl_q32 cam_q_rot, wf3d_camera3d const* cam)
{
    wf3d_error error = WF3D_SUCCESS;
    owl_q32 const cam_q_rot_conj = owl_q32_conj(cam_q_rot);

    //Rasterization of the objects
    for(int i = 0 ; i < FC3D_OCTREE_NODE_NB_OBJECTS && error == WF3D_SUCCESS ; i++)
    {
        fc3d_rendering_object const* obj = node->objects[i];

        if(obj != NULL)
        {
            owl_q32 rel_q_rot = owl_q32_mul(
                                                    cam_q_rot_conj,
                                                    obj->q_rot
                                                );
            owl_v3f32 rel_v_pos = owl_q32_transform_v3f32(
                                                                cam_q_rot_conj,
                                                                owl_v3f32_sub( obj->v_pos, cam_v_pos )
                                                               );

            error = obj->wolf_obj_interface->Rasterization(obj->wolf_obj, img_out, cam_lightsource_list, nb_lightsources, rel_v_pos, rel_q_rot, cam);
        }
    }

    //Rasterization of the auxiliary storage node
    if(error == WF3D_SUCCESS && node->auxiliary_storage_node != NULL)
    {
        error = fc3d_rendering_octree_node_Rasterization(node->auxiliary_storage_node, img_out, cam_lightsource_list, nb_lightsources, cam_v_pos, cam_q_rot, cam);
    }

    //Rasterization of the children
    if(error == WF3D_SUCCESS && node->children != NULL)
    {
        for(int i = 0 ; i < 8 && error == WF3D_SUCCESS; i++)
        {
            //Are the children in the view cone ?
            error = fc3d_rendering_octree_node_Rasterization(node->children + i, img_out, cam_lightsource_list, nb_lightsources, cam_v_pos, cam_q_rot, cam);
        }
    }

    return error;
}

