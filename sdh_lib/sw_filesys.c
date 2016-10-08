/*************************************************
Copyright (C), 
File name: filesys.c
Author:		sundh
Version:	v1.0
Date: 		14-03-04
Description: 
��w25q��ʵ��һ�����׵��ļ�ϵͳ��
�ڷ����ļ�ϵͳ��ʱ��ʹ���˻��������������flash���ʳ��־���
Ϊÿ�������ṩһ���ļ������������������������ͬһ���ļ�ʱ�������ҡ�
��֧��Ŀ¼�ṹ�����д����flash�е��ļ�����ƽ���ġ�
����0�����ļ���Ϣ����Ϊ��������
����1,2����ÿ��ҳ��ʹ�������ÿһ��bit����һ��ҳ������������ҳ�Ѿ���ʹ�ã����Ӧ��bit��0.
������������ʼλ�ô�ŵ�һ�����ļ���Ϣ
flash�����512kB���ڴ�Ҫ���ڴ洢ϵͳ���񣬲��������ļ��Ĵ洢��
����һ�������Ĵ�С�������ζ�flash�ķ����������ϴβ���ͬ��ʱ�򣬲�ȥ�ѻ���ˢ�µ�flash��ȥ.

����0���ļ���Ϣ��3������ɣ�
1������ͷ��Ϣ���Ѿ��������ļ��������汾��
2���ļ���Ϣ�洢��
3���ļ��洢����洢��


Others: 
Function List: 
1. fs_write
History: 
1. Date:
Author:
Modification:
16-03-16
�ڿռ䲻��ʱ���·���ռ�󷵻�һ��INCREASE_PAGE������ʾ������
2. ...
*************************************************/
#include "list.h"
#include "sw_filesys.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "stdbool.h"
#include "sdhError.h"


static List L_File_opened;
uint8_t	Flash_buf[SECTOR_SIZE];
uint8_t	*Ptr_Flash_buf;		//���ڶ�ȡ����1��2���ڴ�ʹ�����


static	uint8_t		Buf_chg_flag = 0;				//�������ݱ��޸ĵı�־,���汻д��flashʱ��־����
static uint16_t	Src_sector = INVALID_SECTOR;		//�����е����ݵ�����
static char Flash_err_flag = 0;

static bool check_bit(uint8_t *data, int bit);
static void clear_bit(uint8_t *data, int bit);
static void set_bit(uint8_t *data, int bit);
static int get_available_page( uint8_t *data, int start, int end, int len);
static int page_malloc( area_t *area, int len);
static int page_free( area_t *area, int area_num);
static int read_flash( uint16_t sector);
static int flush_flash( uint16_t sector);

static int mach_file(const void *key, const void *data)
{
	char *name = ( char *)key;
	file_Descriptor_t	*fd = (file_Descriptor_t *)data;
	return strcmp( name, fd->name);
	
	
	
}

int filesys_init(void)
{
	
	if( STORAGE_INIT != ERR_OK)
	{
		
		Flash_err_flag = 1;
		return ERR_FLASH_UNAVAILABLE;
		
	}
	else
		Flash_err_flag = 0;
	SYS_ARCH_INIT;
	
	list_init( &L_File_opened, free, mach_file);
	return ERR_OK;
	
}

int filesys_close(void)
{
	
	STORAGE_CLOSE;
	
	return ERR_OK;
	
}

