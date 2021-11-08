/*
 * Copyright (c) 2020 Bouffalolab.
 *
 * This file is part of
 *     *** Bouffalolab Software Dev Kit ***
 *      (see www.bouffalolab.com).
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of Bouffalo Lab nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include "ut_config.h"
#include "ez_iot_log.h"
#include "utest.h"

#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <stdio.h>
#include <vfs.h>
#include <aos/kernel.h>
#include <aos/yloop.h>
#include <event_device.h>
#include <cli.h>

#include <lwip/tcpip.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <lwip/tcp.h>
#include <lwip/err.h>
#include <http_client.h>
#include <netutils/netutils.h>

#include <bl_uart.h>
#include <bl_chip.h>
#include <bl_wifi.h>
#include <hal_wifi.h>
#include <bl_sec.h>
#include <bl_cks.h>
#include <bl_irq.h>
#include <bl_dma.h>
#include <bl_gpio_cli.h>
#include <bl_wdt_cli.h>
#include <hal_uart.h>
#include <hal_sys.h>
#include <hal_gpio.h>
#include <hal_boot2.h>
#include <hal_board.h>
#include <hal_wifi.h>
#include <looprt.h>
#include <loopset.h>
#include <sntp.h>
#include <bl_sys_time.h>
#include <bl_sys_ota.h>
#include <bl_romfs.h>
#include <fdt.h>
#include <easyflash.h>
#include <wifi_mgmr_ext.h>
#include <libfdt.h>
#include <blog.h>
#include "demo.h"

#include "bscJSON.h"
#include "ez_iot_ap.h"

#define TASK_PRIORITY_FW (30)
#define mainHELLO_TASK_PRIORITY (20)
#define UART_ID_2 (2)
#define WIFI_AP_PSM_INFO_SSID "conf_ap_ssid"
#define WIFI_AP_PSM_INFO_PASSWORD "conf_ap_psk"
#define WIFI_AP_PSM_INFO_PMK "conf_ap_pmk"
#define WIFI_AP_PSM_INFO_BSSID "conf_ap_bssid"
#define WIFI_AP_PSM_INFO_CHANNEL "conf_ap_channel"
#define WIFI_AP_PSM_INFO_IP "conf_ap_ip"
#define WIFI_AP_PSM_INFO_MASK "conf_ap_mask"
#define WIFI_AP_PSM_INFO_GW "conf_ap_gw"
#define WIFI_AP_PSM_INFO_DNS1 "conf_ap_dns1"
#define WIFI_AP_PSM_INFO_DNS2 "conf_ap_dns2"
#define WIFI_AP_PSM_INFO_IP_LEASE_TIME "conf_ap_ip_lease_time"
#define WIFI_AP_PSM_INFO_GW_MAC "conf_ap_gw_mac"
#define CLI_CMD_AUTOSTART1 "cmd_auto1"
#define CLI_CMD_AUTOSTART2 "cmd_auto2"

extern void ble_stack_start(void);

extern uint8_t _heap_start;
extern uint8_t _heap_size; // @suppress("Type cannot be resolved")
extern uint8_t _heap_wifi_start;
extern uint8_t _heap_wifi_size; // @suppress("Type cannot be resolved")
static HeapRegion_t xHeapRegions[] =
    {
        {&_heap_start, (unsigned int)&_heap_size}, //set on runtime
        {&_heap_wifi_start, (unsigned int)&_heap_wifi_size},
        {NULL, 0}, /* Terminates the array. */
        {NULL, 0}  /* Terminates the array. */
};

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    puts("Stack Overflow checked\r\n");
    while (1)
    {
        /*empty here*/
    }
}

void vApplicationMallocFailedHook(void)
{
    printf("Memory Allocate Failed. Current left size is %d bytes\r\n", xPortGetFreeHeapSize());
    while (1)
    {
        /*empty here*/
    }
}

void vApplicationIdleHook(void)
{
    __asm volatile(
        "   wfi     ");
    /*empty*/
}
static void *bl_malloc(size_t size)
{
    return malloc(size);
}
static void bl_free(void *pointer)
{
    free(pointer);
}
static void proc_hellow_entry(void *pvParameters)
{
    vTaskDelay(500);
    bscJSON_Hooks hook = {bl_malloc, bl_free};
    bscJSON_InitHooks(&hook);

    ez_iot_log_init();
    ez_iot_log_start();
    ez_iot_log_filter_lvl(4);

    ez_log_d(TAG_SDK, "%s enter.", __func__);
    ez_log_i(TAG_SDK, "%s enter.", __func__);
    ez_log_w(TAG_SDK, "%s enter.", __func__);
    ez_log_e(TAG_SDK, "%s enter.", __func__);

    utest_log_lv_set(EZ_IOT_UT_DBG_LVL);

    // utest_testcase_run(1, NULL);

    printf("%d %s: RISC-V rv32imafc\r\n", __LINE__, __func__);

    vTaskDelay(10000);
    vTaskDelete(NULL);
}

static void _cli_init()
{
    /*Put CLI which needs to be init here*/
    easyflash_cli_init();
    wifi_mgmr_cli_init();
    httpc_client_cli_init();
}

static int get_dts_addr(const char *name, uint32_t *start, uint32_t *off)
{
    uint32_t addr = hal_board_get_factory_addr();
    const void *fdt = (const void *)addr;
    uint32_t offset;

    if (!name || !start || !off)
    {
        return -1;
    }

    offset = fdt_subnode_offset(fdt, 0, name);
    if (offset <= 0)
    {
        log_error("%s NULL.\r\n", name);
        return -1;
    }

    *start = (uint32_t)fdt;
    *off = offset;

    return 0;
}

