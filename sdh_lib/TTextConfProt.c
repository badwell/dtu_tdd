/**
* @file 		TTextConfProt.c
* @brief		�ı�����Э�飬�ο�AT֮��Ĺ���.
* @details	��Э��Դ��dtu������Э��,����֮���Ѵ�������ķָ����滻��'\0'.һ�����������У������������������
* @author		author
* @date		date
* @version	A001
* @par Copyright (c): 
* 		XXX��˾
* @par History:         
*	version: author, date, desc

*/
#include "TTextConfProt.h"
#include "string.h"
#include "sdhError.h"
static Atcmd_t	Decode_Atcmd;


int decodeTTCP_begin (char *cmd)
{
	int	local = 0;
	int cmd_len = strlen( cmd);
	char *pdeal = cmd;
	char* ptmp = NULL;
	
	TTCP_MUTEX_LOCK;
	Decode_Atcmd.cmd_type = CONFCMD_TYPE_NONE;
	if( cmd == NULL)
	{
		goto exit_fail;
	}
	
	local = strcspn( pdeal, CMD_LETTER_SET);	
	if( local >= cmd_len)		//�ַ�����ͷ����β���Ҳ���
		goto exit_fail;
	
	pdeal += local;
	
	//ATC��atc
	ptmp = strstr((const char*)pdeal,"ATC");	
	if( ptmp == NULL)
	{
		
		ptmp = strstr((const char*)pdeal,"atc");	
		if( ptmp == NULL)
			goto exit_fail;
	}
	ptmp += 3;
	pdeal = ptmp;
	
	//ATC+
	if( *pdeal != '+')
		goto exit_fail;
	pdeal ++;
	
	//ATC+XXX=YYY
	Decode_Atcmd.cmd_type = CONFCMD_TYPE_ATC;
	Decode_Atcmd.cmd = pdeal;
	while( *pdeal)
	{
		if( *pdeal == OFS_CMD_ARG)
			*pdeal = '\0';
		else
			pdeal ++;
		
	}
	pdeal ++;
	if( *pdeal != '\0')
		Decode_Atcmd.arg = pdeal;
	else
		Decode_Atcmd.arg = NULL;
	
	return ERR_OK;
	
	exit_fail:
	TTCP_MUTEX_UNLOCK;
	return ERR_BAD_PARAMETER;
	
}
int get_cmdtype(void)
{
	return Decode_Atcmd.cmd_type;
}
char	*get_cmd(void)
{
	if( Decode_Atcmd.cmd_type == CONFCMD_TYPE_NONE)
	{
		TTCP_MUTEX_UNLOCK;
		return NULL;
	}
	
	return Decode_Atcmd.cmd;
	
}

char *get_firstarg(void)
{
	char *pdeal;
	char	*parg;
	if( Decode_Atcmd.cmd_type == CONFCMD_TYPE_NONE)
	{
		TTCP_MUTEX_UNLOCK;
		return NULL;
	}
	if( Decode_Atcmd.arg == NULL)
	{
		return NULL;
	}
	pdeal = Decode_Atcmd.arg;
	parg =  Decode_Atcmd.arg;
	
	while( *pdeal)
	{
		if( *pdeal == OFS_ARG_ARG)
			*pdeal = '\0';
		else
			pdeal ++;
		
	}
	
	pdeal ++;
	if( *pdeal != '\0')
		Decode_Atcmd.arg = pdeal;		//
	else
		Decode_Atcmd.arg = NULL;
	
	return parg;
}

void decodeTTCP_finish(void)
{
	
	TTCP_MUTEX_UNLOCK;
	Decode_Atcmd.cmd_type = CONFCMD_TYPE_NONE;
	Decode_Atcmd.arg = NULL;
	Decode_Atcmd.cmd = NULL;
}