//���ļ���¼����0���ҵ�ָ�����ֵ��ļ���¼��Ϣ
int fs_open(char *name, file_Descriptor_t **fd)
{
	int ret = 0;
	file_info_t	*file_in_storage;
	file_Descriptor_t *pfd;
	storage_area_t	*src_area;
	area_t					*dest_area;	
	sup_sector_head_t	*sup_head;
	ListElmt			*ele;
	short 				i,j, k;
	short step = 0;
	if( Flash_err_flag )
		return (ERR_FLASH_UNAVAILABLE);
	while(1)
	{
		switch( step)
		{
			case 0:
				SYS_ARCH_PROTECT;
				ele = list_get_elmt( &L_File_opened,name);		//�ȴ��Ѿ��򿪵��ļ��в����Ƿ��Ѿ�����������򿪹�
				if(  ele!= NULL)
				{
					pfd = list_data(ele);
					pfd->reference_count ++;
					*fd = pfd;
					pfd->rd_pstn[SYS_GETTID] = 0;
					pfd->wr_pstn[SYS_GETTID] = 0;
					SYS_ARCH_UNPROTECT;
					return ERR_OK;
					
				}
				step ++;
				break;
			case 1:
				ret = read_flash(FILE_INFO_SECTOR);
				if( ret == ERR_OK)
					step ++;
				else
					return ERR_DRI_OPTFAIL;
			case 2:
				sup_head = ( sup_sector_head_t *)Flash_buf;
				if( sup_head->ver[0] != 'V')			//��һ���ϵ磬��������flash��д��ͷ��Ϣ
				{
					SYS_ARCH_UNPROTECT;
					return (ERR_FILESYS_ERROR);
					
				}	
				file_in_storage = ( file_info_t *)( Flash_buf + sizeof(sup_sector_head_t));	
				for( i = 0; i < FILE_NUMBER_MAX; i ++)
				{
				
					if( strcmp(file_in_storage->name, name) == 0x00 )	
					{
						//�ҵ����ļ�
						pfd = (file_Descriptor_t *)malloc(sizeof( file_Descriptor_t));
						dest_area = malloc( file_in_storage->area_total * sizeof( area_t));
					
						pfd->area = dest_area;
						//���ļ��Ĵ洢���丳ֵ���ļ�������
						src_area = ( storage_area_t *)( Flash_buf + sizeof(sup_sector_head_t) + FILE_NUMBER_MAX * sizeof(file_info_t));
						k = 0;
						for( j = 0; j < file_in_storage->area_total ;)
						{
							if( src_area->file_id == file_in_storage->file_id)		
							{
								
								pfd->area[src_area->seq].start_pg = src_area->area.start_pg;
								pfd->area[src_area->seq].pg_number = src_area->area.pg_number;
								j ++;
							}
							k++;
							if( k > STOREAGE_AREA_NUMBER_MAX)
							{
								if( j == 0)		//�ļ�����
								{
									free( dest_area);
									pfd->reference_count = 1;
									strcpy( pfd->name, name);
									pfd->area = NULL;
									*fd = pfd;
								
									step = 0;
									SYS_ARCH_UNPROTECT;
									return (ERR_FILE_ERROR);
								
								}
							
							}
							src_area ++;						
						}				
						pfd->area_total = j ;			//��ʵ���ҵ�������������ֵ���ļ�������
						file_in_storage->area_total = j;			//����ʵ�ʵ���������
						strcpy( pfd->name, name);
						pfd->reference_count = 1;
						memset( pfd->rd_pstn, 0, sizeof( pfd->rd_pstn));
						memset( pfd->wr_pstn, 0, sizeof( pfd->wr_pstn));
						*fd = pfd;
						step = 0;
						list_ins_next( &L_File_opened, L_File_opened.tail, pfd);
						SYS_ARCH_UNPROTECT;
						return ERR_OK;
					}
					file_in_storage ++;
				}		//for
				*fd = NULL;
				SYS_ARCH_UNPROTECT;
				return (ERR_OPEN_FILE_FAIL);
		}		//switch
		
	}		//while(1)

}

