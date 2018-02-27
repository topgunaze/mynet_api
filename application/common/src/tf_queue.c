/**************************************************************
 * 文件名称:  tf_queue.c
 * 作           者:  keith.gong
 * 日           期:  2014.12.27
 * 文件描述:  系统消息队列管理
 * 版           本:  V1.00
 * 修改历史:  
 *     <修改者>   <时间>    <版本 >   <描述>
**************************************************************/


#include <unistd.h>
#include <sys/msg.h>
#include <errno.h>

#include "tf_types.h"
#include "tf_queue.h"

#include <stdio.h>
#define QUEUE_PRINTF(fmt, arg...)  printf(fmt,##arg)

/* 记录每个消息占用情况，0:未被使用1:已经被使用 */
static UINT8 queue_used_flag[SYS_QUEUE_MAX_NUM];


/**************************************************************
 * 函数名称: tf_queue_create
 * 作           者: keith.gong
 * 日           期: 2014年12月27日
 *
 * 函数功能: 创建有名消息队列
 * 输入参数: 
                            p_queue_id
                            sub_key  : 0 ~ (SYS_QUEUE_MAX_NUM-1)
                            queue_size
 * 输出参数: 
 * 返回数值: 
                            OK : 
                            ERROR : 
 * 其           它:  
**************************************************************/
RTN_STATUS 
tf_queue_create (
                INT32 *p_queue_id, 
                SYS_QUEUE_SUB_KEY_E sub_key, 
                UINT32 queue_size)
{
	int tmp_skt;
	key_t q_key;
	struct msqid_ds msg_ds;

	if(p_queue_id == NULL) 
	{
		QUEUE_PRINTF("Create queue failed cause some parameter is NULL.\r\n");
		return RTN_ERROR;
	}

	if(queue_size == 0) 
	{
		QUEUE_PRINTF("Create queue with queue size %d.\r\n", queue_size);
		return RTN_ERROR;
	}

	if(sub_key >= SYS_QUEUE_MAX_NUM || queue_used_flag[sub_key] != 0)
	{
		QUEUE_PRINTF("Create queue with sub key %d failed cause wrong key or recreating. \r\n", sub_key);
		return RTN_ERROR;
	}
    
	queue_used_flag[sub_key] = 1;

	/* Set the possible task Id to not free so that  no other task can try to use it */			  
	q_key = ftok("/dev/", sub_key); 	
	if((tmp_skt = msgget(q_key, IPC_CREAT)) < 0)
	{
		QUEUE_PRINTF("Create msgget failed.\r\n");
		return RTN_ERROR;
	}

	if(msgctl(tmp_skt, IPC_RMID, NULL) < 0)
	{
		QUEUE_PRINTF("Delete queue failed.\r\n");
		return RTN_ERROR;
	}

	if((tmp_skt = msgget(q_key, IPC_CREAT)) < 0)
	{
		QUEUE_PRINTF("Create msgget failed.\r\n");
		return RTN_ERROR;
	}
	
	/*Set msg queue max bytes*/
	msgctl(tmp_skt, IPC_STAT, &msg_ds);
	msg_ds.msg_qbytes = queue_size;
	
	if(msgctl(tmp_skt, IPC_SET, &msg_ds) < 0)
	{
		QUEUE_PRINTF("Set msgq size failed.\r\n");
		return RTN_ERROR;
	}

	*p_queue_id = tmp_skt;

	return RTN_OK;
}


