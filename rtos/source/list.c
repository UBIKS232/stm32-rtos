#include "list.h"

/**
 * @brief 链表节点初始化
 */
void vListInitialiseItem(ListItem_t *const pxListItem)
{
    // 该节点所在链表为空, 即未插入任何链表
    pxListItem->pvContainer = NULL;
}

/**
 * @brief 链表根节点初始化
 */
void vListInitialise(List_t *const pxList)
{
    // 初始化链表节点计数器的值为0，表示链表为空
    pxList->uxNumberOfItems = (UBaseType_t)0U;
    // 将链表索引指针指向最后一个节点, 私有化xListEnd, 将pxIndex作为节点的外部访问方式
    pxList->pxIndex = (ListItem_t *)&(pxList->xListEnd);
    // 将链表最后一个节点的辅助排序的值设置为最大，确保该节点就是链表的最后节点
    pxList->xListEnd.xItemValue = portMAX_DELAY;
    // 将最后一个节点的pxNext和pxPrevious指针均指向节点自身，表示链表为空
    pxList->xListEnd.pxNext = (ListItem_t *)&(pxList->xListEnd);
    pxList->xListEnd.pxPrevious = (ListItem_t *)&(pxList->xListEnd);
}

/**
 * @brief 将新节点插入链表头部(或尾部, 头部更好理解一点), 就是将一个新的节点插入到一个空的链
表, 仅用于空链表??
 */
void vListInsertEnd(List_t *const pxList, ListItem_t *const pxNewListItem)
{
    // index索引的prev是当前环形链表的"尾部", 而index自身(end)为"头部"
    // ??pxIndex总是指向end吗, negative:
    // 1. listGET_OWNER_OF_NEXT_ENTRY改变index指向
    ListItem_t *const pxIndex = pxList->pxIndex;

    // 将新链表节点的prev指向当前的(未加入新节点时的链表的)"尾部"
    pxNewListItem->pxPrevious = pxIndex->pxPrevious;
    // 将新链表节点的next指向"头部"(index)
    pxNewListItem->pxNext = pxIndex;
    // index索引的prev(原链表"尾部")的next更新为新节点地址
    pxIndex->pxPrevious->pxNext = pxNewListItem;
    // index索引的prev(原链表"尾部")更新为新节点, 新节点作为新的"尾部"
    pxIndex->pxPrevious = pxNewListItem;

    pxNewListItem->pvContainer = (void *)pxList;

    (pxList->uxNumberOfItems)++;
}

/**
 * @brief 将节点按照升序排列插入链表
 */
void vListInsert(List_t *const pxList, ListItem_t *const pxNewListItem)
{
    ListItem_t *pxIterator = NULL;

    const TickType_t xValueOfInsertion = pxNewListItem->xItemValue;

    // 查找插入位置
    if (xValueOfInsertion == portMAX_DELAY)
    {
        // 指向链表尾部节点
        pxIterator = pxList->xListEnd.pxPrevious;
    }
    else
    {
        // 按链表顺序从头到尾遍历查找
        for (pxIterator = (ListItem_t *)&(pxList->xListEnd);
             pxIterator->pxNext->xItemValue <= xValueOfInsertion;
             pxIterator = pxIterator->pxNext)
            ;
    }

    // 更新链表插入位置的prev, next, 此时的pxIterator指向要插入的位置的前一个节点
    // pxNewListItem前后节点更新
    pxNewListItem->pxPrevious = pxIterator;
    pxNewListItem->pxNext = pxIterator->pxNext;
    // pxNewListItem后一个节点的前一个结点从pxIterator更新为pxNewListItem
    pxNewListItem->pxNext->pxPrevious = pxNewListItem;
    // pxNewListItem前一个结点(pxIterator)的后一个节点从pxNewListItem->pxNext(aka 原pxIterator->pxNext)更新为pxNewListItem
    pxIterator->pxNext = pxNewListItem;

    pxNewListItem->pvContainer = pxList;

    (pxList->uxNumberOfItems)++;
}

/**
 * @brief 将某个节点删除
 */
UBaseType_t uxListRemove(ListItem_t *const pxItemToRemove)
{
    List_t *const pxList = (List_t *)(pxItemToRemove->pvContainer);

    if (pxList->pxIndex == pxItemToRemove)
    {
        pxList->pxIndex = pxItemToRemove->pxPrevious;
    }

    pxItemToRemove->pvContainer = NULL;
    pxItemToRemove->pxNext->pxPrevious = pxItemToRemove->pxPrevious;
    pxItemToRemove->pxPrevious->pxNext = pxItemToRemove->pxNext;

    (pxList->uxNumberOfItems)--;

    return pxList->uxNumberOfItems;
}

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

// 获取链表第一个节点的owner, 即TCB??
#define listGET_OWNER_OF_NEXT_ENTRY(pxTCB, pxList)                                 \
    do                                                                             \
    {                                                                              \
        List_t *const pxConstList = (pxList);                                      \
        (pxConstList)->pxIndex = (pxConstList)->pxIndex->pxNext;                   \
        if ((void *)((pxConstList)->pxIndex) == (void *)((pxConstList)->xListEnd)) \
        {                                                                          \
            /* 防止index索引到root内的end节点(没有owner), 因此加入了一次判断?? */    \
            (pxConstList)->pxIndex = (pxConstList)->pxIndex->pxNext;              \
        }                                                                          \
        (pxTCB) = (pxConstList)->pxIndex->pvOwner;                                 \
    } while (0);