//��ʽ���ļ�ϵͳ��ɾ�����е����ݣ����������⣩
int fs_format(void)
{
	sup_sector_head_t	*sup_head;
	int ret;
	int wr_addr = 0;
	
	//���������ļ�ϵͳ
	while(1)
	{
		ret = w25q_Erase_block(wr_addr);
		if( ret == ERR_OK)
		{
			if( 	wr_addr < W25Q_flash.block_num - PAGE_REMIN_FOR_IMG/BLOCK_HAS_SECTORS/SECTOR_HAS_PAGES )
			{
				wr_addr ++;
				
				
			}
			else
				break;
			
		}
		else
			return ERR_DRI_OPTFAIL;
		
		
	}
	//��ȡ����FILE_INFO_SECTOR
	//��Ϊ�ղ����������е�flash���ݶ���0xff��Ҳ�Ͳ���ȥ��Ķ�ȡ��
	memset( Flash_buf, 0xff, SECTOR_SIZE);		
	//�����ϴζ�ȡ������ΪFILE_INFO_SECTOR
	Src_sector = FILE_INFO_SECTOR;
	
	sup_head = ( sup_sector_head_t *)Flash_buf;
	
	sup_head->file_count = 0;
	memcpy( sup_head->ver, FILESYS_VER, 6);
	Buf_chg_flag = 1;
	
	return ERR_OK;
	
}


//�ļ��Ĵ洢��Ϣ�ṹ�嶼��4�ֽڶ���ģ����Բ��ؿ����ֽڶ�������
//ǰ������:�ڴ����ļ���ʱ��,������ǲ��ܹ����ڿն���,������ɾ��������ʱ��,Ҫȥ�������������е��ڴ�
//�����ǿ�ѡ����
int fs_creator(char *name, file_Descriptor_t **fd, int len)
{	
	int i = 0;
	int ret = 0;
	area_t		*tmp_area;
	sup_sector_head_t	*sup_head;
	file_info_t	*file_in_storage;
	file_info_t	*creator_file;
	storage_area_t	*target_area;
	file_Descriptor_t *p_fd;
	
	//�洢������ȷ�Ͳ�����ֱ�ӷ���
	if( Flash_err_flag )
		return ERR_FLASH_UNAVAILABLE;
	if (len == 0)
		len = 1;
	tmp_area = malloc( sizeof( storage_area_t));
	if( tmp_area == NULL)
	{
		ret = ERR_MEM_UNAVAILABLE;
		goto err1;
	}
	ret = page_malloc( tmp_area, len);
	if( ret < 0) {
		
		ret =  ERR_NO_FLASH_SPACE;
		goto err2;
	}
	
	ret = read_flash(FILE_INFO_SECTOR);
	if( ret != ERR_OK)
	{
		goto err2;
	}
	
	sup_head = (sup_sector_head_t *)Flash_buf;
	creator_file = ( file_info_t *)( Flash_buf+ sizeof( sup_sector_head_t));

	if( sup_head->file_count > FILE_NUMBER_MAX)
	{
		ret =  ERR_FILESYS_OVER_FILENUM;
		goto err2;
	}
	
	//����ļ��Ƿ��Ѿ�����
	file_in_storage = ( file_info_t *)( Flash_buf + sizeof(sup_sector_head_t));
	for( i = 0; i < FILE_NUMBER_MAX; i ++)
	{
		
		if( strcmp(file_in_storage->name, name) == 0x00 )	
		{	
			ret =  ERR_FILESYS_FILE_EXIST;
			goto err2;
		}
		file_in_storage ++;	
	}
	
	//�ҵ���һ��û�б�ʹ�õ��ļ���Ϣ�洢��
	i = 0;
	for( i = 0; i < FILE_NUMBER_MAX; i ++)
	{
		if( creator_file->file_id == 0xff)
			break;
		creator_file ++;
		
	}
	if ( i == 	FILE_NUMBER_MAX)
	{
		ret = ERR_NO_SUPSECTOR_SPACE;
		goto err2;	
	}
	target_area = ( storage_area_t *)( Flash_buf+ sizeof( sup_sector_head_t) + FILE_NUMBER_MAX * sizeof(file_info_t));
		
	for( i = 0; i < STOREAGE_AREA_NUMBER_MAX; i ++)
	{
		if( target_area[i].file_id == 0xff)
		{
			target_area[i].file_id = sup_head->file_count;
			target_area[i].seq = 0;
			target_area[i].area.start_pg = tmp_area->start_pg;
			target_area[i].area.pg_number = tmp_area->pg_number;
			break;
		}
		
		
	}
	if( i == STOREAGE_AREA_NUMBER_MAX)
	{
		ret = ERR_NO_SUPSECTOR_SPACE;
		goto err2;	
	}
			
	strcpy( creator_file->name, name);
	creator_file->file_id = sup_head->file_count;
	creator_file->area_total = 1;
	sup_head->file_count ++;
		
	Buf_chg_flag = 1;
		
	p_fd = malloc( sizeof( file_Descriptor_t));
	if( p_fd == NULL) {
		ret = ERR_MEM_UNAVAILABLE;
		goto err2;	
	}
		
	strcpy( p_fd->name, name);
	memset( p_fd->rd_pstn, 0 , sizeof( p_fd->rd_pstn));
	memset( p_fd->wr_pstn, 0 , sizeof( p_fd->wr_pstn));
	p_fd->wr_size = 0;
	p_fd->reference_count = 1;
	p_fd->area = malloc( sizeof( area_t));
		
	//װ�ش洢���ĵ�ַ���䵽�ļ��������еĴ洢����������ȥ
	p_fd->area->start_pg = tmp_area->start_pg;
	p_fd->area->pg_number = tmp_area->pg_number;
	p_fd->area_total = 1;
	*fd = p_fd;			
	list_ins_next( &L_File_opened, L_File_opened.tail, p_fd);
	free( tmp_area);
	return ERR_OK;
	

err2:
	page_free( tmp_area, 1);
err1:
	free( tmp_area);
	return ret;

}

