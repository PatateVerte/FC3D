#include <FC3D/Rendering/Octree/octree.h>

//Create an RenderingOctree
//
//
fc3d_RenderingOctree* fc3d_RenderingOctree_Create(owl_v3f32 center, float half_size, int max_depth, size_t children_data_pool_block_len, size_t auxiliary_data_pool_block_len)
{
    fc3d_RenderingOctree* octree = malloc(sizeof(*octree));

    if(octree != NULL)
    {
        octree->center = center;
        octree->half_size = half_size;

        octree->max_depth = max_depth;

        octree->octree_children_data_pool = fc3d_DataPool_Create(children_data_pool_block_len, 8 * sizeof(fc3d_rendering_octree_node));
        octree->octree_auxiliary_data_pool = fc3d_DataPool_Create(auxiliary_data_pool_block_len, sizeof(fc3d_rendering_octree_node));

        if(octree->octree_children_data_pool == NULL || octree->octree_children_data_pool == NULL)
        {
            fc3d_RenderingOctree_Destroy(octree);
            octree = NULL;
        }
        else
        {
            fc3d_RenderingOctree_Rewind(octree);
        }
    }

    return octree;
}

//Destroy an RenderingOctree
//
//
void fc3d_RenderingOctree_Destroy(fc3d_RenderingOctree* octree)
{
    if(octree != NULL)
    {
        fc3d_DataPool_Destroy(octree->octree_children_data_pool);
        fc3d_DataPool_Destroy(octree->octree_auxiliary_data_pool);

        free(octree);
    }
}

//Rewind an octree
//Cannot fail
//
fc3d_rendering_octree_node* fc3d_RenderingOctree_Rewind(fc3d_RenderingOctree* octree)
{
    if(octree == NULL)
    {
        return NULL;
    }

    octree->node_0 = fc3d_DataPool_Rewind(octree->octree_children_data_pool);
    fc3d_DataPool_Rewind(octree->octree_auxiliary_data_pool);

    return fc3d_rendering_octree_node_Set(octree->node_0, octree->center, octree->half_size);
}

//Add an object into the octree
//
//
fc3d_rendering_octree_node* fc3d_RenderingOctree_AddObject(fc3d_RenderingOctree* octree, fc3d_rendering_object* obj, bool spatial_extension)
{
    if(octree == NULL)
    {
        return NULL;
    }

    return fc3d_rendering_octree_node_AddObject(octree->node_0, obj, octree->max_depth, spatial_extension, octree->octree_children_data_pool, octree->octree_auxiliary_data_pool);
}

//Rasterization
//
//
fc3d_error fc3d_RenderingOctree_Rasterization(fc3d_RenderingOctree* octree, wf3d_Image2d* img_out, wf3d_lightsource const* lightsource_list, unsigned int nb_lightsources, owl_v3f32 cam_v_pos, owl_q32 cam_q_rot, wf3d_camera3d const* cam)
{
    if(octree == NULL)
    {
        return FC3D_SUCCESS;
    }
    else
    {
        fc3d_error error = FC3D_SUCCESS;

        owl_q32 cam_q_rot_conj = owl_q32_conj(cam_q_rot);
        owl_v3f32 opp_cam_v_pos = owl_v3f32_sub(owl_v3f32_zero(), cam_v_pos);

        wf3d_lightsource* cam_lightsource_list = malloc(nb_lightsources * sizeof(*cam_lightsource_list));
        if(nb_lightsources == 0 || cam_lightsource_list != NULL)
        {
            for(unsigned int k = 0 ; k < nb_lightsources ; k++)
            {
                wf3d_lightsource_transform(cam_lightsource_list + k, lightsource_list + k, opp_cam_v_pos, cam_q_rot_conj);
            }

            error = fc3d_rendering_octree_node_Rasterization(octree->node_0, img_out, cam_lightsource_list, nb_lightsources, cam_v_pos, cam_q_rot, cam);
        }
        else
        {
            error = FC3D_MEMORY_ERROR;
        }

        free(cam_lightsource_list);

        return error;
    }


}

