#ifndef _CIRCULARBUFFER_H__
#define _CIRCULARBUFFER_H__
#include "stdint.h"

typedef void* tElement;	
typedef struct {
	tElement	*buf;
	uint32_t	size;		//������2����
	uint16_t	read;		//��ǰ�Ķ�λ��
	uint16_t	write;		//��ǰ��дλ��
	
}sCircularBuffer;

#define CPU_IRQ_OFF	
#define CPU_IRQ_ON


uint16_t	CBLengthData( sCircularBuffer *cb);
int	CBWrite( sCircularBuffer *cb, tElement data);
int	CBRead( sCircularBuffer *cb, tElement *data);
#endif