//��λ��ǰλ���ڴ洢�������ʼҳ,����������
static area_t *locate_page( file_Descriptor_t *fd, uint32_t pstn, uint16_t *pg)
{
	short i = 0;
	uint32_t offset = pstn;
	int	lct_page = 0;

	
	
	for( i = 0; i < fd->area_total; i ++)
	{
		//λ��λ�ڱ������ڲ�
		if( fd->area[i].pg_number *PAGE_SIZE > offset)
		{
			
			lct_page = fd->area[i].start_pg + offset/PAGE_SIZE ;
			*pg = lct_page;
			return &fd->area[i];
		}
		//��֪�����䷶Χ�ڣ�ȥ��һ���������
		offset -= fd->area[i].pg_number * PAGE_SIZE;
		
		
	}
	

	
	return NULL;		//�޷���λ������������ֵĻ���Ӧ���·���flash�ռ�
	
	
}

//flash����ʱҪ�ܹ����ļ��·���flashҳ
//
int fs_write( file_Descriptor_t *fd, uint8_t *data, int len)
{
	
	
	int 			ret;
	int 				i, limit;
	int  				myid = SYS_GETTID;
	area_t		*wr_area;
	uint16_t	wr_page = 0, wr_sector = 0;

	while( 1)
	{
		
		wr_area = locate_page( fd, fd->wr_pstn[myid], &wr_page);
		if( wr_area == NULL)
		{
			return (ERR_FILE_FULL);
		}
		wr_sector = 	wr_page/SECTOR_HAS_PAGES;
		
		if( wr_sector < SUP_SECTOR_NUM)
		{
			return (ERR_FILE_ERROR);
			
		}
		ret = read_flash(wr_sector);
		if( ret != ERR_OK)
			return ret;
		
	
		i = ( wr_page % SECTOR_HAS_PAGES)*PAGE_SIZE + fd->wr_pstn[myid] % PAGE_SIZE;
		if( wr_area->pg_number + wr_area->start_pg > ( wr_sector + 1 ) *  SECTOR_HAS_PAGES )				//�ļ��Ľ���λ�ó������������������Ĵ�СΪ����
				limit  = SECTOR_SIZE;
		else		//�ļ��Ľ���λ���ڱ������ڲ������ļ�����λ���ڱ������е����λ��Ϊ����
				limit = ( wr_area->start_pg +  wr_area->pg_number - wr_sector *  SECTOR_HAS_PAGES) * PAGE_SIZE;
		while( len)
		{
			if( i >= SECTOR_SIZE || i > limit)		 //�����˵�ǰ�������������䷶Χ
			{
				break;
			}
			

			if( Flash_buf[i] != *data)
			{
				Flash_buf[i] = *data;
				Buf_chg_flag = 1;	
			}
			i ++;
			fd->wr_pstn[myid] ++;
			len --;
			data++;
			
		}
		fd->wr_size =  fd->wr_pstn[myid];
		if( len == 0)			
		{
			break;
		}
	}
	
	return ERR_OK;
	
}

