#include <FC3D/Rendering/Octree/octree_node.h>

//Set up a new node
//
//
fc3d_rendering_octree_node* fc3d_rendering_octree_node_Set(fc3d_rendering_octree_node* node, owl_v3f32 center, float half_size)
{
    node->center = center;

    node->half_size = half_size;

    for(unsigned int k = 0 ; k < 8 ; k++)
    {
        node->children[k] = NULL;
    }

    for(unsigned int k = 0 ; k < FC3D_OCTREE_NODE_NB_OBJECTS ; k++)
    {
        node->objects[k] = NULL;
    }

    node->auxiliary_storage_node = NULL;

    return node;
}

//Activates children i
//Return a pointer to the first child
//
fc3d_rendering_octree_node* fc3d_rendering_octree_node_ChildrenOn(fc3d_rendering_octree_node* node, int i, fc3d_DataPool* octree_children_data_pool)
{
    if(0 <= i && i < 8)
    {
        if(node->children[i] == NULL)
        {
            node->children[i] = fc3d_DataPool_NextData(octree_children_data_pool);

            if(node->children[i] != NULL)
            {
                float const half_size = node->half_size;
                float const child_half_size = 0.5 * half_size;
                owl_v3f32 child_center = node->center;

                owl_v3f32 base_xyz[3];
                owl_v3f32_base_xyz(base_xyz, child_half_size);

                for(unsigned int j = 0 ; j < 3 ; j++)
                {
                    child_center = owl_v3f32_add(
                                                    child_center,
                                                    owl_v3f32_scalar_mul(
                                                                            base_xyz[j],
                                                                            (((i>>j) & 0x1) != 0) ? 1.0 : -1.0
                                                                         )
                                                 );
                }

                fc3d_rendering_octree_node_Set(node->children[i], child_center, child_half_size);
            }
        }

        return node->children[i];
    }
    else
    {
        return NULL;
    }
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
        int i_node = owl_v3f32_sign_mask(owl_v3f32_sub(obj->v_pos, node->center));

        //Does the object fit into the child node
        bool fit_into_child_node = false;
        float child_half_size = 0.5 * node->half_size;

        owl_v3f32 child_center = node->center;
        owl_v3f32 base_xyz[3];
        owl_v3f32_base_xyz(base_xyz, child_half_size);
        for(unsigned int j = 0 ; j < 3 ; j++)
        {
            child_center = owl_v3f32_add_scalar_mul(
                                                        child_center,
                                                        base_xyz[j],
                                                        (((i_node >> j) & 0x1) != 0) ? 1.0 : -1.0
                                                    );
        }

        owl_v3f32 rel_child_center = owl_v3f32_sub(obj->v_pos, child_center);
        if(spatial_extension)
        {
            float obj_radius = obj->rendering_obj_interface->Radius(obj->obj);
            owl_v3f32 obj_radius_vect = owl_v3f32_broadcast(obj_radius);
            float quick_test_result = fmaxf(
                                                owl_v3f32_norminf( owl_v3f32_add( rel_child_center, obj_radius_vect ) ),
                                                owl_v3f32_norminf( owl_v3f32_sub( rel_child_center, obj_radius_vect ) )
                                            );

            if(quick_test_result <= child_half_size)
            {
                fit_into_child_node = true;
            }
            else if(obj_radius <= 2.0 * child_half_size)
            {
                //Precise test
                float advanced_test_result = obj->rendering_obj_interface->InfRadiusWithTransform(obj->obj, rel_child_center, obj->q_rot);
                if(advanced_test_result <= child_half_size)
                {
                    fit_into_child_node = true;
                }
            }
        }
        else
        {
            fit_into_child_node = true;
        }

        if(fit_into_child_node)
        {
            fc3d_rendering_octree_node* child = fc3d_rendering_octree_node_ChildrenOn(node, i_node, octree_children_data_pool);
            if(child != NULL)
            {
                return fc3d_rendering_octree_node_AddObject(child, obj, max_depth - 1, spatial_extension, octree_children_data_pool, octree_auxiliary_data_pool);
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

//
//
//
bool fc3d_rendering_octree_node_NearestIntersectionWithRay(fc3d_rendering_octree_node* node, owl_v3f32 octree_v_pos, owl_q32 octree_q_rot, owl_v3f32 ray_origin, owl_v3f32 ray_dir, float t_min, float t_max, float* t_ret, owl_v3f32* normal_ret, wf3d_surface* surface_ret)
{
    bool intersection_exists = false;
    float t = t_max;

    float node_center_coords[4] OWL_ALIGN16;
    owl_v3f32_store4(node_center_coords, node->center);

    bool node_has_intersection_with_ray = false;
    owl_q32 octree_q_rot_conj = owl_q32_conj(octree_q_rot);

    owl_v3f32 rel_ray_origin = owl_q32_transform_v3f32(octree_q_rot_conj, owl_v3f32_sub(ray_origin, octree_v_pos));
    float rel_ray_origin_coords[4] OWL_ALIGN16;
    owl_v3f32_store4(rel_ray_origin_coords, rel_ray_origin);

    owl_v3f32 rel_ray_dir = owl_q32_transform_v3f32(octree_q_rot_conj, ray_dir);
    float rel_ray_dir_coords[4] OWL_ALIGN16;
    owl_v3f32_store4(rel_ray_dir_coords, rel_ray_dir);

    for(unsigned int j = 0 ; j < 3 && !node_has_intersection_with_ray ; j++)
    {
        unsigned int j1 = (j + 1) % 3;
        unsigned int j2 = (j + 2) % 3;

        for(float sign = -1.0 ; sign <= 1.0 && !node_has_intersection_with_ray ; sign += 2.0)
        {
            float t_plan = (node_center_coords[j] + sign * node->half_size - rel_ray_origin_coords[j]) / rel_ray_dir_coords[j];
            if(t_min <= t_plan && t_plan <= t_max && isfinite(t_plan) != 0)
            {
                float x_j1 = rel_ray_origin_coords[j1] + t_plan * rel_ray_dir_coords[j1] - node_center_coords[j1];
                float x_j2 = rel_ray_origin_coords[j2] + t_plan * rel_ray_dir_coords[j2] - node_center_coords[j2];
                if(fmaxf(fabsf(x_j1), fabsf(x_j2)) <= node->half_size)
                {
                    node_has_intersection_with_ray = true;
                }
            }
        }
    }

    if(node_has_intersection_with_ray)
    {
        for(unsigned int k = 0 ; k < FC3D_OCTREE_NODE_NB_OBJECTS ; k++)
        {
            fc3d_rendering_object const* rendering_obj = node->objects[k];
            if(rendering_obj != NULL)
            {
                owl_q32 full_q_rot = owl_q32_mul(
                                                    octree_q_rot,
                                                    rendering_obj->q_rot
                                                 );
                owl_v3f32 full_v_pos = owl_v3f32_add(
                                                        octree_v_pos,
                                                        owl_q32_transform_v3f32(octree_q_rot, rendering_obj->v_pos)
                                                     );
                intersection_exists = rendering_obj->rendering_obj_interface->NearestIntersectionWithRay(rendering_obj->obj, full_v_pos, full_q_rot, ray_origin, ray_dir, t_min, t, &t, normal_ret, surface_ret) || intersection_exists;
            }
        }

        if(node->auxiliary_storage_node != NULL)
        {
            intersection_exists = fc3d_rendering_octree_node_NearestIntersectionWithRay(node->auxiliary_storage_node, octree_v_pos, octree_q_rot, ray_origin, ray_dir, t_min, t, &t, normal_ret, surface_ret) || intersection_exists;
        }

        if(node->children != NULL)
        {
            for(unsigned int k = 0 ; k < 8 ; k++)
            {
                if(node->children[k] != NULL)
                {
                    intersection_exists = fc3d_rendering_octree_node_NearestIntersectionWithRay(node->children[k], octree_v_pos, octree_q_rot, ray_origin, ray_dir, t_min, t, &t, normal_ret, surface_ret) || intersection_exists;
                }
            }
        }
    }

    if(intersection_exists)
    {
        if(t_ret != NULL)
        {
            *t_ret = t;
        }
    }

    return intersection_exists;
}

//Rasterization
//
//
void fc3d_rendering_octree_node_Rasterization(fc3d_rendering_octree_node* node, fc3d_Image3d* img3d, wf3d_rasterization_rectangle const* rect, owl_v3f32 octree_v_pos, owl_q32 octree_q_rot, wf3d_camera3d const* cam)
{
    //Is the node inside of the view cone ? (the node is considered to be a sphere)
    bool is_inside_view_cone = true;
    owl_v3f32 node_pos = owl_v3f32_add(octree_v_pos, owl_q32_transform_v3f32(octree_q_rot, node->center));
    if(owl_v3f32_dot(node_pos, node_pos) > 3.0 * node->half_size * node->half_size)
    {
        float zf = owl_v3f32_unsafe_get_component(node_pos, 2);
        float abs_xf = fabsf(owl_v3f32_unsafe_get_component(node_pos, 0) / zf);
        float abs_yf = fabsf(owl_v3f32_unsafe_get_component(node_pos, 1) / zf);

        if(zf >= 0.0 || (abs_xf > cam->tan_h_half_opening_angle || abs_yf > cam->tan_v_half_opening_angle))
        {
            float width_f = (float)rect->width;
            float x_minus = (((float)rect->x_min) - 0.5 * width_f) * cam->tan_h_half_opening_angle / width_f;
            float x_plus = (((float)rect->x_max) - 0.5 * width_f) * cam->tan_h_half_opening_angle / width_f;
            float height_f = (float)rect->height;
            float y_minus = (((float)rect->y_min) - 0.5 * height_f) * cam->tan_v_half_opening_angle / height_f;
            float y_plus = (((float)rect->y_max) - 0.5 * height_f) * cam->tan_v_half_opening_angle / height_f;

            owl_v3f32 edges_dir_vect[4] =
            {
                owl_v3f32_set(x_minus, y_minus, -1.0),
                owl_v3f32_set(x_plus, y_minus, -1.0),
                owl_v3f32_set(x_plus, y_plus, -1.0),
                owl_v3f32_set(x_minus, y_plus, -1.0)
            };

            float square_edge_distance = INFINITY;
            float face_distance_list[4];
            for(unsigned int i = 0 ; i < 4 ; i++)
            {
                float lambda_proj = fmaxf(0.0, owl_v3f32_dot(node_pos, edges_dir_vect[i])) / owl_v3f32_dot(edges_dir_vect[i], edges_dir_vect[i]);
                owl_v3f32 edge_proj = owl_v3f32_scalar_mul(edges_dir_vect[i], lambda_proj);
                owl_v3f32 edge_diff = owl_v3f32_sub(node_pos, edge_proj);
                square_edge_distance = fminf(
                                                square_edge_distance,
                                                owl_v3f32_dot(edge_diff, edge_diff)
                                             );

                unsigned int i_next = (i + 1) % 4;
                owl_v3f32 n = owl_v3f32_normalize(owl_v3f32_cross(edges_dir_vect[i], edges_dir_vect[i_next]));
                face_distance_list[i] = owl_v3f32_dot(n, node_pos);
            }

            float face_distance = INFINITY;
            for(unsigned int i = 0 ; i < 4 ; i++)
            {
                unsigned int i_next1 = (i + 1) % 4;
                unsigned int i_next2 = (i + 3) % 4;
                unsigned int i_opp = (i + 2) % 4;
                if(face_distance_list[i_opp] < 0.0 && face_distance_list[i_next1] < 0.0 && face_distance_list[i_next2] < 0.0)
                {
                    face_distance = fminf(face_distance, face_distance_list[i]);
                }
            }

            float square_distance = fminf(face_distance * face_distance, square_edge_distance);
            if(square_distance > 3 * node->half_size * node->half_size)
            {
                is_inside_view_cone = false;
            }
        }
    }

    if(is_inside_view_cone)
    {
        //Rasterization of the objects
        for(unsigned int i = 0 ; i < FC3D_OCTREE_NODE_NB_OBJECTS ; i++)
        {
            fc3d_rendering_object const* rendering_obj = node->objects[i];

            if(rendering_obj != NULL)
            {
                owl_q32 full_q_rot = owl_q32_mul(
                                                    octree_q_rot,
                                                    rendering_obj->q_rot
                                                 );
                owl_v3f32 full_v_pos = owl_v3f32_add(
                                                        octree_v_pos,
                                                        owl_q32_transform_v3f32(octree_q_rot, rendering_obj->v_pos)
                                                     );

                rendering_obj->rendering_obj_interface->Rasterization(rendering_obj->obj,img3d, rect , full_v_pos, full_q_rot, cam);
            }
        }

        //Rasterization of the auxiliary storage node
        if(node->auxiliary_storage_node != NULL)
        {
            fc3d_rendering_octree_node_Rasterization(node->auxiliary_storage_node, img3d, rect, octree_v_pos, octree_q_rot, cam);
        }

        //Rasterization of the children
        for(unsigned int k = 0 ; k < 8 ; k++)
        {
            if(node->children[k] != NULL)
            {
                fc3d_rendering_octree_node_Rasterization(node->children[k], img3d, rect, octree_v_pos, octree_q_rot, cam);
            }
        }
    }
}

