/**
* @file 		gprs_uart.c
* @brief		�ṩΪgprsģ�����Ĵ��ڹ���.
* @details	
* @author		sundh
* @date		16-09-20
* @version	A001
* @par Copyright (c): 
* 		XXX??
* @par History:         
*	version: author, date, desc\n
* 	A001:sundh,16-09-20,��ʼ������
*/
#include "chipset.h"
#include "stdint.h"
#include "serial485_uart.h"
#include "hardwareConfig.h"
#include "stm32f10x_usart.h"
#include "sdhError.h"
#include "osObjects.h"                      // RTOS object definitions
#include <stdarg.h>
#include <string.h>
#include "def.h"




osSemaphoreId SemId_s485txFinish;                         // Semaphore ID
uint32_t os_semaphore_cb_Sem_s485txFinish[2] = { 0 }; 
const osSemaphoreDef_t os_semaphore_def_Sem_s485txFinish = { (os_semaphore_cb_Sem_s485txFinish) };



osSemaphoreId SemId_s485rxFrame;                         // Semaphore ID
uint32_t os_semaphore_cb_Sem_s485rxFrame[2] = { 0 }; 
const osSemaphoreDef_t os_semaphore_def_Sem_s485rxFrame = { (os_semaphore_cb_Sem_s485rxFrame) };

char	S485Uart_buf[S485_UART_BUF_LEN];			//����DMA���ջ���
static struct usart_control_t {
	short	tx_block;		//������־
	short	rx_block;
	
	uint16_t	recv_size;
	short	tx_waittime_ms;
	short	rx_waittime_ms;
	

	
}S485_uart_ctl;


static void DMA_s485Uart_Init(void);


/*!
**
**
** @param txbuf ���ͻ����ַ
** @param rxbuf ���ܻ����ַ
** @return
**/
int s485_uart_init(void)
{
	SemId_s485txFinish = osSemaphoreCreate(osSemaphore(Sem_s485txFinish), 1);
	SemId_s485rxFrame = osSemaphoreCreate(osSemaphore(Sem_s485rxFrame), 1);
	

	
	USART_DeInit( SER485_USART);
	
	DMA_s485Uart_Init();

	USART_Init( SER485_USART, &Conf_S485Usart);
	
	
	USART_ITConfig( SER485_USART, USART_IT_RXNE, ENABLE);
	USART_ITConfig( SER485_USART, USART_IT_IDLE, ENABLE);
	USART_DMACmd(SER485_USART, USART_DMAReq_Tx, ENABLE);  // ����DMA����
	USART_DMACmd(SER485_USART, USART_DMAReq_Rx, ENABLE); // ����DMA����
	USART_Cmd(SER485_USART, ENABLE);
	
	

	
	S485_uart_ctl.rx_block = 1;
	S485_uart_ctl.tx_block = 1;
	S485_uart_ctl.rx_waittime_ms = 100;
	S485_uart_ctl.tx_waittime_ms = 100;
	
	return ERR_OK;
}


/*!
**
**
** @param data 
** @param size 
** @return
**/
int s485_Uart_write(char *data, uint16_t size)
{
	int ret;
	if( data == NULL)
		return ERR_BAD_PARAMETER;
	DMA_s485_usart.dma_tx_base->CMAR = (uint32_t)data;
	DMA_s485_usart.dma_tx_base->CNDTR = (uint16_t)size; 
	DMA_Cmd( DMA_s485_usart.dma_tx_base, ENABLE);        //��ʼDMA����

	
	if( S485_uart_ctl.tx_block)
	{
		if( S485_uart_ctl.tx_waittime_ms == 0)
			ret = osSemaphoreWait( SemId_s485txFinish, osWaitForever );
		else
			ret = osSemaphoreWait( SemId_s485txFinish, S485_uart_ctl.tx_waittime_ms );
		
		if ( ret > 0) 
		{
			return ERR_OK;
		}
		
		
		return ERR_DEV_TIMEOUT;
		
	}
	
	return ERR_OK;
}



/*!
**
**
** @param data 
** @param size 
** @return
**/
int s485_Uart_read(char *data, uint16_t size)
{
	int  ret;
	int len = size;
	if( data == NULL)
		return ERR_BAD_PARAMETER;
	
//	memset( data, 0, size);
	
	if( S485_uart_ctl.rx_block)
	{
		if( S485_uart_ctl.rx_waittime_ms == 0)
			ret = osSemaphoreWait( SemId_s485rxFrame, osWaitForever );
		else
			ret = osSemaphoreWait( SemId_s485rxFrame, S485_uart_ctl.rx_waittime_ms );
		
		
		if( ret < 0)
		{
			return ERR_DEV_TIMEOUT;
		}
		
	}
	else 
		ret = osSemaphoreWait( SemId_s485rxFrame, 0 );
	
	if( ret > 0)
	{
		if( len > S485_uart_ctl.recv_size)
			len = S485_uart_ctl.recv_size;
		memset( data, 0, size);
		memcpy( data, S485Uart_buf, len);
		memset( S485Uart_buf, 0, len);
		return len;
	}
	
	return 0;
}


