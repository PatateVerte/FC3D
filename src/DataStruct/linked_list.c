#include <FC3D/DataStruct/linked_list.h>

/*
    LINKED LIST ELEMENT
*/

//Create a LinkedListElement
//
//
fc3d_LinkedListElement* fc3d_LinkedListElement_Create(size_t data_size)
{
    fc3d_LinkedListElement* link_elem = malloc(sizeof(*link_elem));

    link_elem->next_element = NULL;

    if(link_elem != NULL)
    {
        link_elem->data = malloc(data_size);

        //If an error occured
        if(data_size != 0 && link_elem->data == NULL)
        {
            fc3d_LinkedListElement_Destroy(link_elem);
            link_elem = NULL;
        }
    }

    return link_elem;
}

//Destroy a LinkedListElement
//
//
void fc3d_LinkedListElement_Destroy(fc3d_LinkedListElement* link_elem)
{
    if(link_elem != NULL)
    {
        free(link_elem->data);
        free(link_elem);
    }
}

//Destroy recursively an LinkedListElement
//
//
void fc3d_LinkedListElement_DestroyRecursively(fc3d_LinkedListElement* link_elem, size_t nb_kept)
{
    if(link_elem != NULL)
    {
        fc3d_LinkedListElement* next_link_elem = link_elem->next_element;

        if(nb_kept == 0)
        {
            fc3d_LinkedListElement_DestroyRecursively(next_link_elem, 0);
            fc3d_LinkedListElement_Destroy(link_elem);
        }
        else
        {
            fc3d_LinkedListElement_DestroyRecursively(next_link_elem, nb_kept - 1);
        }
    }
}

//Get the data of the linked list
//
//
void* fc3d_LinkedListElement_GetData(fc3d_LinkedListElement* link_elem)
{
    return link_elem->data;
}

//Get the next element of the linked list
//
//
fc3d_LinkedListElement* fc3d_LinkedListElement_GetNext(fc3d_LinkedListElement* link_elem, bool create_if_needed, size_t data_size)
{
    fc3d_LinkedListElement* next_link_elem = link_elem->next_element;

    if(next_link_elem == NULL && create_if_needed)
    {
        next_link_elem = fc3d_LinkedListElement_Create(data_size);
        link_elem->next_element = next_link_elem;
    }

    return next_link_elem;
}

/*
    LINKED LIST
*/

//Create a LinkedList with one element
//
//
fc3d_LinkedList* fc3d_LinkedList_Create(size_t data_size)
{
    fc3d_LinkedList* linked_list = malloc(sizeof(*linked_list));

    if(linked_list != NULL)
    {
        linked_list->data_size = data_size;
        linked_list->first_element = fc3d_LinkedListElement_Create(data_size);
        linked_list->current_element = linked_list->first_element;

        if(linked_list->first_element == NULL)
        {
            fc3d_LinkedList_Destroy(linked_list);
            linked_list = NULL;
        }
    }

    return linked_list;
}

//Destroy a LinkedList
//
//
void fc3d_LinkedList_Destroy(fc3d_LinkedList* linked_list)
{
    if(linked_list != NULL)
    {
        fc3d_LinkedListElement_DestroyRecursively(linked_list->first_element, 0);
        free(linked_list);
    }
}

//Rewind the linked list and return a pointer to the first element's data
//
//Cannot fail
void* fc3d_LinkedList_Rewind(fc3d_LinkedList* linked_list)
{
    linked_list->current_element = linked_list->first_element;
    return fc3d_LinkedListElement_GetData(linked_list->current_element);
}

//Get the current data
//
//
void* fc3d_LinkedList_GetCurrentElement(fc3d_LinkedList* linked_list)
{
    return fc3d_LinkedListElement_GetData(linked_list->current_element);
}

//Next element
//
//
void* fc3d_LinkedList_NextElement(fc3d_LinkedList* linked_list, bool create_if_needed)
{
    fc3d_LinkedListElement* next_element = fc3d_LinkedListElement_GetNext(linked_list->current_element, create_if_needed, linked_list->data_size);
    if(next_element != NULL)
    {
        linked_list->current_element = next_element;
    }

    return fc3d_LinkedListElement_GetData(linked_list->current_element);
}