static void __opt_feature_init(void)
{
#ifdef CONF_USER_ENABLE_VFS_ROMFS
    romfs_register();
#endif
}

static void aos_loop_proc(void *pvParameters)
{
    int fd_console;
    uint32_t fdt = 0, offset = 0;
    static StackType_t proc_stack_looprt[512];
    static StaticTask_t proc_task_looprt;

    /*Init bloop stuff*/
    looprt_start(proc_stack_looprt, 512, &proc_task_looprt);
    loopset_led_hook_on_looprt();

    easyflash_init();
    vfs_init();
    vfs_device_init();

    /* uart */
#if 1
    if (0 == get_dts_addr("uart", &fdt, &offset))
    {
        vfs_uart_init(fdt, offset);
    }
#else
    vfs_uart_init_simple_mode(0, 7, 16, 2 * 1000 * 1000, "/dev/ttyS0");
#endif
    // if (0 == get_dts_addr("gpio", &fdt, &offset))
    // {
    //   hal_gpio_init_from_dts(fdt, offset);
    // }

    __opt_feature_init();
    aos_loop_init();

    fd_console = aos_open("/dev/ttyS0", 0);
    if (fd_console >= 0)
    {
        printf("Init CLI with event Driven\r\n");
        aos_cli_init(0);
        aos_poll_read_fd(fd_console, aos_cli_event_cb_read_get(), (void *)0x12345678);
        _cli_init();
    }
    ez_iot_wifi_init();
    aos_loop_run();

    puts("------------------------------------------\r\n");
    puts("+++++++++Critical Exit From Loop++++++++++\r\n");
    puts("******************************************\r\n");
    vTaskDelete(NULL);
}

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize)
{
    /* If the buffers to be provided to the Idle task are declared inside this
    function then they must be declared static - otherwise they will be allocated on
    the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/* configSUPPORT_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize)
{
    /* If the buffers to be provided to the Timer task are declared inside this
    function then they must be declared static - otherwise they will be allocated on
    the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
    task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

void vAssertCalled(void)
{
    volatile uint32_t ulSetTo1ToExitFunction = 0;

    taskDISABLE_INTERRUPTS();
    while (ulSetTo1ToExitFunction != 1)
    {
        __asm volatile("NOP");
    }
}

static void _dump_boot_info(void)
{
    char chip_feature[40];
    const char *banner;

    puts("Booting BL602 Chip...\r\n");

    /*Display Banner*/
    if (0 == bl_chip_banner(&banner))
    {
        puts(banner);
    }
    puts("\r\n");
    /*Chip Feature list*/
    puts("\r\n");
    puts("------------------------------------------------------------\r\n");
    puts("RISC-V Core Feature:");
    bl_chip_info(chip_feature);
    puts(chip_feature);
    puts("\r\n");

    puts("Build Version: ");
    puts(BL_SDK_VER); // @suppress("Symbol is not resolved")
    puts("\r\n");
    puts("Build Date: ");
    puts(__DATE__);
    puts("\r\n");
    puts("Build Time: ");
    puts(__TIME__);
    puts("\r\n");
    puts("------------------------------------------------------------\r\n");
}

static void system_init(void)
{
    blog_init();
    bl_irq_init();
    bl_sec_init();
    bl_sec_test();
    bl_dma_init();
    hal_boot2_init();

    /* board config is set after system is init*/
    hal_board_cfg(0);
}

static void system_thread_init()
{
    /*nothing here*/
}
__attribute__((constructor(101))) void uart_init(void)
{
    bl_uart_init(0, 16, 7, 255, 255, 2 * 1000 * 1000);
}

void *bl_realloc(void *p, size_t new_size)
{
    void *new_ptr;
    if (NULL == p)
    {
        return malloc(new_size);
    }
    if (0 == new_size)
    {
        free(p);
        return NULL;
    }
    new_ptr = malloc(new_size);
    if (new_ptr != NULL && p != NULL)
    {
        size_t old_size = strlen((char *)p) + 1;

        /* get the size of old memory block */
        if (new_size > old_size)
            memcpy(new_ptr, p, old_size);
        else
            memcpy(new_ptr, p, new_size);

        free(p);
    }
    return new_ptr;
}

void bfl_main()
{
    static StackType_t aos_loop_proc_stack[1024];
    static StaticTask_t aos_loop_proc_task;
    static StackType_t proc_hellow_stack[1024];
    static StaticTask_t proc_hellow_task;

    /*Init UART In the first place*/
    // bl_uart_init(0, 16, 7, 255, 255, 2 * 1000 * 1000);
    puts("Starting bl602 now....\r\n");
    //GLB_Set_EM_Sel(0);

    _dump_boot_info();

    vPortDefineHeapRegions(xHeapRegions);
    printf("Heap %u@%p, %u@%p\r\n",
           (unsigned int)&_heap_size, &_heap_start,
           (unsigned int)&_heap_wifi_size, &_heap_wifi_start);

    system_init();
    system_thread_init();

    puts("[OS] Starting proc_hellow_entry task...\r\n");
    xTaskCreateStatic(proc_hellow_entry, (char *)"hellow", 1024, NULL, 15, proc_hellow_stack, &proc_hellow_task);
    puts("[OS] Starting aos_loop_proc task...\r\n");
    xTaskCreateStatic(aos_loop_proc, (char *)"event_loop", 1024, NULL, 15, aos_loop_proc_stack, &aos_loop_proc_task);
    puts("[OS] Starting TCP/IP Stack...\r\n");
    tcpip_init(NULL, NULL);

    puts("[OS] Starting OS Scheduler...\r\n");
    vTaskStartScheduler();
}