int fs_read( file_Descriptor_t *fd, uint8_t *data, int len)
{
	
	int 			ret;
	int 				i, limit;
	int  				myid = SYS_GETTID;
	area_t			*rd_area;
	uint16_t	rd_page = 0, rd_sector = 0;

	
	while(1)
	{
		rd_area = locate_page( fd, fd->rd_pstn[myid], &rd_page);
		if( rd_area == NULL)
		{
			return (ERR_FILE_EMPTY);
		}
		
		rd_sector = 	rd_page/SECTOR_HAS_PAGES;
		ret = read_flash(rd_sector);
		if( ret != ERR_OK)
			return ret;
		
		//��д��ʱ��Ҫ�����ļ��Ľ�β���ڱ����������
		i = ( rd_page % SECTOR_HAS_PAGES)*PAGE_SIZE + fd->rd_pstn[myid] % PAGE_SIZE;
			
		if( rd_area->start_pg +  rd_area->pg_number > ( rd_sector + 1) *  SECTOR_HAS_PAGES)	//��βλ�ڱ�������
			limit = SECTOR_SIZE;
		else	//��βλ�ڱ�������
			limit = ( rd_area->start_pg +  rd_area->pg_number - rd_sector *  SECTOR_HAS_PAGES) * PAGE_SIZE;
			
		while( len)
		{
			if( i >= SECTOR_SIZE || i > limit)		 //�����˵�ǰ�������������䷶Χ
			{
				break;
			}
			*data++ = Flash_buf[i];
			i ++;
			fd->rd_pstn[myid] ++;
			len --;
				
		}
			
		if( len == 0)			//��������û����
		{
			
			break;
			
		}
			
	}
	
	return ERR_OK;
		
	
}

int fs_lseek( file_Descriptor_t *fd, int offset, int whence)
{
	char myid = SYS_GETTID;

	switch( whence)
	{
		case WR_SEEK_SET:
			fd->wr_pstn[ myid] = offset;
			break;
		case WR_SEEK_CUR:
			fd->wr_pstn[ myid] += offset;
			break;
		
		
		case WR_SEEK_END:			//����5��0xff��Ϊ��β

		
			break;
		
		case RD_SEEK_SET:
			fd->rd_pstn[ myid] = offset;
			break;
		case RD_SEEK_CUR:
			fd->rd_pstn[ myid] += offset;
			break;
		
		
		case RD_SEEK_END:
			
		
			break;
		case GET_WR_END:
			return fd->wr_pstn[ myid];
		
		default:
			break;
		
	}
	
	return ERR_OK;
	
	
}

