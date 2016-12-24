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
	
	ppbuf->loading_buf = BUF_NONE;
	ppbuf->playload_buf = BUF_NONE;
}

void switch_receivebuf( PPBuf_t *ppbuf, char **buf, short *len)
{
	if( ppbuf->loading_buf == BUF_NONE)
	{
		*buf = ppbuf->ping_buf;
		*len = ppbuf->ping_len;
		ppbuf->loading_buf = BUF_PING;
		ppbuf->playload_buf = BUF_NONE;
	}
	else if( ppbuf->loading_buf == BUF_PING)
	{
		//��һ����������ݱ�ȡ���˲��л���ȥ
		if( ppbuf->playload_buf != BUF_PONG)
		{
			*buf = ppbuf->pong_buf;
			*len = ppbuf->pong_len;
			ppbuf->loading_buf = BUF_PONG;
			//���˻�����Ϊ�����غɻ���
			ppbuf->playload_buf = BUF_PING;
		}
		
	}
	else if( ppbuf->loading_buf == BUF_PONG)
	{
		//��һ����������ݱ�ȡ���˲��л���ȥ
		if( ppbuf->playload_buf != BUF_PING)
		{
			*buf = ppbuf->ping_buf;
			*len = ppbuf->ping_len;
			ppbuf->loading_buf = BUF_PING;
			
			ppbuf->playload_buf = BUF_PONG;
		}
			
	}
	else
	{
		*buf = ppbuf->ping_buf;
		*len = ppbuf->ping_len;
		ppbuf->loading_buf = BUF_PING;
		ppbuf->playload_buf = BUF_NONE;
	}
	
}



char *get_playloadbuf( PPBuf_t *ppbuf)
{
	
	if( ppbuf->playload_buf == BUF_PING)
	{
		return ppbuf->ping_buf;
		
	}
	else
	{
		return ppbuf->pong_buf;
		
	}
	ppbuf->playload_buf = BUF_NONE;
}
void free_playloadbuf( PPBuf_t *ppbuf)
{
	if( ppbuf->playload_buf == BUF_PING)
	{
		memset( ppbuf->ping_buf, 0, ppbuf->ping_len);
	}
	else
	{
		memset( ppbuf->ping_buf, 0, ppbuf->pong_len);
		
	}
	ppbuf->playload_buf = BUF_NONE;
	
}
//��ping��pong��װ�ر�־��Ϊ1��ʱ��
//ping���������δ����ȡ
//pong������Ϊ���ջ���һֱ�ڱ��������ݸ���
int get_loadbuflen( PPBuf_t *ppbuf)
{
	if( ppbuf->playload_buf == BUF_PING)
	{
		return ppbuf->ping_len;
	}
	else
	{
		return ppbuf->pong_len;
	}
	
}