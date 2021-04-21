#ifndef FC3D_LINKED_LIST_H_INCLUDED
#define FC3D_LINKED_LIST_H_INCLUDED

#include <stdlib.h>
#include <stdbool.h>

#include <FC3D/fc3d.h>

typedef struct fc3d_LinkedListElement fc3d_LinkedListElement;
struct fc3d_LinkedListElement
{
    fc3d_LinkedListElement* next_element;
    void* data;
};

//Creates a LinkedListElement
FC3D_DLL_EXPORT fc3d_LinkedListElement* fc3d_LinkedListElement_Create(size_t data_size);

//Destroy a LinkedListElement
FC3D_DLL_EXPORT void fc3d_LinkedListElement_Destroy(fc3d_LinkedListElement* link_elem);

//Destroy recursively a LinkedListElement
//Starts to destroy only after nb_kept elements
FC3D_DLL_EXPORT void fc3d_LinkedListElement_DestroyRecursively(fc3d_LinkedListElement* link_elem, size_t nb_kept);

//Get the data of the linked list
FC3D_DLL_EXPORT void* fc3d_LinkedListElement_GetData(fc3d_LinkedListElement* link_elem);

//Get the next element of the linked list
FC3D_DLL_EXPORT fc3d_LinkedListElement* fc3d_LinkedListElement_GetNext(fc3d_LinkedListElement* link_elem, bool create_if_needed, size_t data_size);



typedef struct
{
    size_t data_size;
    fc3d_LinkedListElement* first_element;

    fc3d_LinkedListElement* current_element;

} fc3d_LinkedList;

//Create a LinkedList with one element
FC3D_DLL_EXPORT fc3d_LinkedList* fc3d_LinkedList_Create(size_t data_size);

//Destroy a LinkedList
FC3D_DLL_EXPORT void fc3d_LinkedList_Destroy(fc3d_LinkedList* linked_list);

//Rewind the linked list and return a pointer to the first element's data
//Cannot fail
FC3D_DLL_EXPORT void* fc3d_LinkedList_Rewind(fc3d_LinkedList* linked_list);

//Get the current data
FC3D_DLL_EXPORT void* fc3d_LinkedList_GetCurrentElement(fc3d_LinkedList* linked_list);

//Next element
FC3D_DLL_EXPORT void* fc3d_LinkedList_NextElement(fc3d_LinkedList* linked_list, bool create_if_needed);

#endif // FC3D_LINKED_LIST_H_INCLUDED
