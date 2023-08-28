/**
 * \author Nicolas Gutierrez Suarez
 */

#ifndef _DPLIST_H_
#define _DPLIST_H_

typedef enum {
    false, true
} bool; // or use C99 #include <stdbool.h>

/**
 * dplist_t is a struct containing at least a head pointer to the start of the list;
 */
typedef struct dplist dplist_t;

/**
 * A node in the list.
 */
typedef struct dplist_node dplist_node_t;

/* General remark on error handling
 * All functions below will:
 * - use assert() to check if memory allocation was successfully.
 */

/** Create and allocate memory for a new list
 * \param element_free callback function to free memory allocated to element
 * \return a pointer to a newly-allocated and initialized list.
 */
dplist_t *dpl_create(void (*element_free)(void **element));

/** Deletes all elements in the list
 * - Every list node of the list needs to be deleted. (free memory)
 * - The list itself also needs to be deleted. (free all memory)
 * - '*list' must be set to NULL.
 * \param list a double pointer to the list
 * \param free_element if true call datum_free() on the element of the list node to remove
 */
void dpl_free(dplist_t **list);

/** Returns the number of elements in the list.
 * - If 'list' is is NULL, -1 is returned.
 * \param list a pointer to the list
 * \return the size of the list
 */
int dpl_size(dplist_t *list);

/** Inserts a new list node containing an 'element' in the list at position 'index'
 * - the first list node has index 0.
 * - If 'index' is 0 or negative, the list node is inserted at the start of 'list'.
 * - If 'index' is bigger than the number of elements in the list, the list node is inserted at the end of the list.
 * - If 'list' is is NULL, NULL is returned.
 * \param list a pointer to the list
 * \param element a pointer to the data that needs to be inserted
 * \param index the position at which the element should be inserted in the list
 * \param insert_copy if true use datum_copy() to make a copy of 'element' and use the copy in the new list node, otherwise the given element pointer is added to the list
 * \return a pointer to the list or NULL
 */
dplist_t *dpl_insert_at_index(dplist_t *list, void *element, int index);


/** Returns a reference to the list node with index 'index' in the list.
 * - If 'index' is 0 or negative, a reference to the first list node is returned.
 * - If 'index' is bigger than the number of list nodes in the list, a reference to the last_log list node is returned.
 * - If the list is empty, NULL is returned.
 * - If 'list' is is NULL, NULL is returned.
 * \param list a pointer to the list
 * \param index the position of the node for which the reference is returned
 * \return a pointer to the list node at the given index or NULL
 */
dplist_node_t *dpl_get_reference_at_index(dplist_t *list, int index);

/** Returns the list element contained in the list node with index 'index' in the list.
 * - return is not returning a copy of the element with index 'index', i.e. 'datum_copy()' is not used.
 * - If 'index' is 0 or negative, the element of the first list node is returned.
 * - If 'index' is bigger than the number of elements in the list, the element of the last_log list node is returned.
 * - If the list is empty, NULL is returned.
 * - If 'list' is NULL, NULL is returned.
 * \param list a pointer to the list
 * \param index the position of the node for which the element is returned
 * \return a pointer to the element at the given index or NULL
 */
void *dpl_get_element_at_index(dplist_t *list, int index);

#endif  // _DPLIST_H_

