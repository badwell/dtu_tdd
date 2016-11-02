
#include "cmsis_os.h"                                           // CMSIS RTOS header file
#include "osObjects.h"                      // RTOS object definitions
#include "gprs.h"
#include "sdhError.h"
#include "dtuConfig.h"
#include "string.h"
#include "debug.h"
#include "sw_filesys.h"
#include "TTextConfProt.h"
#include "times.h"

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
#define DTU_BUF_LEN		512
#define HEARTBEAT_ALARMID		0
#define SER485_WORKINGMODE_ALARMID		1			//485��Ĭ��ģʽת��������ģʽ��ʱ��
 
static int get_dtuCfg(DtuCfg_t *conf);
static void dtu_conf(void);


void thrd_dtu (void const *argument);                             // thread function
osThreadId tid_ThrdDtu;                                          // thread id
osThreadDef (thrd_dtu, osPriorityNormal, 1, 0);                   // thread object

static uint32_t Heatbeat_count = 0;
gprs_t *SIM800 ;
DtuCfg_t	Dtu_config;
sdhFile *DtuCfg_file;

char	DTU_Buf[DTU_BUF_LEN];
char	Recv_PhnoeNo[16];
int Init_ThrdDtu (void) {
	int retry = 20;
	SIM800 = gprs_t_new();
	SIM800->init(SIM800);
	s485_uart_init( &Conf_S485Usart_default);
	
	
	//todo  :���ֻ�ǹ����ڱ���ģʽ����û�к�SIM900����û�в���SIM����ʱ������ͻᵼ��һֱ��ѭ������Ҫ������������
	while(retry)
	{
		if( SIM800->startup(SIM800) != ERR_OK)
		{
			retry--;
			osDelay(1000);
		}
		else if( SIM800->check_simCard(SIM800) == ERR_OK)
		{	
			
			break;
		}
		else {
			retry --;
			osDelay(1000);
		}
		
	}
	
	
	get_dtuCfg( &Dtu_config);
	clean_time2_flags();

	tid_ThrdDtu = osThreadCreate (osThread(thrd_dtu), NULL);
	if (!tid_ThrdDtu) return(-1);

	return(0);
}


static void prnt_485( char *data)
{
	if( Dtu_config.output_mode)
	{
		
		s485_Uart_write(data, strlen(data) );
	}
	
}

