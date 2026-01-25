#include "list.h"

struct xLIST List_Test = {0};
struct xLIST_ITEM List_Item_1 = {0};
struct xLIST_ITEM List_Item_2 = {0};
struct xLIST_ITEM List_Item_3 = {0};

int main(void)
{
    vListInitialise(&List_Test);

    vListInitialiseItem(&List_Item_1);
    List_Item_1.xItemValue = 1;

    vListInitialiseItem(&List_Item_2);
    List_Item_2.xItemValue = 2;

    vListInitialiseItem(&List_Item_3);
    List_Item_3.xItemValue = 3;

    vListInsert(&List_Test, &List_Item_2);
    vListInsert(&List_Test, &List_Item_1);
    vListInsert(&List_Test, &List_Item_3);

    while (1)
    {
    }
}
