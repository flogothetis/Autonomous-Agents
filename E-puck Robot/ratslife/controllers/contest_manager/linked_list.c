/*
 * File:         linked_list.c
 * Date:         18th Aug 2010
 * Description:  Implementation of the linked_list.h file
 * Author:       fabien.rohrer@cyberbotics.com
 * Modifications:
 */

#include "linked_list.h"
#include <stdlib.h>

LLIST *list_add(LLIST **p, void *i) {
  if (p == NULL)
    return NULL;
 
  LLIST *n = malloc(sizeof(LLIST));
  if (n == NULL)
    return NULL;
 
  n->next = *p;
  *p = n;
  n->data = i;
 
  return *p;
}

LLIST *list_at(LLIST **p, int at) {
  if (p == NULL || at < 0)
    return NULL;
  else if (at == 0)
    return *p;
  else
    return list_at(&((*p)->next), at-1);
}

void list_remove(LLIST **p) {
  if (p != NULL && *p != NULL) {
    LLIST *n = *p;
    *p = (*p)->next;
    free(n);
  }
}

LLIST **list_search(LLIST **n, void *i) {
  if (n == NULL)
    return NULL;
 
  while (*n != NULL) {
    if ((*n)->data == i) {
      return n;
    }
    n = &(*n)->next;
  }
  return NULL;
}

int list_size(LLIST *n) {
  int size = 0;
  while (n != NULL) {
    size++;
    n = n->next;
  }
  return size;
}
