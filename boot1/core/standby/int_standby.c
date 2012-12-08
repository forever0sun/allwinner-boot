/*
**********************************************************************************************************************
*											        eGon
*						                     the Embedded System
*									       script parser sub-system
*
*						  Copyright(C), 2006-2010, SoftWinners Microelectronic Co., Ltd.
*                                           All Rights Reserved
*
* File    : standby.c
*
* By      : Jerry
*
* Version : V2.00
*
* Date	  :
*
* Descript:
**********************************************************************************************************************
*/
#include "include.h"
#include "arm_a8.h"
/*
************************************************************************************************************
*
*                                             eGon2_int_enter_standby
*
*    函数名称：
*
*    参数列表：
*
*
*
*    返回值  ：
*
*    说明    ：	进入standby
*				1) 清除DMA PENDING，保存DMA enable
*				2) 关闭DMA AHB
*
************************************************************************************************************
*/
int standby_int_init(void)
{
	asm("mrs r0, cpsr");
	asm("orr r0, r0, #(0x40|0x80)");
	asm("msr cpsr_c, r0");

    return 0;
}
/*
************************************************************************************************************
*
*                                             eGon2_int_exit_standby
*
*    函数名称：
*
*    参数列表：
*
*
*
*    返回值  ：
*
*    说明    ：	退出standby
*				1) 打开DMA AHB
*				2) 清除DMA PENDING，恢复DMA enable
*
*
************************************************************************************************************
*/
int standby_int_exit(void)
{
	asm("mrs r0, cpsr");
	asm("bic r0, r0, #(0x40|0x80)");
	asm("msr cpsr_c, r0");

    return 0;
}


int standby_int_query(void)
{
	return 0;
}

