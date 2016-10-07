#ifndef __FILE_SYS_H__
#define __FILE_SYS_H__
#include "system_data.h"
//#include "lw_oopc.h"
#include "app.h"
#include "list.h"

#define FLASH_NULL_FLAG  0xffffffff
#define FILE_NUMBER_MAX								20				//�����Դ������ļ�����
#define STOREAGE_AREA_NUMBER_MAX			60				//
#define SUP_SECTOR_NUM					3
#define FILE_INFO_SECTOR				0
#define FLASH_USEE_SECTOR1			1
#define FLASH_USEE_SECTOR2			2
#define PAGE_AVAILBALE_OFFSET		( SUP_SECTOR_NUM * SECTOR_SIZE / PAGE_SIZE)
#define PAGE_REMIN_FOR_IMG			1024				//256k���ڴ洢��ִ�о����ļ�		
#define SECTOR_PAGE_BIT					32768			//8 * 4096

#define	INVALID_SECTOR					0xffff


#define flash_erase_sector 			w25q_Erase_Sector
#define	flash_write_sector			w25q_Write_Sector_Data
#define	flash_read_sector				w25q_Read_Sector_Data

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

typedef enum {
	ERROR_FILESYS_BEGIN = ERROR_BEGIN(MODULE_FILESYS),
	ERR_FLASH_UNAVAILABLE,
	ERR_FILE_UNAVAILABLE,
	ERR_FILE_EMPTY,
	ERR_FILE_FULL,
	ERR_FILESYS_BUSY,
	ERR_FILESYS_ERROR,
	ERR_FILE_ERROR,
	ERR_FILE_OCCUPY,
	ERR_OPEN_FILE_FAIL,
	ERR_CREATE_FILE_FAIL,
	ERR_NO_SUPSECTOR_SPACE,
	ERR_NO_FLASH_SPACE,
	
	INCREASE_PAGE,
	
}filesys_err_t;

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
//	uint32_t										wr_bytes;							//��ǰ�ļ���д����ֽ���
}file_info_t;


//�ļ�ϵͳ�в����ļ�ʱʹ�õ�������



typedef struct {
	char name[16];
	
	uint32_t		rd_pstn[Prio_task_end];								//�ļ��Ķ�дλ�ã����ֽ�Ϊ��λ
	uint32_t		wr_pstn[Prio_task_end];	
	
	uint32_t		wr_size;													//���浱ǰ�ļ��Ѿ���д��������
	
	uint16_t		reference_count;	
	uint16_t		area_total;		
	
	area_t			*area;									//�洢����
	
}file_Descriptor_t;








int filesys_init(void);
int filesys_close(void);
int filesys_dev_check(void);
error_t fs_open(char *name, file_Descriptor_t **fd);
error_t fs_creator(char *name, file_Descriptor_t **fd, int len);
error_t fs_expansion(file_Descriptor_t *fd, int len);			//�����ļ�������
error_t fs_write( file_Descriptor_t *fd, uint8_t *data, int len);
error_t fs_read( file_Descriptor_t *fd, uint8_t *data, int len);
error_t fs_lseek( file_Descriptor_t *fd, int offset, int whence);
error_t fs_delete( file_Descriptor_t *fd);
error_t fs_close( file_Descriptor_t *fd);
error_t fs_flush( void);
error_t fs_format(void);
error_t wait_fs(void);
#endif
 