int fs_close( file_Descriptor_t *fd)
{
	char myid = SYS_GETTID;
	void 							*data = NULL;
	ListElmt           *elmt;
	int i, ret;
	file_info_t	*file_in_storage;

	fd->reference_count --;			
	fd->rd_pstn[ myid] = 0;
	fd->wr_pstn[ myid] = 0;
	if( fd->reference_count > 0)
		return ERR_OK;
	ret = read_flash( FILE_INFO_SECTOR);
	if( ret != ERR_OK)
		return ret;
	
	file_in_storage = ( file_info_t *)( Flash_buf + sizeof(sup_sector_head_t));
	for( i = 0; i < FILE_NUMBER_MAX; i ++)
	{
		if( strcmp(file_in_storage->name, fd->name) == 0x00 )
		{
			Buf_chg_flag = 1;
			break;
		}
		file_in_storage ++;	
	}
	elmt = list_get_elmt( &L_File_opened,fd->name);
	list_rem_next( &L_File_opened, elmt, &data);
	free( fd->area);
	
	free(fd);
	return ERR_OK;
}


int fs_delete( file_Descriptor_t *fd)
{
	int ret = 0;
	file_info_t	*file_in_storage;
	storage_area_t	*src_area;
	sup_sector_head_t	*sup_head;
	short i, j, k;

	
	fd->reference_count --;
	if( fd->reference_count > 0)
		return ERR_FILE_OCCUPY;
	ret = read_flash(FILE_INFO_SECTOR);
	if( ret != ERR_OK)
		return ret;
	sup_head = ( sup_sector_head_t *)Flash_buf;
	file_in_storage = ( file_info_t *)( Flash_buf + sizeof(sup_sector_head_t));
	k = 0;
	
	//�洢���Ĺ������ҵ����ļ��Ĺ�������Ȼ��ɾ����
	for( i = 0; i < FILE_NUMBER_MAX; i ++)
	{
		if( strcmp(file_in_storage->name, fd->name) == 0x00 )
		{
			
			
			src_area = ( storage_area_t *)( Flash_buf + sizeof(sup_sector_head_t) + FILE_NUMBER_MAX * sizeof(file_info_t) );
			for( j = 0; j < STOREAGE_AREA_NUMBER_MAX; j ++)
			{
				if( src_area[j].file_id == file_in_storage->file_id)
				{
					memset( &src_area[j], 0xff, sizeof(storage_area_t));
					k ++;
					
				}
				if( k == file_in_storage->area_total)
					break;
				
				
			}
			
			Buf_chg_flag = 1;
			memset( file_in_storage, 0xff, sizeof(file_info_t));
			break;
		}
		
		file_in_storage ++;
		
		
	}
	sup_head->file_count --;
	if( fd->area != NULL)
	{
		page_free( fd->area, fd->area_total);
		free( fd->area);
	}
	
	free(fd);
	fd = NULL;
			
	return ERR_OK;
	
}

int fs_flush( void)
{
	int ret;
	//�洢������ȷ�Ͳ�����ֱ�ӷ���
	if( Flash_err_flag )
		return ERR_FLASH_UNAVAILABLE;

		
	if( Buf_chg_flag == 0)
		return ERR_OK;
	ret = flush_flash( Src_sector);
	if( ret == ERR_OK)
	{
		Buf_chg_flag = 0;
		
		return ERR_OK;
		
	}
	return ret;
		
	
	
}





static int read_flash( uint16_t sector)
{
	int ret = 0;
	if( Src_sector == sector )		
	{
		//���ζ�ȡ���������ϴζ�ȡ��һ��ʱ�򣬿��Բ�����ȥ��ȡ
		//��������޸ı�־����˵�������е����ݱ�flash�е����ݸ���
		
		return ERR_OK;
	}
	
	//��ȡ����һ������������ݻ����޸ı�־�������Ƿ񽫻�������д��flash

	if( Buf_chg_flag)
	{
		ret = flush_flash( Src_sector);
		if( ret != ERR_OK)
		{
			return ret;
			
		}
//		Src_sector = INVALID_SECTOR;
	}
	SYS_ARCH_PROTECT;
	ret = flash_read_sector(Flash_buf,sector);
	SYS_ARCH_UNPROTECT;
	if( ret == ERR_OK)
	{
		Buf_chg_flag = 0;
		Src_sector = sector;
		return ERR_OK;
	}
		
	return ret;

}

