#include "list.h"

/**
 * @brief 链表节点初始化
 * @param ListItem_t *const pxListItem: 链表节点描述结构体
 */
void vListInitialiseItem(ListItem_t *const pxListItem)
{
    // 该节点所在链表为空, 即未插入任何链表
    pxListItem->pvContainer = NULL;
}

/**
 * @brief 链表根节点初始化
 * @param List_t *const pxList: 链表根节点描述结构体
 */
void vListInitialise(List_t *const pxList)
{
    // 初始化链表节点计数器的值为 0，表示链表为空
    pxList->uxNumberOfItems = (UBaseType_t)0U;
    // 将链表索引指针指向最后一个节点, 私有化 xListEnd, 将 pxIndex 作为节点的外部访问方式
    pxList->pxIndex = (ListItem_t *)&(pxList->xListEnd);
    // 将链表最后一个节点的辅助排序的值设置为最大，确保该节点就是链表的最后节点
    pxList->xListEnd.xItemValue = portMAX_DELAY;
    // 将最后一个节点的 pxNext 和 pxPrevious 指针均指向节点自身，表示链表为空
    pxList->xListEnd.pxNext = (ListItem_t *)&(pxList->xListEnd);
    pxList->xListEnd.pxPrevious = (ListItem_t *)&(pxList->xListEnd);
}

/**
 * @brief 将新节点插入链表头部(或尾部, 头部更好理解一点), 就是将一个新的节点插入到一个空的链
表, 仅用于空链表??
 * @param List_t *const pxList: 链表根节点描述结构体
 * @param ListItem_t *const pxListItem: 链表节点描述结构体
 */
void vListInsertEnd(List_t *const pxList, ListItem_t *const pxNewListItem)
{
    // index 索引的 prev 是当前环形链表的"尾部", 而 index 自身(end)为"头部"
    // ??pxIndex 总是指向 end 吗, negative:
    // 1. listGET_OWNER_OF_NEXT_ENTRY 改变 index 指向
    ListItem_t *const pxIndex = pxList->pxIndex;

    // 将新链表节点的 prev 指向当前的(未加入新节点时的链表的)"尾部"
    pxNewListItem->pxPrevious = pxIndex->pxPrevious;
    // 将新链表节点的next指向"头部"(index)
    pxNewListItem->pxNext = pxIndex;
    // index 索引的 prev(原链表"尾部") 的 next 更新为新节点地址
    pxIndex->pxPrevious->pxNext = pxNewListItem;
    // index 索引的 prev(原链表"尾部") 更新为新节点, 新节点作为新的"尾部"
    pxIndex->pxPrevious = pxNewListItem;

    pxNewListItem->pvContainer = (void *)pxList;

    (pxList->uxNumberOfItems)++;
}

/**
 * @brief 将节点按照升序排列插入链表
 * @param List_t *const pxList: 链表根节点描述结构体
 * @param ListItem_t *const pxListItem: 链表节点描述结构体
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

    // 更新链表插入位置的 prev, next, 此时的 pxIterator 指向要插入的位置的前一个节点
    // pxNewListItem 前后节点更新
    pxNewListItem->pxPrevious = pxIterator;
    pxNewListItem->pxNext = pxIterator->pxNext;
    // pxNewListItem 后一个节点的前一个结点从 pxIterator 更新为 pxNewListItem
    pxNewListItem->pxNext->pxPrevious = pxNewListItem;
    // pxNewListItem 前一个结点(pxIterator)的后一个节点从 pxNewListItem->pxNext(aka 原pxIterator->pxNext) 更新为 pxNewListItem
    pxIterator->pxNext = pxNewListItem;

    pxNewListItem->pvContainer = pxList;

    (pxList->uxNumberOfItems)++;
}

/**
 * @brief 将某个节点删除
 * @param ListItem_t *const pxItemToRemove: 链表节点描述结构体
 * @returns UBaseType_t pxList->uxNumberOfItems: 链表内节点数量
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