void thrd_dtu (void const *argument) {
	short step = 0;
	short cnnt_seq = 0;

	int ret = 0;
	int lszie = 0;
	short i = 0;
	short ser_confmode = 0;
	char *pp;
	
	
	//���ϵ��ʱ��ȴ��������õ�ģʽ
	//���ָ��ʱ���ڲ��ܽ�������ģʽ�����˳��ȴ���������������ģʽ
	//һ����������ģʽ���Ͳ����˳�������ģʽ���û�ֻ��ͨ���������˳�����ģʽ
	strcpy( DTU_Buf, " wait for signal ...");
	s485_Uart_write(DTU_Buf, strlen(DTU_Buf) );
	osDelay(10);
	set_alarmclock_s( SER485_WORKINGMODE_ALARMID, 30);
	while( 1)
	{
		if( Ringing_s(SER485_WORKINGMODE_ALARMID) == ERR_OK)
		{
			if( ser_confmode == 0)
				break;
			
		}
		
		ret = s485_Uart_read( DTU_Buf, DTU_BUF_LEN);
		if( ret <=  0)
		{
			continue;
		}
			
		if( ser_confmode == 0)
		{
			if( enter_TTCP( DTU_Buf) == ERR_OK)
			{
				get_TTCPVer( DTU_Buf);
				ser_confmode = 1;
				while( s485_Uart_write(DTU_Buf, strlen(DTU_Buf) ) != ERR_OK)
					;
				osDelay(1);
			}
			
		}
		else
		{
			if( decodeTTCP_begin( DTU_Buf) == ERR_OK)
			{
				dtu_conf();
				decodeTTCP_finish();
				
			}
			
		}
		
		
		
		
	}
	
	//ʹ���û�����������������485����
	s485_uart_init( &Dtu_config.the_485cfg);
	s485_Uart_ioctl(S485_UART_CMD_SET_TXBLOCK);
	s485_Uart_ioctl(S485UART_SET_TXWAITTIME_MS, 2000);
	s485_Uart_ioctl(S485_UART_CMD_CLR_RXBLOCK);
	

	while (1) {
		switch( step)
		{
			case 0:
				
				
				sprintf(DTU_Buf, "cnnnect DC :%d,%s,%d,%s ...", cnnt_seq,Dtu_config.DateCenter_ip[ cnnt_seq],\
								Dtu_config.DateCenter_port[cnnt_seq],Dtu_config.protocol[cnnt_seq] );
				prnt_485( DTU_Buf);
				ret = SIM800->tcp_cnnt( SIM800, cnnt_seq, Dtu_config.DateCenter_ip[cnnt_seq], Dtu_config.DateCenter_port[cnnt_seq]);
				if( Dtu_config.Activestandby_mode && ret == ERR_OK)
				{
					prnt_485("succeed !\n");
					step ++;
					break;
				}
				if( ret == ERR_OK)
				{
					prnt_485(" succeed !\n");
					
				}
				else
				{
					prnt_485(" failed !\n");
				}
				cnnt_seq ++;
				if( cnnt_seq >= IPMUX_NUM)
				{
					step ++;
					cnnt_seq = 0;
				}
				break;
			case 1:
				lszie = DTU_BUF_LEN;
				set_alarmclock_s( HEARTBEAT_ALARMID, Dtu_config.hartbeat_timespan_s);
				ret = SIM800->guard_serial( SIM800, DTU_Buf, &lszie);
				if( ret == tcp_receive)
				{
					ret = SIM800->deal_tcprecv_event( SIM800, DTU_Buf,  DTU_Buf, &lszie);
					if( lszie >= 0)
					{
						
						DPRINTF(" recv : %s \n", DTU_Buf);
						s485_Uart_write(DTU_Buf, lszie);
					}
				
					
				}
				if( ret == tcp_close)
				{
					ret = SIM800->deal_tcpclose_event( SIM800, DTU_Buf, lszie);
				}
				if( ret == sms_urc)
				{
					ret = SIM800->deal_smsrecv_event( SIM800, DTU_Buf, DTU_Buf,  &lszie, Recv_PhnoeNo);
					if( decodeTTCP_begin( DTU_Buf) == ERR_OK)
					{
						dtu_conf();
						decodeTTCP_finish();
						
					}
					else if( Dtu_config.work_mode == MODE_SMS)
					{
						for( i = 0; i < ADMIN_PHNOE_NUM; i ++)
						{
							pp = strstr((const char*)Recv_PhnoeNo, Dtu_config.admin_Phone[i]);
							if( pp)
							{
								s485_Uart_write(DTU_Buf, lszie);
								break;
							}
							
						}
						
						
						
					}
					
					
				}
				step ++;
				break;
			case 2:
				//MODE_LOCALRTU ֮�²���Ҫִ���������裬ֻ��Ҫ��ȡ485�����ݲ�������
				ret = s485_Uart_read( DTU_Buf, DTU_BUF_LEN);
				
				
				if( ret <= 0)
				{
					if( Dtu_config.work_mode != MODE_LOCALRTU)
						step++;
					break;
				}
				
				if( Dtu_config.work_mode == MODE_LOCALRTU)
					break;

				for( i = 0; i < IPMUX_NUM; i ++)
				{
					SIM800->sendto_tcp( SIM800, i, DTU_Buf, ret);
					
				}			
				if(  Dtu_config.work_mode == MODE_SMS)
				{
					for( i = 0; i < ADMIN_PHNOE_NUM; i ++)
					{
						SIM800->send_text_sms( SIM800, Dtu_config.admin_Phone[i], DTU_Buf);
						
					}
					
					
				}
				
				step++;
			case 3:
				if( Ringing_s(HEARTBEAT_ALARMID) == ERR_OK)
				{
					strcpy( DTU_Buf,  Dtu_config.heatbeat_package);
					Heatbeat_count ++;
					for( i = 0; i < IPMUX_NUM; i ++)
					{
						SIM800->sendto_tcp( SIM800, i, DTU_Buf, ret);
						
					}
					
				}	
				step++;
			case 4:
				if( Dtu_config.Activestandby_mode )
				{
					ret = SIM800->get_firstCnt_seq(SIM800);
					
					step = 0;
					if( ret >= 0)
					{
						step = 1;
					}
					break;
				}
				
			
				ret = SIM800->get_firstDiscnt_seq(SIM800);
				if( ret >= 0)
				{
					sprintf(DTU_Buf, "cnnnect DC :%d,%s,%d,%s ...", ret,Dtu_config.DateCenter_ip[ ret],\
								Dtu_config.DateCenter_port[ret],Dtu_config.protocol[ret] );
					prnt_485( DTU_Buf);
					if( SIM800->tcp_cnnt( SIM800, ret, Dtu_config.DateCenter_ip[ret], Dtu_config.DateCenter_port[ret]) == ERR_OK)
					{
						prnt_485(" succeed !\n");
					
					}
					else
					{
						prnt_485(" failed !\n");
					}	
				}
				step = 1;
				break;
			default:
				step = 0;
				break;
			  
	  }
	  
	  
	  osThreadYield ();                                           // suspend thread
	}
}


