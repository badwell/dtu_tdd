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
#include "gprs_uart.h"
#include "hardwareConfig.h"
#include "stm32f10x_usart.h"
#include "sdhError.h"
#include "osObjects.h"                      // RTOS object definitions
#include <stdarg.h>

#define SIGNAL_GPRS_UART_RECVFRAM		1
#define SIGNAL_GPRS_UART_TXFINISH		2

static void DMA_GprsUart_Init(void);

struct usart_control_t {
	short	tx_block;		//������־
	short	rx_block;
	
	uint16_t	read_size;
	uint16_t	recv_size;
	short	tx_waittime_ms;
	short	rx_waittime_ms;
	osThreadId	rx_tid, tx_tid;
	
	
}Gprs_uart_ctl;

/*!
**
**
** @param txbuf ���ͻ����ַ
** @param rxbuf ���ܻ����ַ
** @return
**/
int gprs_uart_init(void)
{
	
	USART_DeInit( GPRS_USART);
	
	DMA_GprsUart_Init();

	USART_Init( GPRS_USART, &Conf_GprsUsart);
	
	
	USART_ITConfig( GPRS_USART, USART_IT_RXNE, ENABLE);
	USART_ITConfig( GPRS_USART, USART_IT_IDLE, ENABLE);
	USART_DMACmd(GPRS_USART, USART_DMAReq_Tx, ENABLE);  // ����DMA����
  USART_DMACmd(GPRS_USART, USART_DMAReq_Rx, ENABLE); // ����DMA����
	
	Gprs_uart_ctl.rx_block = 1;
	Gprs_uart_ctl.tx_block = 1;
	Gprs_uart_ctl.rx_waittime_ms = 100;
	Gprs_uart_ctl.tx_waittime_ms = 100;
	
	return ERR_OK;
}


