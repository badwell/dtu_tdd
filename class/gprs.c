/**
* @file 		gprs.c
* @brief		ʵ��gprs�ĸ��ֹ���.
* @details		1. ����
* @author		sundh
* @date		16-09-15
* @version	A001
* @par Copyright (c): 
* 		XXX��˾
* @par History:         
*	version: author, date, desc
*	A001:sundh,16-09-15��������ʵ�ֿ�������
*/

#include "gprs.h"
#include "stm32f10x_gpio.h"
#include "hardwareConfig.h"
#include "cmsis_os.h"  
#include "debug.h"

void startup(gprs_t *self)
{
	
	GPIO_ResetBits(Gprs_powerkey.Port, Gprs_powerkey.pin);
	osDelay(100);
    GPIO_SetBits(Gprs_powerkey.Port, Gprs_powerkey.pin);
	osDelay(1100);
    GPIO_ResetBits(Gprs_powerkey.Port, Gprs_powerkey.pin);
	
}

void shutdown(gprs_t *self)
{
	
	GPIO_ResetBits(Gprs_powerkey.Port, Gprs_powerkey.pin);
	osDelay(100);
    GPIO_SetBits(Gprs_powerkey.Port, Gprs_powerkey.pin);
	osDelay(1500);
    GPIO_ResetBits(Gprs_powerkey.Port, Gprs_powerkey.pin);
	
}












CTOR(gprs_t)
FUNCTION_SETTING(startup, startup);
FUNCTION_SETTING(shutdown, shutdown);

END_CTOR
