/******************************************************************************
 * FILE   : tf_list.h
 * Description : 
 * Author : Gerhard Lao
 * Date   : 2015/12/30
 * Copyright (c) 2015-2025 by C-Data Tech. Co., Ltd. All Rights Reserved.
 ******************************************************************************/
#ifndef _TF_LIST_H_
#define _TF_LIST_H_

#ifdef __cplusplus
extern "C"
{
#endif 

#include <tf_types.h>

typedef struct tf_node_t
{
    struct tf_node_t * previous;
    struct tf_node_t * next;
}tf_node;

typedef struct tf_list_t
{
    tf_node node;
    UINT32 count;
    INT32 (*compare)(void * , UINT32);
}tf_list;

#define tf_tail node.previous
#define tf_head node.next

extern void tf_lst_init(tf_list * pLst , INT32 (*compare)(void * , UINT32));
extern void tf_lst_insert(tf_list * pLst, tf_node * pPrevious, tf_node * pNode);
extern void tf_lst_add(tf_list * pLst, tf_node * pNode);
tf_node *tf_lst_remove(tf_list * pLst , UINT32 key);
extern void tf_lst_delete(tf_list * pLst, tf_node * pNode);
extern tf_node * tf_lst_first(tf_list * pLst);
extern tf_node * tf_lst_last(tf_list * pLst);
extern tf_node * tf_lst_get(tf_list * pLst);
extern tf_node * tf_lst_nth(tf_list * pLst, int nodeNum);
tf_node *tf_lst_find(tf_list *pLst , UINT32 key);
extern tf_node * tf_lst_prev(tf_node * pNode);
extern tf_node * tf_lst_next(tf_node * pNode);
extern UINT32 tf_lst_count(tf_list * pLst);
extern tf_list *tf_lst_concat(tf_list *pDst , tf_list *pSrc);

#define tf_lst_scan(pList , pNode , type) for(pNode=(type)tf_lst_first(pList);pNode;pNode=(type)tf_lst_next((tf_node *)pNode))



#ifdef __cplusplus
}
#endif 


#endif /*_TF_LIST_H_*/


