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

#ifndef __DNA_PWM_H
#define __DNA_PWM_H

#ifdef __cplusplus
    extern "C" {
#endif

/** @defgroup dna_pwm_enum Enum
  * @{
  */
  
/** @brief This enum defines the pwm channel */
enum {
    DNA_PWM_CHANNEL_0 = 0,                              
    DNA_PWM_CHANNEL_1,
    DNA_PWM_CHANNEL_2,
    DNA_PWM_CHANNEL_3,
    DNA_PWM_CHANNEL_4,
    DNA_PWM_CHANNEL_5,
    DNA_PWM_CHANNEL_6,
    DNA_PWM_CHANNEL_7,
    DNA_PWM_CHANNEL_18 = 18,                            /*!< GPIO35  Pin define */
    DNA_PWM_CHANNEL_19,                                 /*!< GPIO36  Pin define */
    DNA_PWM_CHANNEL_20,                                 /*!< GPIO37  Pin define */
    DNA_PWM_CHANNEL_22 = 22,                            /*!< GPIO39  Pin define */
    DNA_PWM_CHANNEL_23,                                 /*!< GPIO2  Pin define */
    DNA_PWM_CHANNEL_24,                                 /*!< GPIO3  Pin define */
    DNA_PWM_CHANNEL_25,                                 /*!< GPIO24  Pin define */
    DNA_PWM_CHANNEL_26,                                 /*!< GPIO25  Pin define */
    DNA_PWM_CHANNEL_34 = 34,                            /*!< GPIO33  Pin define */
    DNA_PWM_CHANNEL_36 = 36,                            /*!< GPIO57  Pin define */
    DNA_PWM_CHANNEL_37,                                 /*!< GPIO58  Pin define */
};

/** @brief pwm clock source seletion */
enum {
    DNA_PWM_CLOCK_32KHZ  = 0,                           /**< pwm clock source: Embedded 32KHz clock */
    DNA_PWM_CLOCK_2MHZ   = 1,                           /**< pwm clock srouce: Embedded 2MHz  clock */
    DNA_PWM_CLOCK_10MHZ	 = 2,							/**< 7682 clock srouce: Embedded 13MHz  clock*/
    DNA_PWM_CLOCK_13MHZ	 = 3,							/**< 7682 clock srouce: Embedded 13MHz  clock*/
    DNA_PWM_CLOCK_40MHZ  = 4,                           /**< pwm clock srouce: External 40MHz clock */
};

/** @brief	This enum defines the API error code (return type)  */
enum {
    DNA_PWM_STATUS_ERROR = -3,                          /**< An error occurred during the function call. */
    DNA_PWM_STATUS_ERROR_CHANNEL,                       /**< A wrong PWM channel is given. */
    DNA_PWM_STATUS_INVALID_PARAMETER,                   /**< A wrong parameter is given. */
    DNA_PWM_STATUS_OK,                                  /**< No error during the function call. */
};

/**
  * @}
  */


/**
 * @brief    This function initializes the PWM hardware channel and source clock.
 * @param[in] channel is PWM channel number.
 * @param[in]  clock is the PWM  source clock.
 * @return    To indicate whether this function call is successful or not.
 *            If the return value is #DNA_PWM_STATUS_OK, it means success;
 *            If the return value is #DNA_PWM_STATUS_ERROR_CHANNEL, it means a wrong channel number is given, the parameter must be verified;
 *            If the return value is #DNA_PWM_STATUS_INVALID_PARAMETER, it means a wrong parameter is given. The parameter needs to be verified;
 *            If the return value is #DNA_PWM_STATUS_ERROR, it means failure.
 * @note
 * @waring
 */
int dna_pwm_init(int channel, int clock);

/**
 * @brief    This function uninitialize the PWM hardware channel.
 * @param[in] channel is PWM channel number.
 * @return    To indicate whether this function call is successful or not.
 *            If the return value is #DNA_PWM_STATUS_OK, it means success;
 *            If the return value is #DNA_PWM_STATUS_ERROR_CHANNEL, it means a wrong channel number is given, the parameter must be verified;
 *            If the return value is #DNA_PWM_STATUS_INVALID_PARAMETER, it means a wrong parameter is given. The parameter needs to be verified;
 *            If the return value is #DNA_PWM_STATUS_ERROR, it means failure.
 * @note
 * @waring
 */
int dna_pwm_deinit(int channel);

/**
 * @brief    This function config the PWM hardware channel, such as freq and duty ratio.
 * @param[in] channel is PWM channel number.
 * @param[in] freq is PWM channel output pulse frequency.
 * @param[in] duty_ratio is pwm channel output pulse duty ratio (0 - 1000).
 * @return    To indicate whether this function call is successful or not.
 *            If the return value is #DNA_PWM_STATUS_OK, it means success;
 *            If the return value is #DNA_PWM_STATUS_ERROR_CHANNEL, it means a wrong channel number is given, the parameter must be verified;
 *            If the return value is #DNA_PWM_STATUS_INVALID_PARAMETER, it means a wrong parameter is given. The parameter needs to be verified;
 *            If the return value is #DNA_PWM_STATUS_ERROR, it means failure.
 * @note
 * @waring
 */
int dna_pwm_config(int channel, unsigned int freq, unsigned int duty_ratio);

/**
 * @brief This function is  used to start the PWM execution.
 * @param[in]  channel is PWM channel number.
 * @return    To indicate whether this function call is successful or not.
 *            If the return value is #DNA_PWM_STATUS_OK, it means success;
 *            If the return value is #DNA_PWM_STATUS_ERROR_CHANNEL, it means a wrong channel number is given, the parameter must be verified;
 *            If the return value is #DNA_PWM_STATUS_INVALID_PARAMETER, it means a wrong parameter is given. The parameter needs to be verified;
 *            If the return value is #DNA_PWM_STATUS_ERROR, it means failure.
 * @note
 * @waring
 */
int dna_pwm_start(int channel);

/**
 * @brief  This function is mainly used to stop the PWM execution.
 * @param[in]  channel is PWM channel number.
 * @return    To indicate whether this function call is successful or not.
 *            If the return value is #DNA_PWM_STATUS_OK, it means success;
 *            If the return value is #DNA_PWM_STATUS_ERROR_CHANNEL, it means a wrong channel number is given, the parameter must be verified;
 *            If the return value is #DNA_PWM_STATUS_INVALID_PARAMETER, it means a wrong parameter is given. The parameter needs to be verified;
 *            If the return value is #DNA_PWM_STATUS_ERROR, it means failure.
 * @note
 * @waring
 */
int dna_pwm_stop(int channel);

#ifdef __cplusplus
}
#endif

#endif