static int flush_flash( uint16_t sector)
{
	int ret = 0;
	
	SYS_ARCH_PROTECT;
	ret = flash_erase_sector(sector);
	
	if( ret != ERR_OK )
	{
		SYS_ARCH_UNPROTECT;
		return ret;
	}

	ret =  flash_write_sector( Flash_buf, sector);
	SYS_ARCH_UNPROTECT;
	return ret;
}

static int page_malloc( area_t *area, int len)
{
	static uint32_t i = 0;
	uint16_t 	j = 0, page_num,k;
	uint16_t	mem_manger_sector = 0;
	short		wr_addr = 0;
	int ret = 0;
	
	
	

	Ptr_Flash_buf = Flash_buf;
	i = 0;
	mem_manger_sector = FLASH_USEE_SECTOR1;
	while(1)
	{
		
		ret = read_flash(mem_manger_sector);
		if( ret != ERR_OK)
			return ret;
		//�Ȳ鿴flash�Ƿ���пɱ�������ڴ�ҳ
		//����0 1 2����ά���ļ�ϵͳ��������Ϊ��ͨ�ڴ�ʹ�ã�����Ҫ����
		i = get_available_page( Ptr_Flash_buf, PAGE_AVAILBALE_OFFSET, ( SECTOR_SIZE * 8 - 1), len);
		if( i != FLASH_NULL_FLAG)	
		{
			if( mem_manger_sector == FLASH_USEE_SECTOR2)
				i += SECTOR_SIZE * 8;
			break;
		}
		//�ڵڶ����ڴ��������Ҳ�Ҳ��������ڴ�ҳ��˵���ڴ汻�þ�
		if( mem_manger_sector == FLASH_USEE_SECTOR2)
			return ERR_NO_FLASH_SPACE;
		
		mem_manger_sector = FLASH_USEE_SECTOR2;
		
	}
			
		
	//������Ϊ�˰�0д��1���ڱ���Ѿ����������ҳʱ��Ҫ�Ѷ�Ӧ��bit��1->0
	//��˲������0->1�ĳ�����Ϊ�˽�ʡ�����Ͳ����в��������ˡ�
	//ֻ��Ҫ�����������ҳ�ı�־�������
	
	area->start_pg = i;
	area->pg_number = len/PAGE_SIZE + 1;
	i %= SECTOR_SIZE * 8;  //��������2��ƫ��
	for( j = 0; j <= len/PAGE_SIZE; j ++)
		clear_bit( Ptr_Flash_buf, i + j);
	
	//����i�����ʼλ����һ��ҳ�е���ʼλ��
	j = i/8%PAGE_SIZE;
	k = len/PAGE_SIZE/8;  //����Ҫ����ĳ�����ռ�õ��ֽڳ���
	
	page_num = (i + k) / PAGE_SIZE + 1;
	while(1)
	{
		ret = flash_write(Ptr_Flash_buf + \
		( i/8/PAGE_SIZE + wr_addr)* PAGE_SIZE,  mem_manger_sector*SECTOR_SIZE + ( i/8/PAGE_SIZE + wr_addr) * PAGE_SIZE, PAGE_SIZE);
		if( ret == ERR_OK )
		{
			wr_addr ++;
			if( wr_addr >= page_num) 
			{
				return wr_addr;
				
			}
			
			
			
		}
		else
			return ret;
	}
}


