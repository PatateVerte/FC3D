#include <FC3D/Rendering/Octree/octree.h>

#include <pthread.h>

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
wf3d_error fc3d_RenderingOctree_Rasterization(fc3d_RenderingOctree* octree, wf3d_Image2d* img_out, wf3d_lightsource const* lightsource_list, unsigned int nb_lightsources, owl_v3f32 cam_v_pos, owl_q32 cam_q_rot, wf3d_camera3d const* cam)
{
    wf3d_error error = WF3D_SUCCESS;

    wf3d_image2d_rectangle img_rect;
    img_rect.img2d = img_out;
    img_rect.x_min = 0;
    img_rect.x_max = img_out->width;
    img_rect.y_min = 0;
    img_rect.y_max = img_out->height;

    owl_q32 cam_q_rot_conj = owl_q32_conj(cam_q_rot);
    owl_v3f32 opp_cam_v_pos = owl_v3f32_sub(owl_v3f32_zero(), cam_v_pos);

    wf3d_lightsource* cam_lightsource_list = malloc(nb_lightsources * sizeof(*cam_lightsource_list));
    if(nb_lightsources == 0 || cam_lightsource_list != NULL)
    {
        for(unsigned int k = 0 ; k < nb_lightsources ; k++)
        {
            wf3d_lightsource_transform(cam_lightsource_list + k, lightsource_list + k, opp_cam_v_pos, cam_q_rot_conj);
        }

        error = fc3d_rendering_octree_node_Rasterization(octree->node_0, &img_rect, cam_lightsource_list, nb_lightsources, cam_v_pos, cam_q_rot, cam);
    }
    else
    {
        error = WF3D_MEMORY_ERROR;
    }

    free(cam_lightsource_list);

    return error;
}

//Multithread rasterization data structures
typedef struct
{
    fc3d_RenderingOctree const* octree;

    wf3d_lightsource const* cam_lightsource_list;
    unsigned int nb_lightsources;
    owl_v3f32 cam_v_pos;
    owl_q32 cam_q_rot;
    wf3d_camera3d const* cam;

    wf3d_Image2d* img_out;

    unsigned int x_block_len;
    unsigned int y_block_len;

    pthread_mutex_t common_mutex;
    unsigned int next_x;
    unsigned int next_y;

} wf3d_octree_rasterization_thread_common_arg;

typedef struct
{
    wf3d_octree_rasterization_thread_common_arg* common_arg;
    wf3d_error error_ret;

} wf3d_octree_rasterization_thread_arg;

//Multithread function
static void* wf3d_octree_multithread_rasterization(void* ptr)
{
    wf3d_error error = WF3D_SUCCESS;

    wf3d_octree_rasterization_thread_arg* arg = ptr;
    wf3d_octree_rasterization_thread_common_arg* common_arg = arg->common_arg;

    wf3d_image2d_rectangle img_rect;
    img_rect.img2d = common_arg->img_out;

    bool rasterization_over = false;

    while(error == WF3D_SUCCESS && !rasterization_over)
    {
        int thread_error = pthread_mutex_lock(&common_arg->common_mutex);

        if(thread_error == 0)
        {
            if(common_arg->next_x < common_arg->img_out->width && common_arg->next_y < common_arg->img_out->height)
            {
                img_rect.y_min = common_arg->next_y;
                img_rect.x_min = common_arg->next_x;

                img_rect.y_max = common_arg->next_y + common_arg->y_block_len;
                if(img_rect.y_max > common_arg->img_out->height)
                {
                    img_rect.y_max = common_arg->img_out->height;
                }

                img_rect.x_max = img_rect.x_min + common_arg->x_block_len;

                if(img_rect.x_max > common_arg->img_out->width)
                {
                    img_rect.x_max = common_arg->img_out->width;
                    common_arg->next_x = 0;
                    common_arg->next_y += common_arg->y_block_len;
                }
                else
                {
                    common_arg->next_x += common_arg->x_block_len;
                }
            }
            else
            {
                rasterization_over = true;
            }
        }
        else
        {
            error = WF3D_THREAD_ERROR;
        }

        pthread_mutex_unlock(&common_arg->common_mutex);

        if(error == WF3D_SUCCESS && !rasterization_over)
        {
            error = fc3d_rendering_octree_node_Rasterization(common_arg->octree->node_0, &img_rect, common_arg->cam_lightsource_list, common_arg->nb_lightsources, common_arg->cam_v_pos, common_arg->cam_q_rot, common_arg->cam);
        }
    }

    arg->error_ret = error;

    return NULL;
}

