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
#include "standby.h"


static int eGon2_enter_standby(void);
static int eGon2_exit_standby(void);
static int eGon2_standby_detect(void);
static int eGon2_mod_enter_standby(void);
static int eGon2_mod_exit_standby(void);
static int eGon2_early_standby_mode(void);

//static int standby_flag = 0;
int eGon2_standby_mode(void)
{
	int status;

	axp_set_vbus_vol_limit();
	axp_set_vbus_cur_limit();
	axp_set_suspend_chgcur();

	eGon2_store_sp();
	eGon2_set_sp();

	status = eGon2_early_standby_mode();

	eGon2_restore_sp();

	return status;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int eGon2_early_standby_mode(void)
{
	__s32 key_status;
	__s32 status;

	//检查是否有按键按下
	key_status = standby_axp_probe_key();
	if(key_status & 0x01)			//长按键的情况下，不管电源是否移除，直接进入系统
	{
		return 3;
	}
	//检查外部电源是否存在
	if(standby_axp_probe_dcin_exist() <= 0)
	{
		return 4;
	}
	if(key_status & 0x02)			//短按电压按键的情况下，显示充电动画
	{
		return 2;
	}
	eGon2_mod_enter_standby();      //控制模块进入standby
	status = -1;
	eGon2_enter_standby();
	//standby_serial_putc('2');
	do
	{
		//standby_serial_putc('3');
		//开始循环检查是否开始唤醒
		asm("wfi");
		//standby_serial_putc('4');
		status = eGon2_standby_detect();
		//standby_serial_putc('5');
	}
	while(status <= 0);
	//standby_serial_putc('6');
	//发现需要唤醒，退出standby
	eGon2_exit_standby();
	//standby_serial_putc('a');
	//退出模块的standby
	eGon2_mod_exit_standby();

	return status;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
static int eGon2_enter_standby(void)
{
	//处理中断
	standby_int_disable();

	//mctl_deep_sleep_test();
	dram_power_save_process();

	//standby_serial_putc('1');

	standby_gic_store();
	//standby_serial_putc('m');
	standby_axp_store_int_status();
	//standby_serial_putc('n');
	standby_clock_to_24M();
	//standby_serial_putc('o');
	standby_clock_plldisable();
	//standby_serial_putc('p');
	standby_axp_output_control(0);

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
static int eGon2_exit_standby(void)
{
	standby_axp_output_control(1);
	standby_clock_pllenable();
	standby_clock_to_pll1();
	//standby_serial_putc('7');
	dram_power_up_process();
	//standby_serial_putc('8');
	standby_axp_restore_int_status();
	standby_gic_restore();
	standby_int_enable();
	//standby_serial_putc('9');

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
static int eGon2_standby_detect(void)
{
	__u8  power_int_status[8];

	//检查中断触发
	standby_axp_int_query(power_int_status);
	if(power_int_status[2] & 0x02)			//电源按键短按
	{
		return 2;
	}
	if(power_int_status[2] & 0x01)			//电源按键长按
	{
		return 3;
	}
	if(power_int_status[0] & 0x22)			//外部电源移除
	{
		if(standby_axp_probe_dcin_exist() <= 0)
		{
			return 4;
		}
	}
//	if((power_int_status[1] & 0x04) && (battery_exist==1))			//充电完成
//	{
//		return 5;
//	}
//	if(power_int_status[0] & 0x08)			//usb火牛接入
//	{
//		return 8;
//	}
//	if(power_int_status[0] & 0x04)			//usb火牛移除
//	{
//		return 9;
//	}

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
static int eGon2_mod_enter_standby(void)
{
	int i;

	for(i=0;i<EMOD_COUNT_MAX;i++)
	{
		eGon2_driver_standby(i, EGON2_MOD_ENTER_STANDBY, 0);
	}

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
static int eGon2_mod_exit_standby(void)
{
	int i;

	for(i=0;i<EMOD_COUNT_MAX;i++)
	{
		eGon2_driver_standby(i, EGON2_MOD_EXIT_STANDBY, 0);
	}

	return 0;
}




