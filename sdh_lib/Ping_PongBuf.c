/**
* @file 		Ping_PongBuf.c
* @brief		ʵ��˫RAM
* @details		1. ��ping��pong��û������ʹ�õ�ʱ������ʹ��ping�� 
*				2. ��ping��pong����װ��������ʱ�����ݶ�û��Ӧ�ó���ȡ�ߵ�ʱ�򣬰�pong�Ļ������ݷ�����
* @author		author
* @date		date
* @version	A001
* @par Copyright (c): 
* 		XXX??
* @par History:         
*	version: author, date, desc\n
*/
#include "Ping_PongBuf.h"
#include <string.h>
void  init_pingponfbuf( PPBuf_t *ppbuf)
{
	ppbuf->ping_status = PPBUF_STATUS_IDLE;
	ppbuf->pong_status = PPBUF_STATUS_IDLE;
	ppbuf->loading_buf = BUF_NONE;
	ppbuf->playload_buf = BUF_NONE;
}
//
void switch_receivebuf( PPBuf_t *ppbuf, char **buf, short *len)
{
	if( ppbuf->ping_status == PPBUF_STATUS_IDLE)
	{
		if( ppbuf->loading_buf == BUF_PONG)	//��һ��װ�صĻ�����PONG��˵���Ѿ������ݴ�����
		{
			ppbuf->playload_buf = BUF_PONG;
		}
		*buf = ppbuf->ping_buf;
		*len = ppbuf->ping_len;
		ppbuf->loading_buf = BUF_PING;
		ppbuf->ping_status = PPBUF_STATUS_LOADING;
		
	}
	else if( ppbuf->pong_status == PPBUF_STATUS_IDLE)
	{
		if( ppbuf->loading_buf == BUF_PING)	//��һ��װ�صĻ�����PING��˵���Ѿ������ݴ�����
		{
			ppbuf->playload_buf = BUF_PING;
		}
		*buf = ppbuf->pong_buf;
		*len = ppbuf->pong_len;
		ppbuf->loading_buf = BUF_PONG;
		ppbuf->pong_status = PPBUF_STATUS_LOADING;
		
	}
	else		//û�п��еĻ��������Ͳ��л�
	{
		//������һ��ʹ�õĻ���
		if( ppbuf->loading_buf == BUF_PING)
		{
		
			*buf = ppbuf->ping_buf;
			*len = ppbuf->ping_len;
		}
		else if( ppbuf->loading_buf == BUF_PONG)
		{
			*buf = ppbuf->pong_buf;
			*len = ppbuf->pong_len;
			
		}
			
		
	}
}



char *get_playloadbuf( PPBuf_t *ppbuf)
{
	
	if( ppbuf->playload_buf == BUF_PING)
	{
		ppbuf->ping_status = PPBUF_STATUS_IDLE;
		return ppbuf->ping_buf;
		
	}
	else if( ppbuf->playload_buf == BUF_PONG)
	{
		ppbuf->pong_status = PPBUF_STATUS_IDLE;
		return ppbuf->pong_buf;
		
	}
	else
	{
		
		return ppbuf->ping_buf;
	}
}
void free_playloadbuf( PPBuf_t *ppbuf)
{
//	if( ppbuf->playload_buf == BUF_PING)
//	{
//		memset( ppbuf->ping_buf, 0, ppbuf->ping_len);
//	}
//	else if( ppbuf->playload_buf == BUF_PONG)
//	{
//		memset( ppbuf->pong_buf, 0, ppbuf->pong_len);
//		
//	}
//	ppbuf->playload_buf = BUF_NONE;
	
}
//��ping��pong��װ�ر�־��Ϊ1��ʱ��
//ping���������δ����ȡ
//pong������Ϊ���ջ���һֱ�ڱ��������ݸ���
int get_loadbuflen( PPBuf_t *ppbuf)
{
//	if( ppbuf->playload_buf == BUF_PING)
//	{
		return ppbuf->ping_len;
//	}
//	else
//	{
//		return ppbuf->pong_len;
//	}
	
}
