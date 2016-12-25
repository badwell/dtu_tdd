#ifndef __GPRS_UART_H__
#define __GPRS_UART_H__
#include "stdint.h"


/**
 * @brief gprs���ڳ�ʼ��
 * 
 * @return ERR_OK �ɹ�
 * @return 
 */
int gprs_uart_init(void);

/**
 * @brief gprs���ڷ��͹���
 * 
 * @param data �������ݵ��ڴ��ַ
 * @param size �������ݵĳ���
 * @return ERR_OK �ɹ�
 * @return 
 */
int gprs_Uart_write(char *data, uint16_t size);

/**
 * @brief gprs���ڽ��ճ���
 * 
 * @param data 
 * @param size 
 * @return ERR_OK �ɹ�
 * @return 
 */
int gprs_Uart_read(char *data, uint16_t size);

/**
 * @brief ���ƴ��ڵ���Ϊ
 * 
 * @param data 
 * @param size 
 * @return ERR_OK �ɹ�
 * @return 
 */
void gprs_Uart_ioctl(int cmd, ...);

/**
 * @brief ����gprs uart���շ�����
 * 
 * @param size �������ݵĳ���
 * @return 0xFF if error, else return 0
 */
int gprs_uart_test(char *buf, int size);

typedef void (*rxirq_cb)(void *rxbuf, void *arg);
void regRxIrq_cb(rxirq_cb cb, void *arg);
#define GPRS_UART_BUF_LEN		1024

#define GPRS_UART_CMD_SET_TXBLOCK	1
#define GPRS_UART_CMD_CLR_TXBLOCK	2
#define GPRS_UART_CMD_SET_RXBLOCK	3
#define GPRS_UART_CMD_CLR_RXBLOCK	4
#define GPRSUART_SET_TXWAITTIME_MS	5
#define GPRSUART_SET_RXWAITTIME_MS	6

#endif
