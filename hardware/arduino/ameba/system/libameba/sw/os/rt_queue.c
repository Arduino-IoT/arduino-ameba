
#include "basic_types.h"
#include "rt_os_service.h"

#include "rt_queue.h"





void list_add_tail(struct list_head *cur, struct list_head *tail)
{
	__list_add(cur, tail->prev, tail);
}

void rtw_init_listhead(_list *list)
{
	INIT_LIST_HEAD(list);
}

u32 rtw_is_list_empty(_list *phead)
{
	if(list_empty(phead))
		return _TRUE;

	return _FALSE;
}

void rtw_list_delete(_list *plist)
{
	list_del_init(plist);
}


void	rtw_init_queue(_queue	*pqueue)
{
	rtw_init_listhead(&(pqueue->queue));
	rtw_spinlock_init(&(pqueue->lock));
}

u32	  rtw_queue_empty(_queue	*pqueue)
{
	return (rtw_is_list_empty(&(pqueue->queue)));
}

u32 rtw_end_of_queue_search(_list *head, _list *plist)
{
	if (head == plist)
		return _TRUE;
	else
		return _FALSE;
}