/*!
**
**
** @param data 
** @param size 
** @return
**/
int gprs_Uart_write(char *data, uint16_t size)
{
	osEvent evt;
	if( data == 0)
		return ERR_BAD_PARAMETER;
	DMA_gprs_usart.dma_tx_base->CMAR = (uint32_t)data;
	DMA_gprs_usart.dma_tx_base->CNDTR = (uint16_t)size; 
	DMA_Cmd( DMA_gprs_usart.dma_tx_base, ENABLE);        //��ʼDMA����

	Gprs_uart_ctl.tx_tid =  osThreadGetId();
	if( Gprs_uart_ctl.tx_block)
	{
		if( Gprs_uart_ctl.tx_waittime_ms == 0)
			evt = osSignalWait( SIGNAL_GPRS_UART_TXFINISH, osWaitForever );
		else
			evt = osSignalWait( SIGNAL_GPRS_UART_TXFINISH, Gprs_uart_ctl.tx_waittime_ms );
		
		if (evt.status == osEventSignal) 
		{
			return ERR_OK;
		}
		if( evt.status == osEventTimeout)
		{
			return ERR_DEV_TIMEOUT;
		}
		
		return ERR_UNKOWN;
		
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
int gprs_Uart_read(char *data, uint16_t size)
{
	osEvent evt;
	if( data == 0)
		return ERR_BAD_PARAMETER;
	DMA_gprs_usart.dma_rx_base->CMAR = (uint32_t)data;
	DMA_gprs_usart.dma_rx_base->CNDTR = (uint16_t)size; 
	DMA_Cmd( DMA_gprs_usart.dma_rx_base, ENABLE);        //����DMAͨ�����ȴ���������
	Gprs_uart_ctl.rx_tid =  osThreadGetId();
	Gprs_uart_ctl.read_size = size;
	if( Gprs_uart_ctl.rx_block)
	{
		if( Gprs_uart_ctl.rx_waittime_ms == 0)
			evt = osSignalWait( SIGNAL_GPRS_UART_RECVFRAM, osWaitForever );
		else
			evt = osSignalWait( SIGNAL_GPRS_UART_RECVFRAM, Gprs_uart_ctl.rx_waittime_ms );
		
		if (evt.status == osEventSignal) 
		{
			return Gprs_uart_ctl.recv_size;
		}
		if( evt.status == osEventTimeout)
		{
			return ERR_DEV_TIMEOUT;
		}
		return ERR_UNKOWN;
		
	}
	
	
	return ERR_OK;
}


/*!
**
**
** @param size
**
** @return
**/
void gprs_Uart_ioctl(int cmd, ...)
{
	int int_data;
	va_list arg_ptr; 
	va_start(arg_ptr, cmd); 
	
	switch(cmd)
	{
		case GPRS_UART_CMD_SET_TXBLOCK:
			Gprs_uart_ctl.tx_block = 1;
			break;
		case GPRS_UART_CMD_CLR_TXBLOCK:
			Gprs_uart_ctl.tx_block = 0;
			break;
		case GPRS_UART_CMD_SET_RXBLOCK:
			Gprs_uart_ctl.rx_block = 1;
			break;
		case GPRS_UART_CMD_CLR_RXBLOCK:
			Gprs_uart_ctl.rx_block = 0;
			break;
		case GPRS_UART_CMD_SET_TXWAITTIME:
			int_data = va_arg(arg_ptr, int);
			va_end(arg_ptr); 
			Gprs_uart_ctl.tx_waittime_ms = int_data;
			break;
		case GPRS_UART_CMD_SET_RXWAITTIME:
			int_data = va_arg(arg_ptr, int);
			va_end(arg_ptr); 
			Gprs_uart_ctl.rx_waittime_ms = int_data;
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
int gprs_uart_test(char *buf, int size)
{
	int i = 0;
	for( i = 0; i < size; i ++)
		buf[i] = 0xa5;
	
	gprs_Uart_ioctl( GPRS_UART_CMD_SET_TXWAITTIME, 0);
	gprs_Uart_write( buf, size);
	for( i = 0; i < size; i ++)
		buf[i] = 0;
	gprs_Uart_read( buf, size);
	
	return 0;
	
}




















/*! gprs uart dma Configuration*/
void DMA_GprsUart_Init(void)
{

		DMA_InitTypeDef DMA_InitStructure;	

    /* DMA clock enable */

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); // ??DMA1??
//=DMA_Configuration==============================================================================//	
/*--- UART_Tx_DMA_Channel DMA Config ---*/
    DMA_Cmd( DMA_gprs_usart.dma_tx_base, DISABLE);                           // �ر�DMA
    DMA_DeInit(DMA_gprs_usart.dma_tx_base);                                 // �ָ���ʼֵ
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&GPRS_USART->DR);// �����ַ
//    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)txbuf;        
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;                      // ���ڴ浽����
//    DMA_InitStructure.DMA_BufferSize = sizeof(txbuf)/sizeof(txbuf[0]);                    
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        // �����ַ������
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 // �ڴ��ַ����
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // �������ݿ��1B
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         // �ڴ��ַ���1B
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                           // ���δ���ģʽ
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;                 // ���ȼ�
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                            // �ر��ڴ浽�ڴ�ģʽ
    DMA_Init(DMA_gprs_usart.dma_tx_base, &DMA_InitStructure);               // 

    DMA_ClearFlag( DMA_gprs_usart.dma_tx_flag );                                 // �����־

		DMA_Cmd(DMA_gprs_usart.dma_tx_base, DISABLE); 												// �ر�DMA

    DMA_ITConfig(DMA_gprs_usart.dma_tx_base, DMA_IT_TC, ENABLE);            // ��������DMA�ж�

   

/*--- UART_Rx_DMA_Channel DMA Config ---*/

 

    DMA_Cmd(DMA_gprs_usart.dma_rx_base, DISABLE);                           
    DMA_DeInit(DMA_gprs_usart.dma_rx_base);                                 
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&GPRS_USART->DR);
//    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)rxbuf;         
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                     
//    DMA_InitStructure.DMA_BufferSize = sizeof(rxbuf)/sizeof(rxbuf[0]);;                     
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; 
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                           
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;                 
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                            
    DMA_Init(DMA_gprs_usart.dma_rx_base, &DMA_InitStructure);               
    DMA_ClearFlag( DMA_gprs_usart.dma_rx_flag);                                
    DMA_Cmd(DMA_gprs_usart.dma_rx_base, DISABLE);                            

   

}





void DMA1_Channel2_IRQHandler(void)
{

    if(DMA_GetITStatus(DMA1_FLAG_TC1))
    {

        DMA_ClearFlag(DMA_gprs_usart.dma_tx_flag);         // �����־
				DMA_Cmd( DMA_gprs_usart.dma_tx_base, DISABLE);   // �ر�DMAͨ��
				osSignalSet( Gprs_uart_ctl.tx_tid, SIGNAL_GPRS_UART_TXFINISH);
    }
}

void USART3_IRQHandler(void)
{

	if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)  // �����ж�
	{

		
		DMA_Cmd(DMA_gprs_usart.dma_rx_base, DISABLE);       // �ر�DMA
		DMA_ClearFlag( DMA_gprs_usart.dma_rx_flag );           // ���DMA��־
		Gprs_uart_ctl.recv_size = Gprs_uart_ctl.read_size - DMA_GetCurrDataCounter(DMA_gprs_usart.dma_rx_base); //��ý��յ����ֽ�
		DMA_gprs_usart.dma_rx_base->CNDTR = Gprs_uart_ctl.read_size;
		DMA_Cmd( DMA_gprs_usart.dma_rx_base, ENABLE);
		
		USART_ReceiveData( USART3 ); // Clear IDLE interrupt flag bit
		osSignalSet( Gprs_uart_ctl.rx_tid, SIGNAL_GPRS_UART_RECVFRAM);
	}

}



