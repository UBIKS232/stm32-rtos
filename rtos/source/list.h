#ifndef _LIST_H_
#define _LIST_H_

#include "portmacro.h"

// 链表节点, list item( = list node)
typedef struct xLIST_ITEM ListItem_t;
struct xLIST_ITEM
{
    // 辅助值, 节点排序
    TickType_t xItemValue;
    // 下一节点
    ListItem_t *pxNext;
    // 上一节点
    ListItem_t *pxPrevious;
    // 指向拥有该节点的内核对象, 即节点内嵌于的数据结构, 属于某个数据结构的一个成员, 数据结构通常为TCB(?)
    void *pvOwner;
    // 指向该节点所在链表, 通常指向根节点
    void *pvContainer;
};

// 环形链表最后一个节点/第一个节点, head/tail, 生产者
typedef struct xMINI_LIST_ITEM MiniListItem_t;
struct xMINI_LIST_ITEM
{
    // 辅助值, 节点排序
    TickType_t xItemValue;
    // 下一节点
    ListItem_t *pxNext;
    // 上一节点
    ListItem_t *pxPrevious;
};

// 链表根节点, 描述整个链表, list root
typedef struct xLIST List_t;
struct xLIST
{
    // 链表节点计数
    UBaseType_t uxNumberOfItems;
    // 链表节点索引
    ListItem_t *pxIndex;
    // 最后一个节点/第一个节点, head/tail, 生产者
    MiniListItem_t xListEnd;
};

void vListInitialiseItem(ListItem_t *const pxListItem);
void vListInitialise(List_t *const pxList);
void vListInsertEnd(List_t *const pxLis, ListItem_t *const pxNewListItem);
void vListInsert(List_t *const pxLis, ListItem_t *const pxNewListItem);

#endif // _LIST_H_
