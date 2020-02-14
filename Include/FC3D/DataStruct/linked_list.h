#ifndef FC3D_LINKED_LIST_H_INCLUDED
#define FC3D_LINKED_LIST_H_INCLUDED

#include <stdlib.h>
#include <stdbool.h>

typedef struct fc3d_LinkedListElement fc3d_LinkedListElement;
struct fc3d_LinkedListElement
{
    fc3d_LinkedListElement* next_element;
    void* data;
};

//Creates a LinkedListElement
fc3d_LinkedListElement* fc3d_LinkedListElement_Create(size_t data_size);

//Destroy a DataPoolBlock
void fc3d_LinkedListElement_Destroy(fc3d_LinkedListElement* link_elem);

//Destroy recursively an DataPoolBlock
//Starts to destroy only after nb_kept elements
void fc3d_LinkedListElement_DestroyRecursively(fc3d_LinkedListElement* link_elem, size_t nb_kept);

//Get the data of the linked list
void* fc3d_LinkedListElement_GetData(fc3d_LinkedListElement* link_elem);

//Get the next element of the linked list
fc3d_LinkedListElement* fc3d_LinkedListElement_GetNext(fc3d_LinkedListElement* link_elem, bool create, size_t data_size);



typedef struct
{
    size_t data_size;
    fc3d_LinkedListElement* first_element;

    fc3d_LinkedListElement* current_element;

} fc3d_LinkedList;

//Create a LinkedList with one element
fc3d_LinkedList* fc3d_LinkedList_Create(size_t data_size);

//Destroy a LinkedList
void fc3d_LinkedList_Destroy(fc3d_LinkedList* linked_list);

//Rewind the linked list and return a pointer to the first element's data
//Cannot fail
void* fc3d_LinkedList_Rewind(fc3d_LinkedList* linked_list);

//Get the current data
void* fc3d_LinkedList_GetCurrentElement(fc3d_LinkedList* linked_list);

//Next element
void* fc3d_LinkedList_NextElement(fc3d_LinkedList* linked_list, bool create);

#endif // FC3D_LINKED_LIST_H_INCLUDED