static void set_default( DtuCfg_t *conf)
{
	//Ĭ�ϵ�����
	int i = 0;
	memset( conf, 0, sizeof( DtuCfg_t));
		 
	conf->ver[0] = DTU_CONFGILE_MAIN_VER;
	conf->ver[1] = DTU_CONFGILE_SUB_VER;
	conf->Activestandby_mode = 1;
	conf->hartbeat_timespan_s = 5;
	conf->work_mode = MODE_DTU;
	conf->dtu_id = 1;
	conf->rtu_addr = 1;
	strcpy( conf->sim_NO,"13888888888");
	sprintf( conf->registry_package,"XMYN%09d%s",conf->dtu_id, conf->sim_NO);
	conf->heatbeat_package[0] = '$';
	conf->heatbeat_package[1] = '\0';
	conf->output_mode = 0;
	conf->chn_type[0] = 1;
	conf->chn_type[1] = 1;
	conf->chn_type[2] = 1;
	memcpy( &conf->the_485cfg, &Conf_S485Usart_default, sizeof( Conf_S485Usart_default));
	
	for( i = 0; i < IPMUX_NUM; i++)
	{
		strcpy( conf->DateCenter_ip[i], DEF_IPADDR);
		strcpy( conf->protocol[i], DEF_PROTOTOCOL);
		conf->DateCenter_port[i] = DEF_PORTNUM;
		
	}
	
}

static int get_dtuCfg(DtuCfg_t *conf)
{
	
	
	DtuCfg_file	= fs_open( DTUCONF_filename);

	if( DtuCfg_file)
	{
		fs_read( DtuCfg_file, (uint8_t *)conf, sizeof( DtuCfg_t));
		if( conf->ver[0] == DTU_CONFGILE_MAIN_VER &&  conf->ver[1] == DTU_CONFGILE_SUB_VER)
		{
			
			return ERR_OK;
		}
	}
	else
	{
		DtuCfg_file	= fs_creator( DTUCONF_filename, sizeof( DtuCfg_t));
		
			
	}
	
	
	
	set_default( conf);
	
	if( DtuCfg_file)
	{
		fs_write( DtuCfg_file, (uint8_t *)conf, sizeof( DtuCfg_t));
		fs_flush();
	}
	
	return ERR_OK;
	
}


void ack_str( char *str)
{
	
	s485_Uart_write(str, strlen(str));
}





