#ifndef __FILE_SYS_H__
#define __FILE_SYS_H__
#include "hw_w25q.h"
#include "osObjects.h"                      // RTOS object definitions
#include "stdint.h"
#include "list.h"
#define FILESYS_VER	"V2.1"

///����ӿ� ----------------------------------------------------------------
#define	TASK_NUM		8			///�ļ�ϵͳʹ�õ�ʱ��Ϊÿ������ά��һ���������ݽṹ
#define flash_erase_sector 			w25q_Erase_Sector
#define	flash_write_sector			w25q_Write_Sector_Data
#define	flash_read_sector			w25q_Read_Sector_Data
#define	flash_write					w25q_Write
#define STORAGE_INIT()						w25q_init() 
#define STORAGE_CLOSE()						w25q_close()	
#define STORAGE_INFO(info)					w25q_info(info)
#define	SYS_ARCH_INIT()						
#define SYS_ARCH_PROTECT()
#define SYS_ARCH_UNPROTECT()
#define SYS_GETTID()								0			//������ʱ��������ͬ�Ľ���
#define RESE_STOREAGE_SIZE_KB						0			//�����Ĵ洢�ռ�
#define FILE_NUMBER_MAX								20				//�����Դ������ļ�����,һ������£�һ���ļ���Ҫ24B���洢������Ϣ

typedef struct {
	int32_t		page_size;						///һҳ�ĳ���
	int32_t		total_pagenum;					///�����洢����ҳ����
	
	uint16_t		sector_pagenum;
	uint16_t		block_pagenum;
	
//privately
	uint32_t	sector_size;
	uint32_t	block_size;	
	
	uint16_t	sector_number;	
	uint16_t	block_number;	
}storageInfo_t;
//----------------------------------------------------------------------------------

typedef struct {
	uint16_t		fileinfo_sector_begin;
	uint16_t		fileinfo_sector_end;			///�ļ���Ϣ����
	uint16_t		pguseinfo_sector_begin;
	uint16_t		pguseinfo_sector_end;			///ҳ��ʹ����Ϣ����
	uint16_t		data_sector_begin;
	uint16_t		data_sector_end;			//��������
}fs_area;

#define FLASH_NULL_FLAG  0xffffffff

//#define STOREAGE_AREA_NUMBER_MAX			60				//һ���ļ��Ĵ洢���������
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
#define		ERR_STORAGE_FAIL		-13			//�洢������ʧ��
#define 	ERR_NON_EXISTENT 	-14	


typedef struct {
	short				file_count;				//ָ���һ��δʹ�õĵ�ַ
	char				ver[6];
	
	
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
int fs_get_error(void);
int filesys_close(void);


sdhFile * fs_open(char *name);
sdhFile * fs_creator(char *name, int len);
int fs_write( sdhFile *fd, uint8_t *data, int len);
int fs_read( sdhFile *fd, uint8_t *data, int len);
int fs_lseek( sdhFile *fd, int offset, int whence);
int fs_delete( sdhFile *fd);
int fs_du( sdhFile *fd);
int fs_close( sdhFile *fd);



int fs_flush( void);
int fs_format(void);
int fs_test(void);
#endif



