/******************************************************************************
 * @file     main.c
 * @version  V1.00
 * $Revision: 13 $
 * $Date: 18/07/18 3:19p $
 * @brief    Show FMC read flash IDs, erase, read, and write functions.
 *
 * @note
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "NuMicro.h"
#include "easyflash.h"
#include "elog.h"
#include "elog_flash.h"
int IsDebugFifoEmpty(void);

void SYS_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable HIRC clock */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);

    /* Waiting for HIRC clock ready */
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);

    /* Switch HCLK clock source to HIRC and HCLK source divide 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));

    /* Enable UART0 clock */
    CLK_EnableModuleClock(UART0_MODULE);

    /* Switch UART0 clock source to HIRC */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));

    /* Update System Core Clock */
    SystemCoreClockUpdate();

    /* Set PB multi-function pins for UART0 RXD=PB.12 and TXD=PB.13 */
    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB12MFP_Msk | SYS_GPB_MFPH_PB13MFP_Msk))
                    |(SYS_GPB_MFPH_PB12MFP_UART0_RXD | SYS_GPB_MFPH_PB13MFP_UART0_TXD);


    /* Lock protected registers */
    SYS_LockReg();
}


static void elog_user_assert_hook(const char* ex, const char* func, size_t line)
{
#ifdef ELOG_ASYNC_OUTPUT_ENABLE
    /* disable async output */
    elog_async_enabled(false);
#endif

    /* disable logger output lock */
    elog_output_lock_enabled(false);

    /* disable flash plugin lock */
    elog_flash_lock_enabled(false);

    /* output logger assert information */
    elog_a("elog", "(%s) has assert failed at %s:%ld.\n", ex, func, line);

    /* write all buffered log to flash */
    elog_flash_flush();

    while(1);
}


int main()
{


    /* Unlock protected registers to operate FMC ISP function */
    SYS_UnlockReg();

    /* Init System, peripheral clock and multi-function I/O */
    SYS_Init();

    /* Init UART0 to 115200-8n1 for print message */
    UART_Open(UART0, 115200);

    /* Checking if target device supports the feature */
    if( (GET_CHIP_SERIES_NUM == CHIP_SERIES_NUM_I) || (GET_CHIP_SERIES_NUM == CHIP_SERIES_NUM_G) )
    {
        /* Checking if flash size matches with target device */
        if(FMC_FLASH_PAGE_SIZE != 2048)
        {
            /* FMC_FLASH_PAGE_SIZE is different from target device's */
            printf("Please enable the compiler option - PAGE_SIZE_2048 in fmc.h\n");
            while(SYS->PDID);
        }
    }
    else
    {
        if(FMC_FLASH_PAGE_SIZE != 512)
        {
            /* FMC_FLASH_PAGE_SIZE is different from target device's */
            printf("Please disable the compiler option - PAGE_SIZE_2048 in fmc.h\n");
            while(SYS->PDID);
        }
    }


    SYS_UnlockReg();
if(easyflash_init() == EF_NO_ERR)
    {
       printf("easyflash_init PASS\n\r");
	
    }
		 if(elog_init() == EF_NO_ERR)
        {
            elog_set_fmt(ELOG_LVL_ASSERT,  ELOG_FMT_ALL & ~ELOG_FMT_P_INFO);
            elog_set_fmt(ELOG_LVL_ERROR,   ELOG_FMT_LVL |  (ELOG_FMT_TAG  | ELOG_FMT_TIME));
            elog_set_fmt(ELOG_LVL_WARN,    ELOG_FMT_LVL |  (ELOG_FMT_TAG  | ELOG_FMT_TIME));
            elog_set_fmt(ELOG_LVL_INFO,    ELOG_FMT_LVL |  (ELOG_FMT_TAG  | ELOG_FMT_TIME));
            elog_set_fmt(ELOG_LVL_DEBUG,   ELOG_FMT_ALL & ~(ELOG_FMT_FUNC | ELOG_FMT_P_INFO));
            elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC | ELOG_FMT_P_INFO));

            /* set EasyLogger assert hook */
            elog_assert_set_hook(elog_user_assert_hook);

            /* initialize EasyLogger Flash plugin */
            elog_flash_init();

            /* start EasyLogger */
            elog_start();
        }
		
		//log_a("Hello EasyLogger!"); /* ??Assert  */
    //log_e("Hello EasyLogger!"); /* ??Error   */
    //log_w("Hello EasyLogger!"); /* ??Warn    */
    //log_i("Hello EasyLogger!"); /* ??Info    */
    //log_d("Hello EasyLogger!"); /* ??Debug   */
    //log_v("Hello EasyLogger!"); /* ??Verbose */
				//elog_flash_flush();
				elog_flash_output_all();
    while (1);
}

/*** (C) COPYRIGHT 2018 Nuvoton Technology Corp. ***/
