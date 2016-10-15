#ifndef __FILE_SYS_H__
#define __FILE_SYS_H__
#include "hw_w25q.h"
#include "osObjects.h"                      // RTOS object definitions

#include "list.h"
#define FILESYS_VER	"V1.1"

///��Ҫ����ֲ���޸ĵĲ��� ----------------------------------------------------------------
#define	TASK_NUM		8			///�ļ�ϵͳʹ�õ�ʱ��Ϊÿ������ά��һ���������ݽṹ
#define flash_erase_sector 			w25q_Erase_Sector
#define	flash_write_sector			w25q_Write_Sector_Data
#define	flash_read_sector			w25q_Read_Sector_Data
#define	flash_write					w25q_Write
#define STORAGE_INIT()						w25q_init() 
#define STORAGE_CLOSE()						w25q_close()	
#define	SYS_ARCH_INIT()		
#define SYS_ARCH_PROTECT()
#define SYS_ARCH_UNPROTECT()
#define SYS_GETTID()
#define RESE_STOREAGE_SIZE_KB						0			//�����Ĵ洢�ռ�
#define FILE_NUMBER_MAX								20				//�����Դ������ļ�����

//----------------------------------------------------------------------------------


#define FLASH_NULL_FLAG  0xffffffff

//#define STOREAGE_AREA_NUMBER_MAX			60				//
//#define SUP_SECTOR_NUM					3
//#define FILE_INFO_SECTOR				0
//#define FLASH_USEE_SECTOR1			1
//#define FLASH_USEE_SECTOR2			2
//#define PAGE_AVAILBALE_OFFSET		( SUP_SECTOR_NUM * SECTOR_SIZE / PAGE_SIZE)
//#define PAGE_REMIN_FOR_IMG			1024				//256k���ڴ洢��ִ�о����ļ�		
//#define SECTOR_PAGE_BIT					32768			//8 * 4096

#define	INVALID_SECTOR					0xffff


						
//CMD


typedef enum {
	WR_SEEK_SET = 0,
	WR_SEEK_CUR = 1,
	WR_SEEK_END = 2,
	RD_SEEK_SET = 3,
	RD_SEEK_CUR = 4,
	RD_SEEK_END = 5,
	GET_WR_END = 6,
	GET_RD_END = 7,
}lseek_whence_t;
typedef enum {
	

	
	F_State_idle,
	F_State_Number,
	F_State_Err,
	
}task_flash_state;


#define 	ERR_FLASH_UNAVAILABLE -1
#define 	ERR_FILE_EMPTY -2
#define 	ERR_FILE_FULL -3
#define 	ERR_FILESYS_BUSY -4
#define 	ERR_FILESYS_ERROR -5
#define 	ERR_FILE_ERROR -6
#define 	ERR_FILE_OCCUPY -7
#define 	ERR_OPEN_FILE_FAIL -8
#define 	ERR_CREATE_FILE_FAIL -9
#define 	ERR_NO_SUPSECTOR_SPACE -10
#define 	ERR_NO_FLASH_SPACE -11
#define 	ERR_FILESYS_OVER_FILENUM -12
#define 	ERR_FILESYS_FILE_EXIST -12

	


typedef struct {
	short				file_count;				//ָ���һ��δʹ�õĵ�ַ
	uint8_t				ver[6];
	
	
}sup_sector_head_t;

typedef struct {
	
	uint16_t 										start_pg;		//��ʼҳ��
	uint16_t 										pg_number;				//ҳ������
	
}area_t;
typedef struct {
	uint8_t											file_id;
	uint8_t											seq;
	area_t											area;
	
}storage_area_t;




//�ļ���flash�еĴ洢��Ϣ
typedef struct {
	char 												name[16];
	uint8_t											file_id;					//�ļ�id������ϵ�ļ������Ĵ洢������Ϣ.
	uint8_t											area_total;
}file_info_t;


//�ļ�ϵͳ�в����ļ�ʱʹ�õ�������



typedef struct {
	char name[16];
	
	uint32_t		rd_pstn[TASK_NUM];								//�ļ��Ķ�дλ�ã����ֽ�Ϊ��λ
	uint32_t		wr_pstn[TASK_NUM];	
	
	uint32_t		wr_size;													//���浱ǰ�ļ��Ѿ���д��������
	
	uint16_t		reference_count;	
	uint16_t		area_total;		
	
	area_t			*area;									//�洢����
	
}sdhFile;






int filesys_init(void);
int filesys_close(void);
int fs_open(char *name, sdhFile **fd);
int fs_creator(char *name, sdhFile **fd, int len);
int fs_write( sdhFile *fd, uint8_t *data, int len);
int fs_read( sdhFile *fd, uint8_t *data, int len);
int fs_lseek( sdhFile *fd, int offset, int whence);
int fs_delete( sdhFile *fd);
int fs_close( sdhFile *fd);
int fs_flush( void);
int fs_format(void);
int fs_test(void);
#endif



