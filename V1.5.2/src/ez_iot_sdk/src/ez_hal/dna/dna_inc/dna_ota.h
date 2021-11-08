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

#ifndef __DNA_OTA_H
#define __DNA_OTA_H

#ifdef __cplusplus
    extern "C" {
#endif

#define DNA_OTA_URL_MAX_LEN                     128        /* contain '\0' */

/* dna-system firmware OTA status code */
typedef enum {
    DNA_OTA_START              = 0,
    DNA_OTA_IMAGE_DOWNLOADING,
    DNA_OTA_IMAGE_INSTALL,
    DNA_OTA_DONE,
    /* Error code */
    DNA_OTA_BUSY,
    DNA_OTA_URL_DNS_TIMEOUT,
    DNA_OTA_SERV_CONNECT_FAIL,
    DNA_OTA_IMAGE_NOT_FOUND,                /* such as HTTP 404 error */
    DNA_OTA_IMAGE_DOWNLOAD_FAIL,            /* such as network disconnect */
    DNA_OTA_IMAGE_FORMAT_ERROR,
    DNA_OTA_IMAGE_INSTALL_FAIL,             /* such as disk write abnormal */
} dna_ota_status_e;

/* dna-system firmware OTA progress descriptor */
typedef struct dna_ota_progress {
    unsigned char status;
    unsigned char percent;
} dna_ota_progress_t;

/* 
*  dna-system firmware OTA data header,
*  It used for APP push data blocks to module or device (NOT auto HTTP download).
*/
typedef struct dna_ota_data_header {
    unsigned char * data;
    unsigned int len;
} dna_ota_data_header_t;

/**
 * @brief Config whether reboot when firmware upgrade done.
 * @param[in] enable Auto reboot config value.
 * @return DNA_SUCCESS on success.
 * @note default is auto reboot.
 * @warning
 */
int dna_firmware_ota_auto_reboot(int enable);

/**
 * @brief Firmware remote online upgrade.
 *  This function return immediately (asynchronous mode).
 * @param[in] url Firmware download url.
 * @return DNA_SUCCESS on success.
 * @note
 * @warning
 */
int dna_firmware_ota(const char * url);

/**
 * @brief Firmware remote online upgrade.
 *  This function used for APP push data blocks to module or device (NOT auto HTTP download).
 * @param[in] data /length OTA data block and length.
 * @return DNA_SUCCESS on success.
 * @note
 * @warning
 */
int dna_firmware_ota_data(const unsigned char * data, int len);

/**
 * @brief Firmware remote online upgrade stop interface.
 * @param[in] none.
 * @return DNA_SUCCESS on success.
 * @note
 * @warning
 */
int dna_firmware_ota_stop(void);

/**
*  @brief Get Firmware remote online upgrade progress.
*  @param[out] progress OTA status and percent (0 - 100%)
*  @return DNA_SUCCESS on success.
*  @note
*  @warning
*/
int dna_firmware_ota_progress(dna_ota_progress_t * progress);

#ifdef __cplusplus
}
#endif

#endif