//Rasterization with multithreading
//
//
wf3d_error fc3d_RenderingOctree_MultiThreadRasterization(fc3d_RenderingOctree* octree, wf3d_Image2d* img_out, unsigned short nb_threads, wf3d_lightsource const* lightsource_list, unsigned int nb_lightsources, owl_v3f32 cam_v_pos, owl_q32 cam_q_rot, wf3d_camera3d const* cam)
{
    wf3d_error error = WF3D_SUCCESS;

    owl_q32 cam_q_rot_conj = owl_q32_conj(cam_q_rot);
    owl_v3f32 opp_cam_v_pos = owl_v3f32_sub(owl_v3f32_zero(), cam_v_pos);

    if(nb_threads > 0)
    {
        wf3d_lightsource* cam_lightsource_list = malloc(nb_lightsources * sizeof(*cam_lightsource_list));

        pthread_t* thread_list = malloc((size_t)nb_threads * sizeof(*thread_list));

        wf3d_octree_rasterization_thread_common_arg common_arg;
        wf3d_octree_rasterization_thread_arg* thread_arg_list = malloc((size_t)nb_threads * sizeof(*thread_arg_list));

        if((nb_lightsources == 0 || cam_lightsource_list != NULL) && thread_list != NULL && thread_arg_list != NULL)
        {
            for(unsigned int k = 0 ; k < nb_lightsources ; k++)
            {
                wf3d_lightsource_transform(cam_lightsource_list + k, lightsource_list + k, opp_cam_v_pos, cam_q_rot_conj);
            }

            common_arg.octree = octree;
            common_arg.cam_lightsource_list = cam_lightsource_list;
            common_arg.nb_lightsources = nb_lightsources;
            common_arg.cam_v_pos = cam_v_pos;
            common_arg.cam_q_rot = cam_q_rot;
            common_arg.cam = cam;
            common_arg.img_out = img_out;

            unsigned int nb_div = 2;
            while(nb_div * nb_div < 4 * (unsigned int)nb_threads)
            {
                nb_div *= 2;
            }

            common_arg.x_block_len = img_out->width / nb_div;
            common_arg.y_block_len = img_out->height / nb_div;

            common_arg.common_mutex = PTHREAD_MUTEX_INITIALIZER;
            common_arg.next_x = 0;
            common_arg.next_y = 0;

            unsigned short nb_thread_launched = 0;
            for(unsigned short k = 0 ; k < nb_threads && error == WF3D_SUCCESS ; k++)
            {
                thread_arg_list[k].common_arg = &common_arg;
                thread_arg_list[k].error_ret = WF3D_SUCCESS;
                int thread_error = pthread_create(thread_list + k, NULL, wf3d_octree_multithread_rasterization, thread_arg_list + k);

                if(thread_error == 0)
                {
                    nb_thread_launched++;
                }
                else
                {
                    error = WF3D_THREAD_ERROR;
                }
            }

            for(unsigned int k = 0 ; k < nb_thread_launched ; k++)
            {
                int thread_error = pthread_join(thread_list[k], NULL);
                if(thread_error != 0)
                {
                    error = WF3D_THREAD_ERROR;
                }
            }

            if(error == WF3D_SUCCESS)
            {
                for(unsigned short k = 0 ; k < nb_threads && error == WF3D_SUCCESS ; k++)
                {
                    error = thread_arg_list[k].error_ret;
                }
            }
        }
        else
        {
            error = WF3D_MEMORY_ERROR;
        }

        free(cam_lightsource_list);
        free(thread_list);
        free(thread_arg_list);
    }
    else
    {
        error = WF3D_THREAD_ERROR;
    }

    return error;
}

