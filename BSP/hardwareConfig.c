/**
* @file 		hardwareConfig.c
* @brief		ϵͳ��Ӳ������
* @details		1. gprs������������
* @author		sundh
* @date		16-09-15
* @version	A001
* @par Copyright (c): 
* 		XXX��˾
* @par History:         
*	version: author, date, desc
*	A001:sundh,16-09-15,gprs�Ŀ�����������
*/
#include "hardwareConfig.h"


gpio_pins	Gprs_powerkey =  {
	GPIOB,
	GPIO_Pin_0
};

gpio_pins	W25Q_csPin =  {
	GPIOA,
	GPIO_Pin_4
};

USART_InitTypeDef USART_InitStructure = {
		9600,
		USART_WordLength_8b,
		USART_StopBits_1,
		USART_Parity_No,
		USART_Mode_Rx | USART_Mode_Tx,
		USART_HardwareFlowControl_None,
};
USART_InitTypeDef Conf_GprsUsart = {
		115200,
		USART_WordLength_8b,
		USART_StopBits_1,
		USART_Parity_No,
		USART_Mode_Rx | USART_Mode_Tx,
		USART_HardwareFlowControl_None,
		
	
};

USART_InitTypeDef Conf_S485Usart = {
		1200,
		USART_WordLength_8b,
		USART_StopBits_1,
		USART_Parity_No,
		USART_Mode_Rx | USART_Mode_Tx,
		USART_HardwareFlowControl_None,
		
	
};

/** gprs uart DMAͨ������
 *
 */
Dma_source DMA_gprs_usart = {
	DMA1_Channel2,
	DMA1_FLAG_GL2,
	DMA1_Channel2_IRQn,
	
	DMA1_Channel3,
	DMA1_FLAG_GL3,
	DMA1_Channel3_IRQn,
	
};


/** 485 uart DMAͨ������
 *
 */
Dma_source DMA_s485_usart = {
	DMA1_Channel7,
	DMA1_FLAG_GL7,
	DMA1_Channel7_IRQn,
	
	DMA1_Channel6,
	DMA1_FLAG_GL6,
	DMA1_Channel6_IRQn,
	
};

SPI_instance W25Q_Spi = {
	W25Q_SPI,
	((void *)0),
	SPI1_IRQn,
	
	
};







