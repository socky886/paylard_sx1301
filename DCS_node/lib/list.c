/*
 * list.c
 *
 *  Created on: 2018年2月26日
 *      Author: ases-jack
 */
#include "list.h"
#include "print.h"

inline void list_initialize(struct list_node *list)
{
	list->prev = list->next = list;
}

inline void list_clear_node(struct list_node *item)
{
	item->prev = item->next = 0;
}

inline uint8 list_in_list(struct list_node *item)
{
	if (item->prev == 0 && item->next == 0)
		return false;
	else
		return true;
}

inline void list_add_head(struct list_node *list, struct list_node *item)
{
	if (list->next == NULL || list->prev == NULL || list->next->prev != list || list->prev->next != list) {
		print("linked list item %p corrupted", list);
	}
	item->next = list->next;
	item->prev = list;
	list->next->prev = item;
	list->next = item;
}

inline void list_add_tail(struct list_node *list, struct list_node *item)
{
	if (list->next == NULL || list->prev == NULL || list->next->prev != list || list->prev->next != list) {
		print("linked list item %p corrupted", list);
	}
	item->prev = list->prev;
	item->next = list;
	list->prev->next = item;
	list->prev = item;
}

inline void list_delete(struct list_node *item)
{
	if (item->next == NULL || item->prev == NULL || item->next->prev != item || item->prev->next != item) {
		print("linked list item %p corrupted", item);
	}
	item->next->prev = item->prev;
	item->prev->next = item->next;
	item->prev = item->next = 0;
}

inline struct list_node* list_remove_head(struct list_node *list)
{
	if(list->next != list) {
		struct list_node *item = list->next;
		list_delete(item);
		return item;
	} else {
		return NULL;
	}
}

inline struct list_node* list_remove_tail(struct list_node *list)
{
	if(list->prev != list) {
		struct list_node *item = list->prev;
		list_delete(item);
		return item;
	} else {
		return NULL;
	}
}

inline struct list_node* list_peek_head(struct list_node *list)
{
	if(list->next != list) {
		return list->next;
	} else {
		return NULL;
	}
}

inline struct list_node* list_peek_tail(struct list_node *list)
{
	if(list->prev != list) {
		return list->prev;
	} else {
		return NULL;
	}
}

inline struct list_node* list_next(struct list_node *list, struct list_node *item)
{
	if(item->next != list)
		return item->next;
	else
		return NULL;
}

inline struct list_node* list_next_wrap(struct list_node *list, struct list_node *item)
{
	if(item->next != list)
		return item->next;
	else if(item->next->next != list)
		return item->next->next;
	else
		return NULL;
}

inline uint8 list_is_empty(struct list_node *list)
{
	return (list->next == list) ? TRUE : FALSE;
}
