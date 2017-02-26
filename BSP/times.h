#ifndef _TIMERS_H_
#define _TIMERS_H_
#include "stdint.h"
//#define MAX_COMMA 256
#define MAX_ALARM_TOP		8			//֧�ֵ������������� 4��TCP���Ӻ�1��ģʽת�����干5��
#define ALARM_CHGWORKINGMODE	0			//485��Ĭ��ģʽת��������ģʽ��ʱ��
#define ALARM_GPRSLINK(n)		(1+n)			//GPRS link
typedef struct TIME2_T
{
	uint32_t		time_ms;
    uint32_t    time_s;
   
}TIME2_T;
typedef void (*time_job)(void);
typedef struct  
{
	void 		*next;
	uint16_t 	period_ms;
	uint16_t	count_ms;
	time_job 	my_job;
	
}time_task_manager;

void regist_timejob( uint16_t period_ms, time_job job);
void clean_time2_flags(void);
uint32_t get_time_s(void);
uint32_t get_time_ms(void);
void set_alarmclock_s(int alarm_id, int sec);
int Ringing(int alarm_id);
void time_test(void);
#endif