//������ͬһ��������bitһ����һ���������������ٶ�д�����Ĵ���
static int page_free( area_t *area, int area_num)
{
	char  boundary = 0;
	uint16_t i = 0;
	uint16_t j = 0, k = 0;
	uint16_t	mem_manger_sector = 0;

	int ret = 0;
	area_t	tmp_area ;
	
		

	Ptr_Flash_buf = Flash_buf;


	//����洢���䣬������������������
	if( area_num > 1)
	{
		for( i = 0; i < area_num; i ++)
		{
			if( area[i].start_pg + area[i].pg_number > W25Q_flash.page_num)
				return -1;
			
			
			//��¼flash�ڴ�ҳ�����ҳ����ҳ��ҳ1��ҳ2
			//Ϊ�˷������������ҳ1��ҳ2��˳�������ҳ1�ڵ�����ŵ�ǰ�棬ҳ2������ŵ�����
			//ע��û�п��������Խҳ1��ҳ2�����
			if( area[i].start_pg > SECTOR_PAGE_BIT)			
			{
				tmp_area.pg_number =  area[i].pg_number;
				tmp_area.start_pg =  area[i].start_pg;
				for( j = i; j < area_num; j ++)
				{
					if( area[j].start_pg < SECTOR_PAGE_BIT)
					{
						area[i].pg_number = area[j].pg_number;
						area[i].start_pg = area[j].start_pg;
						area[j].pg_number = tmp_area.pg_number;
						area[j].start_pg = tmp_area.start_pg;
						boundary = i;
						break;
					}
					
				}
			}
		}
		
		if( boundary == 0)			//ȫ���Ĵ洢���䶼��������2��
			mem_manger_sector = FLASH_USEE_SECTOR2;
		else
			mem_manger_sector = FLASH_USEE_SECTOR1;
		
	}
	else 
	{
		if( area->start_pg + area->pg_number > W25Q_flash.page_num)
				return -1;
		
		if( area->start_pg > SECTOR_PAGE_BIT)
			mem_manger_sector = FLASH_USEE_SECTOR2;
		else
			mem_manger_sector = FLASH_USEE_SECTOR1;
	}
			
	while(1)
	{
		ret = read_flash(mem_manger_sector);
		if( ret != ERR_OK)
			return ret;	
		
		if( mem_manger_sector == FLASH_USEE_SECTOR1)
		{
			for( k = 0; k <= boundary; k ++)
			{
				i = area[k].start_pg;
				for( j = 0; j < area[k].pg_number; j ++)
					set_bit( Ptr_Flash_buf, i + j);
				
				
			}
			
		}
		else 
		{
			for( k = boundary  + 1; k < area_num; k ++)
			{
				i = area[k].start_pg % SECTOR_PAGE_BIT;
				for( j = 0; j < area[k].pg_number; j ++)
					set_bit( Ptr_Flash_buf, i + j);
				
				
			}
			
		}
		Buf_chg_flag = 1;
		
		if( mem_manger_sector == FLASH_USEE_SECTOR1 && boundary > 0 && boundary < ( area_num - 1))
		{
			mem_manger_sector = FLASH_USEE_SECTOR2;
		}
		else
		{
			
			return ERR_OK;
		}
	}
			
			
			
			
			
	
	
}



static bool check_bit(uint8_t *data, int bit)
{
	int i, j ;
	i = bit/8;
	j = bit % 8;
	return ( data[i] & ( 1 << j));
	
	
}
static void clear_bit(uint8_t *data, int bit)
{
	int i, j ;
	i = bit/8;
	j = bit % 8;
	data[i] &= ~( 1 << j);
	
	
	
	
}

static void set_bit(uint8_t *data, int bit)
{
	int i, j ;
	i = bit/8;
	j = bit % 8;
	data[i] |=  1 << j;
	
	
	
	
}

static int get_available_page( uint8_t *data, int start, int end, int len)
{
	short i, j ;
	int pages = len / PAGE_SIZE + 1;
	
	for( i = start; i < end; i++)
	{
		
		if( check_bit( data, i)) 		
		{
			for( j = 0; j < pages; j ++)	//��鳤���Ƿ���
			{
				if( !check_bit( data, i+j)) 
					break;
				
			}
			if( j == pages)
				return i;
			
		}
		
	}
	
	return FLASH_NULL_FLAG;
	
	
}














