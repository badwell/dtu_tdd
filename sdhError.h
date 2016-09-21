#ifndef __MYERROR_H__
#define __MYERROR_H__


#define ERR_OK 									0 					/* No error, everything OK. */
#define ERR_UNKOWN     					-1    			/* ???? ,?????????????   */
#define ERR_BAD_PARAMETER     	-2    			/*       */
#define ERR_ERROR_INDEX    	 		-3    			/*       */
#define ERR_UNINITIALIZED     	-4    			/* 没有初始化完成，一般表示执行操作所需的前期准备没有完成     */
#define ERR_CATASTROPHIC_ERR		-5 					/* ?????			*/
#define ERR_MEM_UNAVAILABLE			-6					/* ??????			*/
#define ERR_RES_UNAVAILABLE			-7					/* ??????			*/
#define ERR_DEV_TIMEOUT					-8					/* ??????			*/
#define ERR_FAIL								-9					/* 操作失败		*/ 

#endif