/*!
**
**
** @param size
**
** @return
**/
void s485_Uart_ioctl(int cmd, ...)
{
	int int_data;
	va_list arg_ptr; 
	va_start(arg_ptr, cmd); 
	
	switch(cmd)
	{
		case S485_UART_CMD_SET_TXBLOCK:
			S485_uart_ctl.tx_block = 1;
			break;
		case S485_UART_CMD_CLR_TXBLOCK:
			S485_uart_ctl.tx_block = 0;
			break;
		case S485_UART_CMD_SET_RXBLOCK:
			S485_uart_ctl.rx_block = 1;
			break;
		case S485_UART_CMD_CLR_RXBLOCK:
			S485_uart_ctl.rx_block = 0;
			break;
		case S485UART_SET_TXWAITTIME_MS:
			int_data = va_arg(arg_ptr, int);
			va_end(arg_ptr); 
			S485_uart_ctl.tx_waittime_ms = int_data;
			break;
		case S485UART_SET_RXWAITTIME_MS:
			int_data = va_arg(arg_ptr, int);
			va_end(arg_ptr); 
			S485_uart_ctl.rx_waittime_ms = int_data;
			break;
		default: break;
		
	}
}



/*!
**
**
** @param size
**
** @return
**/
int s485_uart_test(char *buf, int size)
{
	char *pp = NULL;
    int len;
	
	
	strcpy( buf, "Serial 485 test\n" );
	s485_Uart_ioctl( S485UART_SET_TXWAITTIME_MS, 0);
	s485_Uart_ioctl( S485UART_SET_RXWAITTIME_MS, 0);
	s485_Uart_write( buf, strlen(buf));
	
	while(1)
	{
		len = s485_Uart_read( buf, size);
		pp = strstr((const char*)buf,"OK");
		if(pp)
			return ERR_OK;
		
		if( len > 0)
			s485_Uart_write( buf, len);
	}
	
}

/*! gprs uart dma Configuration*/
void DMA_s485Uart_Init(void)
{

		DMA_InitTypeDef DMA_InitStructure;	

    /* DMA clock enable */

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); // ??DMA1??
//=DMA_Configuration==============================================================================//	
/*--- UART_Tx_DMA_Channel DMA Config ---*/
    DMA_Cmd( DMA_s485_usart.dma_tx_base, DISABLE);                           // �ر�DMA
    DMA_DeInit(DMA_s485_usart.dma_tx_base);                                 // �ָ���ʼֵ
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&SER485_USART->DR);// �����ַ
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)S485Uart_buf;        
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;                      // ���ڴ浽����
    DMA_InitStructure.DMA_BufferSize = S485_UART_BUF_LEN;                    
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        // �����ַ������
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 // �ڴ��ַ����
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // �������ݿ��1B
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         // �ڴ��ַ���1B
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                           // ���δ���ģʽ
    DMA_InitStructure.DMA_Priority = DMA_Priority_Low;                 // ���ȼ�
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                            // �ر��ڴ浽�ڴ�ģʽ
    DMA_Init(DMA_s485_usart.dma_tx_base, &DMA_InitStructure);               // 

    DMA_ClearFlag( DMA_s485_usart.dma_tx_flag );                                 // �����־

	DMA_Cmd(DMA_s485_usart.dma_tx_base, DISABLE); 												// �ر�DMA

    DMA_ITConfig(DMA_s485_usart.dma_tx_base, DMA_IT_TC, ENABLE);            // ����������ж�

   

/*--- UART_Rx_DMA_Channel DMA Config ---*/

 

    DMA_Cmd(DMA_s485_usart.dma_rx_base, DISABLE);                           
    DMA_DeInit(DMA_s485_usart.dma_rx_base);                                 
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&SER485_USART->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)S485Uart_buf;         
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                     
    DMA_InitStructure.DMA_BufferSize = S485_UART_BUF_LEN;                     
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; 
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                           
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;                 
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                            
    DMA_Init(DMA_s485_usart.dma_rx_base, &DMA_InitStructure);               
    DMA_ClearFlag( DMA_s485_usart.dma_rx_flag);      
	DMA_ITConfig(DMA_s485_usart.dma_rx_base, DMA_IT_TC, ENABLE); 	 // ����������ж�

    DMA_Cmd(DMA_s485_usart.dma_rx_base, ENABLE);                            

   

}





void DMA1_Channel7_IRQHandler(void)
{

    if(DMA_GetITStatus(DMA1_FLAG_TC7))
    {

        DMA_ClearFlag(DMA_s485_usart.dma_tx_flag);         // �����־
		DMA_Cmd( DMA_s485_usart.dma_tx_base, DISABLE);   // �ر�DMAͨ��
		osSemaphoreRelease( SemId_s485txFinish);
    }
}

//dma�����������Ժ󣬾Ͳ�Ҫȥ����ʣ�µ��ֽ���
//void DMA1_Channel6_IRQHandler(void)
//{

//    if(DMA_GetITStatus(DMA1_FLAG_TC6))
//    {
//		DMA_Cmd(DMA_s485_usart.dma_rx_base, DISABLE); 
//        DMA_ClearFlag(DMA_s485_usart.dma_rx_flag);         // �����־
//    }
//}

void USART2_IRQHandler(void)
{
	uint8_t clear_idle = clear_idle;
	if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)  // �����ж�
	{

		
		DMA_Cmd(DMA_s485_usart.dma_rx_base, DISABLE);       // �ر�DMA
		DMA_ClearFlag( DMA_s485_usart.dma_rx_flag );           // ���DMA��־
		S485_uart_ctl.recv_size = S485_UART_BUF_LEN - DMA_GetCurrDataCounter(DMA_s485_usart.dma_rx_base); //��ý��յ����ֽ�
		DMA_s485_usart.dma_rx_base->CNDTR = S485_UART_BUF_LEN;
		DMA_Cmd( DMA_s485_usart.dma_rx_base, ENABLE);
		
		clear_idle = SER485_USART->SR;
		clear_idle = SER485_USART->DR;
		USART_ReceiveData( USART2 ); // Clear IDLE interrupt flag bit
		
		osSemaphoreRelease( SemId_s485rxFrame);
	}

}



