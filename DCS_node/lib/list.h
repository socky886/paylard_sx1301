/*
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#ifndef __LIST_H
#define __LIST_H

#include "types_def.h"

#define containerof(ptr, type, member) \
	((type *)((addr_t)(ptr) - offsetof(type, member)))

struct list_node {
	struct list_node *prev;
	struct list_node *next;
};

extern inline void list_initialize(struct list_node *list);
extern inline void list_clear_node(struct list_node *item);
extern inline uint8 list_in_list(struct list_node *item);
extern inline void list_add_head(struct list_node *list, struct list_node *item);
extern inline void list_add_tail(struct list_node *list, struct list_node *item);
extern inline void list_delete(struct list_node *item);
extern inline struct list_node* list_remove_head(struct list_node *list);
extern inline struct list_node* list_remove_tail(struct list_node *list);
extern inline struct list_node* list_peek_head(struct list_node *list);
extern inline struct list_node* list_peek_tail(struct list_node *list);
extern inline struct list_node* list_next(struct list_node *list, struct list_node *item);
extern inline struct list_node* list_next_wrap(struct list_node *list, struct list_node *item);
extern inline uint8 list_is_empty(struct list_node *list);

#define LIST_INITIAL_VALUE(list) (struct list_node){ &(list), &(list) }
#define list_add_after(entry, new_entry) list_add_head(entry, new_entry)
#define list_add_before(entry, new_entry) list_add_tail(entry, new_entry)
#define list_remove_head_type(list, type, element) ({\
    struct list_node *__nod = list_remove_head(list);\
    type *__t;\
    if(__nod)\
        __t = containerof(__nod, type, element);\
    else\
        __t = (type *)0;\
    __t;\
})

#define list_remove_tail_type(list, type, element) ({\
    struct list_node *__nod = list_remove_tail(list);\
    type *__t;\
    if(__nod)\
        __t = containerof(__nod, type, element);\
    else\
        __t = (type *)0;\
    __t;\
})

#define list_peek_head_type(list, type, element) ({\
    struct list_node *__nod = list_peek_head(list);\
    type *__t;\
    if(__nod)\
        __t = containerof(__nod, type, element);\
    else\
        __t = (type *)0;\
    __t;\
})

#define list_peek_tail_type(list, type, element) ({\
    struct list_node *__nod = list_peek_tail(list);\
    type *__t;\
    if(__nod)\
        __t = containerof(__nod, type, element);\
    else\
        __t = (type *)0;\
    __t;\
})

#define list_next_type(list, item, type, element) ({\
    struct list_node *__nod = list_next(list, item);\
    type *__t;\
    if(__nod)\
        __t = containerof(__nod, type, element);\
    else\
        __t = (type *)0;\
    __t;\
})

#define list_next_wrap_type(list, item, type, element) ({\
    struct list_node *__nod = list_next_wrap(list, item);\
    type *__t;\
    if(__nod)\
        __t = containerof(__nod, type, element);\
    else\
        __t = (type *)0;\
    __t;\
})

// iterates over the list, node should be struct list_node*
#define list_for_every(list, node) \
	for(node = (list)->next; node != (list); node = node->next)

// iterates over the list in a safe way for deletion of current node
// node and temp_node should be struct list_node*
#define list_for_every_safe(list, node, temp_node) \
	for(node = (list)->next, temp_node = (node)->next;\
	node != (list);\
	node = temp_node, temp_node = (node)->next)

// iterates over the list, entry should be the container structure type *
#define list_for_every_entry(list, entry, type, member) \
	for((entry) = containerof((list)->next, type, member);\
		&(entry)->member != (list);\
		(entry) = containerof((entry)->member.next, type, member))

// iterates over the list in a safe way for deletion of current node
// entry and temp_entry should be the container structure type *
#define list_for_every_entry_safe(list, entry, temp_entry, type, member) \
	for(entry = containerof((list)->next, type, member),\
		temp_entry = containerof((entry)->member.next, type, member);\
		&(entry)->member != (list);\
		entry = temp_entry, temp_entry = containerof((temp_entry)->member.next, type, member))

#endif
