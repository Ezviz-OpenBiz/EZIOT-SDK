/*******************************************************************************
*
*               COPYRIGHT (c) 2015-2016 Broadlink Corporation
*                         All Rights Reserved
*
* The source code contained or described herein and all documents
* related to the source code ("Material") are owned by Broadlink
* Corporation or its licensors.  Title to the Material remains
* with Broadlink Corporation or its suppliers and licensors.
*
* The Material is protected by worldwide copyright and trade secret
* laws and treaty provisions. No part of the Material may be used,
* copied, reproduced, modified, published, uploaded, posted, transmitted,
* distributed, or disclosed in any way except in accordance with the
* applicable license agreement.
*
* No license under any patent, copyright, trade secret or other
* intellectual property right is granted to or conferred upon you by
* disclosure or delivery of the Materials, either expressly, by
* implication, inducement, estoppel, except in accordance with the
* applicable license agreement.
*
* Unless otherwise agreed by Broadlink in writing, you may not remove or
* alter this notice or any other notice embedded in Materials by Broadlink
* or Broadlink's suppliers or licensors in any way.
*
*******************************************************************************/

#ifndef __DNA_UART_H
#define __DNA_UART_H

#ifdef __cplusplus
    extern "C" {
#endif

#if defined(__ssv6060__) || defined(__bl0907__) || defined(__rda5981__)
#define DNA_UART_ID                     DNA_UART1_ID
#else
#define DNA_UART_ID                     DNA_UART0_ID
#endif

/** @defgroup dna_uart_enum Enum
  * @{
  */

/**
 *  @brief UART handle definition
 */
typedef void * dna_uart_handle_t;

/**
 *  @brief UART No. type definition
 */
typedef enum {
    DNA_UART0_ID = 0,                           /*!< UART0 port define */
    DNA_UART1_ID = 1,                           /*!< UART1 port define */
    DNA_UART2_ID = 2,                           /*!< UART2 port define */
} dna_uart_id_e;

/**
 *  @brief UART data bits definition
 */
typedef enum {
    DNA_UART_8BITS = 0,                         /*!< UART data 8bits define */
    DNA_UART_9BITS,                             /*!< UART data 9bits define (now not support) */
} dna_uart_databits_e;

/**
 *  @brief UART parity type definition
 */
typedef enum {
    DNA_UART_PARITY_NONE = 0,                   /*!< UART none define */
    DNA_UART_PARITY_ODD,                        /*!< UART odd define */
    DNA_UART_PARITY_EVEN,                       /*!< UART even define */
} dna_uart_parity_e;

/**
 *  @brief UART stop bits type definition
 */
typedef enum {
    DNA_UART_STOP_1BITS = 0,                    /*!< UART stop bits length: 1 bits */
    DNA_UART_STOP_1_5BITS,                      /*!< UART stop bits length: 1.5 bits */
    DNA_UART_STOP_2BITS,                        /*!< UART stop bits length: 2 bits */
} dna_uart_stopbits_e;

/** Enum to select flow control method to be used for
*   uart transfer
*/
typedef enum {
	DNA_UART_FLOW_CONTROL_NONE = 0,             /** Flow control disabled */
	DNA_UART_FLOW_CONTROL_SW,                   /** Software flow control (now not support) */
	DNA_UART_FLOW_CONTROL_HW,                   /** Hardware flow control (now not support) */
} dna_uart_flowctrl_e;


/**
  * @}
  */

int dna_uart_init(unsigned int id);

int dna_uart_deinit(unsigned int id);

/** Open handle to UART Device
 *
 * This function opens the handle to UART device and enables application to use
 * the device. This handle should be used in other calls related to UART device.
 *
 * \param[in] id Port ID of UART, refer to dna_uart_id_e
 * \param[in] databits, refer to dna_uart_databits_e
 * \param[in] baud Baud rate (current support: 110/300/1200/2400/4800/9600/19200/38400/57600/115200/230400/460800/921600).
 * \param[in] parity, refer to dna_uart_parity_e
 * \param[in] stopbits, refer to dna_uart_stopbits_e
 * \param[in] flow Flow control, refer to dna_uart_flowctrl_e
 * \return Handle to the UART device.
 * \return NULL on error.
 * Notice: if used uart1, must shut down debug log(dna_debug_close); 
 */
dna_uart_handle_t dna_uart_open(
#if defined(__mt7628__) || defined(__mt7688__) || defined(__linux__)
    const char *devname,
#else
    unsigned int id,
#endif
    unsigned int databits,
    unsigned int baud, unsigned int parity,
    unsigned int stopbits, unsigned int flow);

/** Write data to UART
 *
 * This function is used to write data to the UART in order to transmit them
 * serially over serial out line.
 *
 * \param[in] handle Handle to the UART device returned by dna_uart_open().
 * \param[in] buff Pointer to a buffer which has the data to be written out.
 * \param[in] len Number of bytes to be written.
 * \return Number of bytes written.
 */
int dna_uart_write(
    dna_uart_handle_t handle,
    const unsigned char * buff,
    int len);

/** Read data from UART
 *
 * This function is used to read data arriving serially over serial in line.
 *
 * \param[in] handle Handle to the UART device returned by dna_uart_open().
 * \param[out] buff Pointer to an allocated buffer of size equal to or more
 * than the value of the third parameter num.
 * \param[in] len The maximum number of bytes to be read from the UART.
 * Note that the actual read bytes can be less than this.
 * \return Number of bytes read.
 */
int dna_uart_read(
    dna_uart_handle_t handle,
    unsigned char * buff,
    int len);

/** Close handle to UART Device
 *
 * This function closes the handle to UART device and resets the UART.
 *
 * \param[in] handle Handle to the UART device returned by dna_uart_open().
 * \return DNA_SUCCESS on success
 * \return -DNA_FAIL on error.
 */
int dna_uart_close(dna_uart_handle_t handle);



/** 
 * Get UART recive data count.
 */

int dna_uart_recv_len(dna_uart_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif

