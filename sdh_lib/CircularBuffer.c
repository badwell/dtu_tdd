/**
* @file 		circularbuffer.c
* @brief		ʵ�ֻ��λ�����㷨.
* @details		1. ���λ���ĳ���
* @author		author
* @date		date
* @version	A001
* @par Copyright (c): 
* 		XXX??
* @par History:         
*	version: author, date, desc\n
*/
#include "CircularBuffer.h"
#include "sdhError.h"
/**
 * @brief ���ػ��λ����д洢���ݵĳ���
 *
 * @details ����.
 * 
 * @param[in]	inArgName input argument description.
 * @param[out]	outArgName output argument description. 
 * @retval	OK	??
 * @retval	ERROR	?? 
 */
uint16_t	CBLengthData( sCircularBuffer *cb)
{
	return ( ( cb->write - cb->read) & ( cb->size - 1));
}

/**
 * @brief ���λ���дһ������.
 *
 * @details This is a detail description.
 * 
 * @param[in]	inArgName input argument description.
 * @param[out]	outArgName output argument description. 
 * @retval	OK	??
 * @retval	ERROR	?? 
 */
int	CBWrite( sCircularBuffer *cb, tElement data)
{
	if( CBLengthData( cb) == ( cb->size - 1))	return ERR_MEM_UNAVAILABLE;
	cb->buf[ cb->write] = data;
	CPU_IRQ_OFF;
	cb->write = ( cb->write + 1) & ( cb->size - 1);  //������ԭ�ӵ�
	CPU_IRQ_ON;
	return ERR_OK;
}

/**
 * @brief �ӻ��λ����ȡһ������.
 *
 * @details This is a detail description.
 * 
 * @param[in]	inArgName input argument description.
 * @param[out]	outArgName output argument description. 
 * @retval	OK	??
 * @retval	ERROR	?? 
 */
int	CBRead( sCircularBuffer *cb, tElement *data)
{
	if( CBLengthData( cb) == 0)	return ERR_RES_UNAVAILABLE;
	*data = cb->buf[ cb->read];
	CPU_IRQ_OFF;
	cb->read = ( cb->read + 1) & ( cb->size - 1);  //������ԭ�ӵ�
	CPU_IRQ_ON;
	return ERR_OK;
}