/**************************************************************
 * 函数名称: tf_queue_get
 * 作           者: keith.gong
 * 日           期: 2014年12月27日
 *
 * 函数功能: 从消息队列中读取消息
 * 输入参数: 
                            queue_id : 
                            type:        消息类型
                            p_data:     buffer 的指针
                            size:         buffer 的大小
                            timeout:    SYS_QUEUE_WAIT_FOREVER / SYS_QUEUE_NO_WAIT / 毫秒值
 * 输出参数: 
                            p_data
                            p_size_copied:  实际读出消息的大小
 * 返回数值: 
                            OK : 
                            ERROR : 
 * 其           它:  
**************************************************************/
RTN_STATUS
tf_queue_get (
                INT32  queue_id, 
                long   type, 
                void   *p_data, 
                UINT32 size, 
                INT32  timeout, /* ms  */
                UINT32 *p_size_copied)
{
	int size_copied;

	/* Check Parameters */
	if( (p_data == NULL) || (p_size_copied == NULL) ) 
	{
		QUEUE_PRINTF("Get msg failed cause some parameter is NULL.\r\n");
		return RTN_ERROR;
	}	

 	/* Read the socket for data */
	if (timeout == SYS_QUEUE_WAIT_FOREVER)
	{
		size_copied = msgrcv(queue_id, p_data, size, type, 0);		
		//if (size_copied == -1)
		if (size_copied <= 0)
		{
			*p_size_copied = 0;
			return RTN_ERROR;
		}
		else if(size_copied <= size )
		{
			*p_size_copied = size_copied;
			return RTN_OK;
		}
        else            
            return RTN_ERROR;
	}
	else if (timeout == SYS_QUEUE_NO_WAIT)
	{
		size_copied = msgrcv(queue_id, p_data, size, type, IPC_NOWAIT);	  
		//if (size_copied == -1 && errno == ENOMSG )
		if (size_copied <= 0)
		{
			*p_size_copied = 0;
			return RTN_ERROR;
		}
		else if(size_copied <= size )
		{
			*p_size_copied = size_copied;
			return RTN_OK;
		}
        else            
            return RTN_ERROR;
	}
	else /* timeout */
	{
		int timeloop;
		/*
		** This "timeout" will check the socket for data every 100 milliseconds
		** up until the timeout value. Although this works fine for a desktop environment,
		** it should be written more efficiently for a flight system.
		** The proper way will be to use select or poll with a timeout
		*/
		for ( timeloop = timeout; timeloop > 0; timeloop = timeloop - 100 )
		{
			size_copied = msgrcv(queue_id, p_data, size, type, IPC_NOWAIT);

			//if (size_copied == -1)
            if (size_copied <= 0)
			{
				/* Sleep for 100 milliseconds */
				usleep(100 * 1000);
			}
			//else if ( size_copied == size )
			else if ( size_copied <= size)
			{
				*p_size_copied = size_copied;
				return RTN_OK;
			}
            else
                return RTN_ERROR;
		}

		return RTN_ERROR;

	} /* END timeout */

	/* Should never really get here. */
	return RTN_OK;
}



/**************************************************************
 * 函数名称: tf_queue_put
 * 作           者: keith.gong
 * 日           期: 2014年12月27日
 *
 * 函数功能: 写消息到消息队列中
 * 输入参数: 
                            queue_id : 
                            p_data:     buffer 的指针
                            size:         buffer 的大小
                            timeout:    SYS_QUEUE_NO_WAIT
 * 输出参数: 
 * 返回数值: 
                            OK : 
                            ERROR : 
 * 其           它:  
                            未实现输入参数size的检查，自行确保输入数据大小未超过queue的大小
                            timeout 参数暂不支持毫秒等待
**************************************************************/
RTN_STATUS
tf_queue_put (
                INT32  queue_id, 
                void   *p_data, 
                UINT32 size, 
                INT32  timeout)
{
	/* Check Parameters */
	if (p_data == NULL) 
    {
		QUEUE_PRINTF("Can not put NULL msg into the queue. \r\n");
		return RTN_ERROR;
	}

    /* kth.debug, maybe need to check size */
	if(msgsnd(queue_id, (INT8 *)p_data, size, (SYS_QUEUE_NO_WAIT == timeout) ? IPC_NOWAIT : 0) < 0)
	{
		QUEUE_PRINTF("Send msg failed.\r\n");
		return RTN_ERROR;
	}

	return RTN_OK;
}