static void dtu_conf(void)
{
	char *pcmd;
	char *parg;
	int 	i_data = 0;
	short		i = 0, j = 0;
	char		tmpbuf[8];
	char		com_Wordbits[4] = { '8', '9', 0, 0};
	char		com_Paritybit[4] = { 'N', 'E', 'O', 0};
	char		com_stopbit[4] = { '1', '2',0,0};
	if( get_cmdtype() != CONFCMD_TYPE_ATC)
		return;
	
	pcmd = get_cmd();
	if( pcmd == NULL)
			return;
	
	while(1)
	{
		parg = get_firstarg();
		if( strcmp(pcmd ,"MODE") == 0)
		{
			i_data = MODE_END;
			if( parg)
				i_data = atoi( parg);
			if( *parg == '?')
			{
				sprintf( DTU_Buf, "%d", Dtu_config.work_mode);
				ack_str( DTU_Buf);
			}
			else if( i_data < MODE_END && i_data >= MODE_BEGIN)
			{
				Dtu_config.work_mode = i_data;
				ack_str( "OK");
			}
			else
			{
				ack_str( "ERROR");
			}
			goto exit;
		}
		
		else if( strcmp(pcmd ,"CONN") == 0)
		{
			i_data = 2;
			if( parg)
				i_data = atoi( parg);
			
			if( *parg == '?')
			{
				sprintf( DTU_Buf, "%d", Dtu_config.Activestandby_mode);
				ack_str( DTU_Buf);
			}
			else if( i_data == 0 || i_data == 1)
			{
				Dtu_config.Activestandby_mode = i_data;
				ack_str( "OK");
			}
			else
			{
				ack_str( "ERROR");
			}
			goto exit;
		}
		
		else if( strcmp(pcmd ,"SVDM") == 0 || strcmp(pcmd ,"SVIP") == 0)
		{
			if( parg == NULL)
			{
				
				goto exit;
			}
			if( parg[0] == '?')
			{
				memset( DTU_Buf, 0, DTU_BUF_LEN);
				for( i = 0; i < IPMUX_NUM; i++)
				{
					sprintf(tmpbuf, "%d,", i);
					strcat(DTU_Buf, tmpbuf);
					strcat(DTU_Buf, Dtu_config.DateCenter_ip[ i]);
					strcat(DTU_Buf, ",");
					sprintf(tmpbuf, "%d,", Dtu_config.DateCenter_port[i]);
					strcat(DTU_Buf, tmpbuf);
					strcat(DTU_Buf, Dtu_config.protocol[i]);
					strcat(DTU_Buf, ",");
					
				}
				
				ack_str(DTU_Buf);
				goto exit;
			}
			//N, ip, port, protocol
			
			switch( i)
			{
				case 0:
					i_data = atoi(parg);
					if( i_data < 0 || i_data >= IPMUX_NUM)
					{
						ack_str( "ERROR");
						goto exit;
					}
					i ++;
					break;
				case 1:
					strcpy( Dtu_config.DateCenter_ip[ i_data], parg);
					i ++;
					break;
				case 2:
					Dtu_config.DateCenter_port[ i_data] = atoi(parg);
					i++;
					break;
				case 3:
					if( strcmp(parg ,"TCP") == 0 || strcmp(parg ,"UDP") == 0)
					{
						strcpy( Dtu_config.protocol[ i_data], parg);
						ack_str( "OK");
					}
					else
					{
						ack_str( "ERROR");
						
					}
					goto exit;
				default:
					ack_str( "ERROR");
					goto exit;
			}
				
				
		}
		else if( strcmp(pcmd ,"DNIP") == 0)
		{
			if( parg[0] == '?')
			{
				if( check_ip( Dtu_config.dns_ip) == ERR_OK)
					sprintf( DTU_Buf, "%s",Dtu_config.dns_ip);
				else
					strcpy( DTU_Buf, "  ");
				
				ack_str( DTU_Buf);
			}
			else
			{
				if( check_ip( parg) != ERR_OK)
				{
					ack_str( "ERROR");
				}
				else
				{
					strcpy( Dtu_config.dns_ip, parg);
					ack_str( "OK");
				}
				
			}
			goto exit;
		}
		
		else if( strcmp(pcmd ,"ATBT") == 0)
		{
				
			if( parg == NULL)
			{
				ack_str( "OK");
				goto exit;
			}
			if( parg[0] == '?')
			{
				i_data = 0;								
				sprintf( DTU_Buf, "%09d,%d,%s", Dtu_config.dtu_id, Dtu_config.hartbeat_timespan_s, Dtu_config.sim_NO );
				for( i = 0; i < ADMIN_PHNOE_NUM; i ++)
				{
					
					if( check_phoneNO( Dtu_config.admin_Phone[i]) == ERR_OK)
					{
						strcat( DTU_Buf,",");
						strcat( DTU_Buf, Dtu_config.admin_Phone[i]);
						
					}
					
				}
				ack_str( DTU_Buf);
				goto exit;
				
			}
			
			switch(i)
			{
				case 0:
					i_data = atoi( parg);
					Dtu_config.dtu_id = i_data;
					i++;
					break;
				case 1:
					i_data = atoi( parg);
					Dtu_config.hartbeat_timespan_s = i_data;
					i++;
					break;
				case 2:
					if( check_phoneNO(parg) != ERR_OK)
					{
						ack_str( "ERROR");
						goto exit;
					}
					copy_phoneNO( Dtu_config.sim_NO, parg);
					
					memset( Dtu_config.admin_Phone[0], 0, sizeof(Dtu_config.admin_Phone));
					i++;
					i_data = 0;
					break;
				case 3:
					if( check_phoneNO(parg) != ERR_OK)
					{
						ack_str( "ERROR");
						goto exit;
					}
					copy_phoneNO( Dtu_config.admin_Phone[ i_data], parg);
					i_data ++;
					
					if( i_data >= ADMIN_PHNOE_NUM)
					{
						ack_str( "OK");
						goto exit;
					}
					break;
				default:
					ack_str( "ERROR");
					goto exit;
					
			}		//switch
			
		}
		
		else if( strcmp(pcmd ,"PRNT") == 0)
		{
			if( parg[0] == '?')
			{
				if( Dtu_config.output_mode)
				{
					ack_str( "YES");
					
				}
				else
				{
					ack_str( "NO");
				}
				goto exit;
				
			}
			
			
			if( strcmp(parg ,"YES") == 0)
			{
				Dtu_config.output_mode = 1;
				ack_str( "OK");
			}
			else if( strcmp(parg ,"NO") == 0)
			{
				Dtu_config.output_mode = 0;
				ack_str( "OK");
			}
			else
			{
				ack_str( "ERROR");
			}
			goto exit;
		}
		
		else if( strcmp(pcmd ,"SCOM") == 0)
		{
			if( parg == NULL)
			{
				ack_str( "OK");
				goto exit;
			}
			
			if( parg[0] == '?')
			{
				
				sprintf( DTU_Buf, "%d,%c,%c,%c", Dtu_config.the_485cfg.USART_BaudRate, \
												 com_Wordbits[Dtu_config.the_485cfg.USART_WordLength/0x1000], \
												 com_Paritybit[Dtu_config.the_485cfg.USART_Parity/0x300], \
												 com_stopbit[Dtu_config.the_485cfg.USART_StopBits/0x2000]	);
	
				
				ack_str(DTU_Buf);
				
				goto exit;
				
			}
			
			switch(i)
			{
				case 0:
					i_data = atoi( parg);
					Dtu_config.the_485cfg.USART_BaudRate = i_data;
					i++;
					break;
				case 1:
					if( parg[0] == '8')
						Dtu_config.the_485cfg.USART_WordLength = USART_WordLength_8b;
					else if( parg[0] == '9')
						Dtu_config.the_485cfg.USART_WordLength = USART_WordLength_9b;
					else
					{
						ack_str( "ERROR");
						goto exit;
					}
					i++;
					break;
				case 2:
					if( parg[0] == 'N')
						Dtu_config.the_485cfg.USART_Parity = USART_Parity_No;
					else if( parg[0] == 'E')
						Dtu_config.the_485cfg.USART_Parity = USART_Parity_Even;
					else if( parg[0] == 'O')
						Dtu_config.the_485cfg.USART_Parity = USART_Parity_Odd;
					else
					{
						ack_str( "ERROR");
						goto exit;
					}
						
					i++;
					break;
				case 3:
					if( parg[0] == '1')
						Dtu_config.the_485cfg.USART_StopBits = USART_StopBits_1;
					else if( parg[0] == '2')
						Dtu_config.the_485cfg.USART_StopBits = USART_StopBits_2;
					else
					{
						ack_str( "ERROR");
						goto exit;
					}
					i++;
					break;
				default:
					ack_str( "ERROR");
					goto exit;
					
			}		//switch
			
		}
		
		else if( strcmp(pcmd ,"REGT") == 0)
		{
			if( *parg == '?')
			{
				sprintf( DTU_Buf, "%s", Dtu_config.registry_package);
				ack_str( DTU_Buf);
			}
			else
			{
				i =  strlen(parg);
				if( i > 31)
					i = 31;
						
				memcpy( Dtu_config.registry_package, parg, i );
				Dtu_config.registry_package[31] = 0;
				ack_str( "OK");
			}
			
			goto exit;
		}
		
		else if( strcmp(pcmd ,"HEAT") == 0)
		{
			if( *parg == '?')
			{
				sprintf( DTU_Buf, "%s", Dtu_config.heatbeat_package);
				ack_str( DTU_Buf);
			}
			else
			{
				i =  strlen(parg);
				if( i > 31)
					i = 31;
						
				memcpy( Dtu_config.heatbeat_package, parg, i );
				Dtu_config.heatbeat_package[31] = 0;
				ack_str( "OK");
			}
			
			goto exit;
		}
		
		else if( strcmp(pcmd ,"VAPN") == 0)
		{
			if( parg == NULL)
			{
				ack_str( "OK");
				goto exit;
			}
			if( parg[0] == '?')
			{
				SIM800->get_apn( SIM800, DTU_Buf);
				
					
				ack_str( DTU_Buf);
					
				
				
			}
			else
			{
				if( i == 0)
				{
					strcpy( Dtu_config.apn, parg);
					
				}
				else
				{
					
					strcat( Dtu_config.apn, ",");
					strcat( Dtu_config.apn, parg);
				}
				i ++;
				
			}
			
			
		}
		
		else if( strcmp(pcmd ,"CNUM") == 0)
		{
			if( *parg == '?')
			{
				SIM800->read_smscAddr( SIM800, Dtu_config.smscAddr);
				ack_str( Dtu_config.smscAddr);
			}
			else
			{
				if( check_phoneNO( parg) == ERR_OK)
				{
					strcpy( Dtu_config.smscAddr, parg);
					ack_str( "OK");
				}
				else
				{
					ack_str( "ERROR");
				}
				
			}
			
			goto exit;
		}
		else if( strcmp(pcmd ,"RTUA") == 0)
		{
			if( *parg == '?')
			{
				sprintf( DTU_Buf, "%d", Dtu_config.rtu_addr);
				ack_str( DTU_Buf);
			}
			else
			{
				i_data = atoi( parg);
				if( i_data > 0)
				{
					Dtu_config.rtu_addr = i_data;
					ack_str( "OK");
				}
				else
				{
					ack_str( "ERROR");
				}
			}
			
			goto exit;
		}
		else if( strcmp(pcmd ,"SIGN") == 0)
		{
			if( *parg == '?')
			{
				sprintf( DTU_Buf, "%d,%d,%d", Dtu_config.chn_type[0],Dtu_config.chn_type[1],Dtu_config.chn_type[2] );
				ack_str( DTU_Buf);
			}
			else
			{
				switch(i)
				{
					case 0:
						j = atoi( parg);
						if( j >2 || j < 0)
						{
							ack_str( "ERROR");
							goto exit;
						}
						i++;
						break;
					case 1:
						i_data = atoi( parg);
						if( i_data != 1 && i_data != 2)
						{
							ack_str( "ERROR");
							goto exit;
						}
						Dtu_config.chn_type[j] = i_data;
						ack_str( "OK");
						goto exit;
					default:
						ack_str( "ERROR");
						goto exit;
						
				}		//switch
				
				
				
			}
			goto exit;
		}
		else if( strcmp(pcmd ,"FACT") == 0)
		{
			set_default(&Dtu_config);
			ack_str( "OK");
			goto exit;
		}
		else if( strcmp(pcmd ,"REST") == 0)
		{
			
			ack_str( "OK");
			fs_flush();
			SIM800->shutdown(SIM800);
			os_reboot();
			goto exit;
		}
		if( parg == NULL)
		{
			ack_str( "ERROR");
			goto exit;
		}

		
	}

	exit:	
	fs_write( DtuCfg_file, (uint8_t *)&Dtu_config, sizeof( DtuCfg_t));
	fs_flush();
	
}
