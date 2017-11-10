/*
 * File:         linked_list.h
 * Date:         18th Aug 2010
 * Description:  Interface of a linked list of void*
 * Author:       fabien.rohrer@cyberbotics.com
 * Modifications:
 * Example:
 *     LLIST *n = NULL;
 *     int zero=0, un=1, deux=2, trois=3, quatre=4;
 *     printf("A:%d\n", list_size(n));
 *     list_add(&n, &zero);                 // list: 0
 *     printf("B:%d\n", list_size(n));
 *     list_add(&n, &un);                   // list: 1 0
 *     list_add(&n, &deux);                 // list: 2 1 0
 *     list_add(&n, &trois);                // list: 3 2 1 0
 *     list_add(&n, &quatre);               // list: 4 3 2 1 0
 *     printf("C:%d\n", list_size(n));
 *     list_remove(&n);                     // remove first (4)
 *     list_remove(&n->next);               // remove new second (2)
 *     list_remove(list_search(&n, &un));   // remove cell containing 1 (first)
 *     list_remove(&n->next);               // remove second to last node (0)
 *     list_remove(&n);                     // remove last (3)
 *     printf("D:%d\n", list_size(n));
 */

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

typedef struct node {
  void *data;
  struct node *next; /* pointer to next element in list */
} LLIST;
 
LLIST *list_add(LLIST **p, void *i);     /* Function definition to add an element */
void list_remove(LLIST **p);             /* Function definition to remove element (head) */
LLIST **list_search(LLIST **n, void *i); /* Function definition to search the list - Look out: O(N)*/
int list_size(LLIST *n);                 /* Function definition to count the list elements - Look out: O(N)*/
LLIST *list_at(LLIST **p, int at);       /* Function definition to return the i-th element of the list - Look out: O(N)*/

#endif
