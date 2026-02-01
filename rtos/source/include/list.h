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
    // 指向拥有该节点的内核对象, 即节点内嵌于的数据结构, 属于某个数据结构的一个成员, 数据结构通常为 TCB(??)
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

/**
 * 链表操作的宏
 */
// 初始化节点拥有者
#define listSET_LIST_ITEM_OWNER(pxListItem, pxOwner) \
    ((pxListItem)->pvOwner = (void *)(pxOwner))

// 获取节点拥有者
#define listGET_LIST_ITEM_OWNER(pxListItem) \
    ((pxListItem)->pvOwner)

// 初始化节点排序辅助值
#define listSET_LIST_ITEM_VALUE(pxListItem, xValue) \
    ((pxListItem)->xItemValue = (xValue))

// 获取节点排序辅助值
#define listGET_LIST_ITEM_VALUE(pxListItem) \
    ((pxListItem)->xItemValue)

// ??获取链表根节点的节点计数器的值
#define listGET_ITEM_VALUE_OF_HEAD_ENTRY(pxList) \
    (((pxList)->xListEnd).pxNext->xItemValue)

// 获取链表的入口节点
#define listGET_HEAD_ENTRY(pxList) \
    (((pxList)->xListEnd).pxNext)

// 获取节点的下一个节点
#define listGET_NEXT(pxListItem) \
    ((pxListItem)->pxNext)

// 获取链表的最后一个节点, 返回的指针的解引用对象不可修改
#define listGET_END_MARKER(pxList) \
    ((ListItem_t const *)&((pxList)->xListEnd))

// 判断链表是否为空
#define listLIST_IS_EMPTY(pxList) \
    ((BaseType_t)((pxList)->uxNumberOfItems == (UBaseType_t)0U))

// 获取链表的节点数
#define listCURRENT_LIST_LENGTH(pxList) \
    ((pxList)->uxNumberOfItems)

// 获取链表第一个节点的 owner
#define listGET_OWNER_OF_HEAD_ENTRY(pxList) \
    ((((pxList)->xListEnd).pxNext)->pvOwner)

#if 0
// 获取链表下一个节点的 owner
#define listGET_OWNER_OF_NEXT_ENTRY(pxTCB, pxList)               \
    do                                                           \
    {                                                            \
        List_t *const pxConstList = (pxList);                    \
        (pxConstList)->pxIndex = (pxConstList)->pxIndex->pxNext; \
    /* if 这一行写错了, xListEnd 是 List 内的成员, 不是 List 内的指针 */
        if ((void *)((pxConstList)->pxIndex) == (void *)((pxConstList)->xListEnd))      \
        {                                                                               \
            /* 防止 index 索引到 root 内的 end 节点(没有 owner ), 因此加入了一次判断 */ \
            (pxConstList)->pxIndex = (pxConstList)->pxIndex->pxNext;                    \
        }                                                                               \
        (pxTCB) = (pxConstList)->pxIndex->pvOwner;                                      \
    } while (0);
#else
/*
 * Access function to obtain the owner of the next entry in a list.
 *
 * The list member pxIndex is used to walk through a list.  Calling
 * listGET_OWNER_OF_NEXT_ENTRY increments pxIndex to the next item in the list
 * and returns that entry's pxOwner parameter.  Using multiple calls to this
 * function it is therefore possible to move through every item contained in
 * a list.
 *
 * The pxOwner parameter of a list item is a pointer to the object that owns
 * the list item.  In the scheduler this is normally a task control block.
 * The pxOwner parameter effectively creates a two way link between the list
 * item and its owner.
 *
 * @param pxTCB pxTCB is set to the address of the owner of the next list item.
 * @param pxList The list from which the next item owner is to be returned.
 *
 * \page listGET_OWNER_OF_NEXT_ENTRY listGET_OWNER_OF_NEXT_ENTRY
 * \ingroup LinkedList
 */
#define listGET_OWNER_OF_NEXT_ENTRY(pxTCB, pxList)                                \
    do                                                                            \
    {                                                                             \
        List_t *const pxConstList = (pxList);                                     \
        /* Increment the index to the next item and return the item, ensuring */  \
        /* we don't return the marker used at the end of the list.  */            \
        (pxConstList)->pxIndex = (pxConstList)->pxIndex->pxNext;                  \
        if ((void *)(pxConstList)->pxIndex == (void *)&((pxConstList)->xListEnd)) \
        {                                                                         \
            (pxConstList)->pxIndex = (pxConstList)->pxIndex->pxNext;              \
        }                                                                         \
        (pxTCB) = (pxConstList)->pxIndex->pvOwner;                                \
    } while (0);
#endif

#endif // _LIST_H_
