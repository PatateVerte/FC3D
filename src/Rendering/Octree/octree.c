#include <FC3D/Rendering/Octree/octree.h>

//Create an RenderingOctree
//
//
fc3d_RenderingOctree* fc3d_RenderingOctree_Create(wf3d_vect3d center, float half_size, size_t children_data_pool_block_len, size_t auxiliary_data_pool_block_len)
{
    fc3d_RenderingOctree* octree = malloc(sizeof(*octree));

    if(octree != NULL)
    {
        octree->center = center;
        octree->half_size = half_size;

        octree->octree_children_data_pool = fc3d_DataPool_Create(children_data_pool_block_len, 8 * sizeof(fc3d_rendering_octree_node));
        octree->octree_children_data_pool = fc3d_DataPool_Create(auxiliary_data_pool_block_len, sizeof(fc3d_rendering_octree_node));

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
fc3d_RenderingOctree* fc3d_RenderingOctree_Rewind(fc3d_RenderingOctree* octree)
{
    octree->node_0 = fc3d_DataPool_Rewind(octree->octree_children_data_pool);
    fc3d_DataPool_Rewind(octree->octree_auxiliary_data_pool);

    return octree;
}

