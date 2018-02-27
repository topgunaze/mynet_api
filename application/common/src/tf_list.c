/******************************************************************************
 * FILE   : tf_list.c
 * Description : 
 * Author : Gerhard Lao
 * Date   : 2015/12/30
 * Copyright (c) 2015-2025 by C-Data Tech. Co., Ltd. All Rights Reserved.
 ******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif 

#include "tf_list.h"

void tf_lst_init(tf_list * pLst , INT32 (*compare)(void * , UINT32))
{
    pLst->tf_head = NULL;
    pLst->tf_tail = NULL;
    pLst->count = 0;
    pLst->compare = compare;
}

/**********************************************************
Description:
   insert a new node 'pNode' after  the specific node  'pPrevious'
**********************************************************/
void tf_lst_insert(tf_list * pLst, tf_node * pPrevious, tf_node * pNode)
{
    tf_node * pNext = NULL;

    if (NULL == pPrevious) {
        pNext = pLst->tf_head;
        pLst->tf_head = pNode;
    } else {
        pNext = pPrevious->next;
        pPrevious->next = pNode;
    }

    if (NULL == pNext) {
        pLst->tf_tail = pNode;
    } else {
        pNext->previous = pNode;
    }

    pNode->previous = pPrevious;
    pNode->next = pNext;

    pLst->count++;
}

/**********************************************************
Description:
   add a new node to the tail of 'pLst'
**********************************************************/
void tf_lst_add(tf_list * pLst, tf_node * pNode)
{
    tf_lst_insert(pLst, pLst->tf_tail, pNode);
}

/**********************************************************
Description:
   Find the actual node according to the 'key' and remove it from 'pLst',
   The operatoin doesn't mean to free the node , it is only cut off from 'pLst'
**********************************************************/
tf_node *tf_lst_remove(tf_list * pLst , UINT32 key)
{
    tf_node *remove_node = (tf_node *)NULL;

    if (pLst->compare == NULL) {
        return (tf_node *)remove_node;
    }

    tf_lst_scan(pLst , remove_node , tf_node *) {
        if (pLst->compare((void *)remove_node , key) == 0) {
            tf_lst_delete(pLst , remove_node);
            return (tf_node *)remove_node;
        }
    }

    return (tf_node *)NULL;
}

/**********************************************************
Description:
   The operatoin doesn't mean to free the node , it is only cut off from 'pLst'
**********************************************************/
void tf_lst_delete(tf_list * pLst, tf_node * pNode)
{
    if (pNode->previous == NULL) {
        pLst->tf_head = pNode->next;
    } else {
        pNode->previous->next = pNode->next;
    }

    if (pNode->next == NULL) {
        pLst->tf_tail = pNode->previous;
    } else {
        pNode->next->previous = pNode->previous;
    }

    pLst->count--;
}

tf_node * tf_lst_first(tf_list * pLst)
{
    return pLst->tf_head;
}

tf_node * tf_lst_last(tf_list * pLst)
{
    return pLst->tf_tail;
}

/**********************************************************
Description:
   get the pointer of the first node in the list , and the node will be cutted off
   from the header of the list
**********************************************************/
tf_node * tf_lst_get(tf_list * pLst)
{
    tf_node * pNode = NULL;

    pNode = pLst->tf_head;

    if (pNode != NULL) {
        pLst->tf_head = pNode->next;

        if (pNode->next == NULL) {
            pLst->tf_tail = NULL;
        } else {
            pNode->next->previous = NULL;
        }

        pLst->count--;
    }

    return pNode;
}


tf_node * tf_lst_nth(tf_list * pLst, int nodeNum)
{
    tf_node * pNode = NULL;

    if (nodeNum < 1 || nodeNum > pLst->count) {
        return NULL;
    }

    if (nodeNum < ((pLst->count + 1) >> 1)) {
        pNode = pLst->tf_head;

        while (--nodeNum > 0) {
            pNode = pNode->next;
        }
    } else {
        nodeNum -= pLst->count;
        pNode = pLst->tf_tail;

        while (nodeNum++ < 0) {
            pNode = pNode->previous;
        }
    }

    return pNode;
}

/**********************************************************
Description:
   Get the pointer of the actual node according to the 'key'
**********************************************************/
tf_node *tf_lst_find(tf_list *pLst , UINT32 key)
{
    tf_node *find_node = NULL;

    if (pLst->compare == NULL) {
        return (tf_node *)find_node;
    }

    tf_lst_scan(pLst , find_node , tf_node *) {
        if (pLst->compare((void *)find_node , key) == 0)
            return (tf_node *)find_node;
    }

    return (tf_node *)NULL;
}

tf_node * tf_lst_prev(tf_node * pNode)
{
    return pNode->previous;
}

tf_node * tf_lst_next(tf_node * pNode)
{
    return pNode->next;
}

UINT32 tf_lst_count(tf_list * pLst)
{
    return pLst->count;
}

tf_list *tf_lst_concat(tf_list *pDst , tf_list *pSrc)
{
    if(pDst == NULL || pSrc == NULL){
        return NULL;
    }

    if(pDst->compare != pSrc->compare) {
        return NULL;
    }

    if(pDst->count == 0) {
        memcpy(pDst , pSrc , sizeof(tf_list));
        return (tf_list *)pDst;
    }

    if(pSrc->count == 0) {
        return (tf_list *)pDst;
    }

    pDst->tf_tail->next = pSrc->tf_head;
    pSrc->tf_head->previous = pDst->tf_tail;    
    pDst->tf_tail = pSrc->tf_tail;
    
    pDst->count += pSrc->count;

    tf_lst_init(pSrc , pSrc->compare);

    return (tf_list *)pDst;
}




#ifdef __cplusplus
}
#endif 


