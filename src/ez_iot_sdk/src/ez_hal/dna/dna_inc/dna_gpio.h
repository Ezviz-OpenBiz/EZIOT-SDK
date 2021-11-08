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

#ifndef __DNA_GPIO_H
#define __DNA_GPIO_H

#ifdef __cplusplus
    extern "C" {
#endif

/** @defgroup dna_gpio_enum Enum
  * @{
  */
  
/** @brief This enum defines the gpio port */
enum {
    DNA_GPIO_0 = 0,                            /*!< GPIO0  Pin define */
    DNA_GPIO_1,                                /*!< GPIO1  Pin define */
    DNA_GPIO_2,                                /*!< GPIO2  Pin define */
    DNA_GPIO_3,                                /*!< GPIO3  Pin define */
    DNA_GPIO_4,                                /*!< GPIO4  Pin define */
    DNA_GPIO_5,                                /*!< GPIO5  Pin define */
    DNA_GPIO_6,                                /*!< GPIO6  Pin define */
    DNA_GPIO_7,                                /*!< GPIO7  Pin define */
    DNA_GPIO_8,                                /*!< GPIO8  Pin define */
    DNA_GPIO_9,                                /*!< GPIO9  Pin define */
    DNA_GPIO_10,                               /*!< GPIO10  Pin define */
    DNA_GPIO_11,                               /*!< GPIO11  Pin define */
    DNA_GPIO_12,                               /*!< GPIO12  Pin define */
    DNA_GPIO_13,                               /*!< GPIO13  Pin define */
    DNA_GPIO_14,                               /*!< GPIO14  Pin define */
    DNA_GPIO_15,                               /*!< GPIO15  Pin define */
    DNA_GPIO_16,                               /*!< GPIO16  Pin define */
    DNA_GPIO_17,                               /*!< GPIO17  Pin define */
    DNA_GPIO_18,                               /*!< GPIO18  Pin define */
    DNA_GPIO_19,                               /*!< GPIO19  Pin define */
    DNA_GPIO_20,                               /*!< GPIO20  Pin define */
    DNA_GPIO_21,                               /*!< GPIO21  Pin define */
    DNA_GPIO_22,                               /*!< GPIO22  Pin define */
    DNA_GPIO_23,                               /*!< GPIO23  Pin define */
    DNA_GPIO_24,                               /*!< GPIO24  Pin define */
    DNA_GPIO_25,                               /*!< GPIO25  Pin define */
    DNA_GPIO_26,                               /*!< GPIO26  Pin define */
    DNA_GPIO_27,                               /*!< GPIO27  Pin define */
    DNA_GPIO_28,                               /*!< GPIO28  Pin define */
    DNA_GPIO_29,                               /*!< GPIO29  Pin define */
    DNA_GPIO_30,                               /*!< GPIO30  Pin define */
    DNA_GPIO_31,                               /*!< GPIO31  Pin define */
    DNA_GPIO_32,                               /*!< GPIO32  Pin define */
    DNA_GPIO_33,                               /*!< GPIO33  Pin define */
    DNA_GPIO_34,                               /*!< GPIO34  Pin define */
    DNA_GPIO_35,                               /*!< GPIO35  Pin define */
    DNA_GPIO_36,                               /*!< GPIO36  Pin define */ 
    DNA_GPIO_37,                               /*!< GPIO37  Pin define */ 
    DNA_GPIO_38,                               /*!< GPIO38  Pin define */
    DNA_GPIO_39,                               /*!< GPIO39  Pin define */
    DNA_GPIO_40,                               /*!< GPIO40  Pin define */
    DNA_GPIO_41,                               /*!< GPIO41  Pin define */
    DNA_GPIO_42,                               /*!< GPIO42  Pin define */
    DNA_GPIO_43,                               /*!< GPIO43  Pin define */
    DNA_GPIO_44,                               /*!< GPIO44  Pin define */
    DNA_GPIO_45,                               /*!< GPIO45  Pin define */
    DNA_GPIO_46,                               /*!< GPIO46  Pin define */
    DNA_GPIO_47,                               /*!< GPIO47  Pin define */
    DNA_GPIO_48,                               /*!< GPIO48  Pin define */
    DNA_GPIO_49,                               /*!< GPIO49  Pin define */
    DNA_GPIO_50,                               /*!< GPIO50  Pin define */
    DNA_GPIO_51,                               /*!< GPIO51  Pin define */
    DNA_GPIO_52,                               /*!< GPIO52  Pin define */
    DNA_GPIO_53,                               /*!< GPIO53  Pin define */
    DNA_GPIO_54,                               /*!< GPIO54  Pin define */
    DNA_GPIO_55,                               /*!< GPIO55  Pin define */
    DNA_GPIO_56,                               /*!< GPIO56  Pin define */
    DNA_GPIO_57,                               /*!< GPIO57  Pin define */
    DNA_GPIO_58,                               /*!< GPIO58  Pin define */
    DNA_GPIO_59,                               /*!< GPIO59  Pin define */
    DNA_GPIO_60,                               /*!< GPIO60  Pin define */
    DNA_GPIO_61,                               /*!< GPIO61  Pin define */
    DNA_GPIO_62,                               /*!< GPIO62  Pin define */
    DNA_GPIO_63,                               /*!< GPIO63  Pin define */
    DNA_GPIO_64,                               /*!< GPIO64  Pin define */
    DNA_GPIO_65,                               /*!< GPIO65  Pin define */
    DNA_GPIO_66,                               /*!< GPIO66  Pin define */
    DNA_GPIO_67,                               /*!< GPIO67  Pin define */
    DNA_GPIO_68,                               /*!< GPIO68  Pin define */
    DNA_GPIO_69,                               /*!< GPIO69  Pin define */
    DNA_GPIO_70,                               /*!< GPIO70  Pin define */
    DNA_GPIO_71,                               /*!< GPIO71  Pin define */
    DNA_GPIO_72,                               /*!< GPIO72  Pin define */
    DNA_GPIO_73,                               /*!< GPIO73  Pin define */
    DNA_GPIO_74,                               /*!< GPIO74  Pin define */
    DNA_GPIO_75,                               /*!< GPIO75  Pin define */
    DNA_GPIO_76,                               /*!< GPIO76  Pin define */
    DNA_GPIO_77,                               /*!< GPIO77  Pin define */
    DNA_GPIO_78,                               /*!< GPIO78  Pin define */
    DNA_GPIO_79,                               /*!< GPIO79  Pin define */
    DNA_GPIO_80,                               /*!< GPIO80  Pin define */
    DNA_GPIO_81,                               /*!< GPIO81  Pin define */
    DNA_GPIO_82,                               /*!< GPIO82  Pin define */
    DNA_GPIO_83,                               /*!< GPIO83  Pin define */
    DNA_GPIO_84,                               /*!< GPIO84  Pin define */
    DNA_GPIO_85,                               /*!< GPIO85  Pin define */
    DNA_GPIO_86,                               /*!< GPIO86  Pin define */
    DNA_GPIO_87,                               /*!< GPIO87  Pin define */
    DNA_GPIO_88,                               /*!< GPIO88  Pin define */
    DNA_GPIO_89,                               /*!< GPIO89  Pin define */
    DNA_GPIO_90,                               /*!< GPIO90  Pin define */
    DNA_GPIO_91,                               /*!< GPIO91  Pin define */
    DNA_GPIO_92,                               /*!< GPIO92  Pin define */
    DNA_GPIO_93,                               /*!< GPIO93  Pin define */
    DNA_GPIO_94,                               /*!< GPIO94  Pin define */
    DNA_GPIO_95,                               /*!< GPIO95  Pin define */
    DNA_GPIO_96,                               /*!< GPIO96  Pin define */
    DNA_GPIO_97,                               /*!< GPIO97  Pin define */
    DNA_GPIO_98,                               /*!< GPIO98  Pin define */
    DNA_GPIO_99,                               /*!< GPIO99  Pin define */
    DNA_GPIO_MAX,
};

/** @brief This enum defines gpio direction */
enum {
    DNA_GPIO_INPUT = 0,                        /**< define GPIO input direction */
    DNA_GPIO_OUTPUT,                           /**< define GPIO output direction */
};

/** @brief This enum defines input or output data of GPIO */
enum {
    DNA_GPIO_LOW = 0,                          /**< define GPIO data of low */
    DNA_GPIO_HIGH,                             /**< define GPIO data of high */
};

/** @brief This enum defines pulldown or pullup of GPIO */
enum {
    DNA_GPIO_PULLDOWN = 0,                     /**< define GPIO pulldown when input */
    DNA_GPIO_PULLUP,                           /**< define GPIO pullup when input */
};

/** @brief This enum defines GPIO API error code (return type) */
enum {
    DNA_GPIO_STATUS_ERROR = -3,                /**< Indicate GPIO function execution failed */
    DNA_GPIO_STATUS_ERROR_PIN,                 /**< Indicate a wrong pin number is given */
    DNA_GPIO_STATUS_INVALID_PARAMETER,         /**< Indicate a wrong parameter is given */
    DNA_GPIO_STATUS_OK,                        /**< Indicate GPIO function execute successfully */
};

/**
  * @}
  */

/** @defgroup dna_gpio_eint_enum Enum
  * @{
  */

/** @brief This emun defines the eint trigger mode.  */
enum {
    DNA_GPIO_EINT_LEVEL_LOW = 0,               /**< level and low trigger */
    DNA_GPIO_EINT_LEVEL_HIGH,                  /**< level and high trigger */
    DNA_GPIO_EINT_EDGE_FALLING,                /**< edge and falling trigger */
    DNA_GPIO_EINT_EDGE_RISING,                 /**< edge and rising trigger */
    DNA_GPIO_EINT_EDGE_FALLING_AND_RISING,     /**< edge and falling or rising trigger */
};

/** @brief This emun defines the eint mask mode.  */
enum {
    DNA_GPIO_EINT_UNMASK = 0,                  /**< interrupt unmask */
    DNA_GPIO_EINT_MASK,                        /**< interrupt mask */
};

/**
  * @}
  */


/**
 * @brief     This function is used to initialize target GPIO when the first use.
 * @param[in] pin specifies pin number to init.
 * @return    To indicate whether this function call is successful or not, for example:
 *            If the return value is #DNA_GPIO_STATUS_OK, it means success;
 *            If the return value is #DNA_GPIO_STATUS_ERROR_PIN, it means a wrong pin number is given, the parameter must be verified;
 *            If the return value is #DNA_GPIO_STATUS_ERROR, it means failure.
 * @note
 * @warning
 */
int dna_gpio_init(int pin);

/**
 * @brief     This function is used to uninitialize target GPIO when the last use.
 * @param[in] pin specifies pin number to init.
 * @return    To indicate whether this function call is successful or not, for example:
 *            If the return value is #DNA_GPIO_STATUS_OK, it means success;
 *            If the return value is #DNA_GPIO_STATUS_ERROR_PIN, it means a wrong pin number is given, the parameter must be verified;
 *            If the return value is #DNA_GPIO_STATUS_ERROR, it means failure.
 * @note
 * @warning
 */
int dna_gpio_deinit(int pin);

/**
 * @brief     This function is used to set direction of target GPIO.
 * @param[in] pin specifies pin number to set.
 * @param[in] dir specified the direction of target GPIO, the direction can be input and output.
 * @return    To indicate whether this function call is successful or not, for example:
 *            If the return value is #DNA_GPIO_STATUS_OK, it means success;
 *            If the return value is #DNA_GPIO_STATUS_INVALID_PARAMETER, it means a wrong parameter(except for pin number) is given, the parameter must be verified;
 *            If the return value is #DNA_GPIO_STATUS_ERROR_PIN, it means a wrong pin number is given, the parameter must be verified;
 *            If the return value is #DNA_GPIO_STATUS_ERROR, it means failure.
 * @note
 * @warning
 */
int dna_gpio_set_direction(int pin, int dir);

/**
 * @brief     This function is used to get direction of target GPIO.
 * @param[in] pin specifies pin number to operate.
 * @return    To indicate whether this function call is successful or not, for example:
 *            
 *            If the return value is #DNA_GPIO_OUTPUT, it means output GPIO;
 *            If the return value is #DNA_GPIO_INPUT, it means input GPIO;
 *            If the return value is #DNA_GPIO_STATUS_INVALID_PARAMETER, it means a wrong parameter(except for pin number) is given, the parameter must be verified;
 *            If the return value is #DNA_GPIO_STATUS_ERROR_PIN, it means a wrong pin number is given, the parameter must be verified;
 *            If the return value is #DNA_GPIO_STATUS_ERROR, it means failure.
 * @note
 * @warning
 */
int dna_gpio_get_direction(int pin);

/**
 * @brief     This function is used to set output data of target GPIO.
 * @param[in] pin specifies pin number to operate.
 * @param[in] level represents output data of target GPIO.
 * @return    To indicate whether this function call is successful or not, for example:
 *            If the return value is #DNA_GPIO_STATUS_OK, it means success;
 *            If the return value is #DNA_GPIO_STATUS_INVALID_PARAMETER, it means a wrong parameter(except for pin number) is given, the parameter must be verified;
 *            If the return value is #DNA_GPIO_STATUS_ERROR_PIN, it means a wrong pin number is given, the parameter must be verified;
 *            If the return value is #DNA_GPIO_STATUS_ERROR, it means failure.
 * @note
 * @warning
 */
int dna_gpio_set_output(int pin, int level);

/**
 * @brief     This function is used to get output data of target GPIO that is last set.
 * @param[in] pin specifies pin number to operate.
 * @return    To indicate whether this function call is successful or not, for example:
 *
 *            If the return value is #DNA_GPIO_HIGH, it means output high level;
 *            If the return value is #DNA_GPIO_LOW, it means output low level;
 *            If the return value is #DNA_GPIO_STATUS_INVALID_PARAMETER, it means a wrong parameter(except for pin number) is given, the parameter must be verified;
 *            If the return value is #DNA_GPIO_STATUS_ERROR_PIN, it means a wrong pin number is given, the parameter must be verified;
 *            If the return value is #DNA_GPIO_STATUS_ERROR, it means failure.
 * @note
 * @warning
 */
int dna_gpio_get_output(int pin);

/**
 * @brief     This function is used to get input data of target GPIO.
 * @param[in] pin specifies pin number to operate.
 * @return    To indicate whether this function call is successful or not, for example:
 * 
 *            If the return value is #DNA_GPIO_HIGH, it means input high level;
 *            If the return value is #DNA_GPIO_LOW, it means input low level;
 *            If the return value is #DNA_GPIO_STATUS_INVALID_PARAMETER, it means a wrong parameter(except for pin number) is given, the parameter must be verified;
 *            If the return value is #DNA_GPIO_STATUS_ERROR_PIN, it means a wrong pin number is given, the parameter must be verified;
 *            If the return value is #DNA_GPIO_STATUS_ERROR, it means failure.
 * @note
 * @warning
 */
int dna_gpio_get_input(int pin);

#ifndef CONFIG_GPIO_INNER_PULL_DISABLE
/**
 * @brief     This function is used to set target GPIO to pull-up/pull-down state, after this function,
 *            the input data of target pin will be equivalent to high(pullup) or low(pulldown) if the pin is left unconnected.
 *            This function only works on the pin which has only one pull-up resister and one pull-down resister.
 * @param[in] pin specifies pin number to set.
 * @param[in] mode pull mode of target GPIO.
 * @return    To indicate whether this function call is successful or not, for example:
 *            If the return value is #DNA_GPIO_STATUS_OK, it means success;
 *            If the return value is #DNA_GPIO_STATUS_ERROR_PIN, it means a wrong pin number is given, the parameter must be verified;
 *            If the return value is #DNA_GPIO_STATUS_ERROR, it means failure.
 * @note
 * @warning
 */
int dna_gpio_set_pull_mode(int pin, int mode);
#endif

#ifndef CONFIG_GPIO_INTERRUPT_DISABLE
/**
 * @brief This function is mainly used to initialize and enable GPIO external interrupt.
 * @param[in] pin specifies pin number to initialize.
 * @param[in] mode specifies interrupt trigger mode.
 * @param[in] callback is the function given by user, which will be called at EINT ISR routine.
 * @param[in] data is a reserved parameter for user,it will push to callback.
 * @return    To indicate whether this function call success or not. 
 *            If the return value is #DNA_GPIO_STATUS_OK, it means success;
 *            If the return value is #DNA_GPIO_STATUS_INVALID_PARAMETER, it means a wrong parameter(except for pin number) is given, the parameter must be verified;
 *            If the return value is #DNA_GPIO_STATUS_ERROR_PIN, it means a wrong pin number is given, the parameter must be verified;
 *            If the return value is #DNA_GPIO_STATUS_ERROR, it means failure.
 * @warning
 */
int dna_gpio_set_eint(
    int pin, int mode,
    void (* callback)(void * data),
    void * data
);

/**
 * @brief This function is used to mask the dedicated GPIO external interrupt source.
 * @param[in] pin specifies pin number to set.
 * @param[in] mask interrupt mask mode.
 * @return    To indicate whether this function call successful or not. 
 *            If the return value is #DNA_GPIO_STATUS_OK, it means success;
 *            If the return value is #DNA_GPIO_STATUS_INVALID_PARAMETER, it means a wrong parameter(except for pin number) is given, the parameter must be verified;
 *            If the return value is #DNA_GPIO_STATUS_ERROR_PIN, it means a wrong pin number is given, the parameter must be verified;
 *            If the return value is #DNA_GPIO_STATUS_ERROR, it means failure.
 * @warning
 */
int dna_gpio_set_eint_mask(int pin, int mask);

/**
 * @brief This function is mainly used to set GPIO external interrupt trigger mode.
 * @param[in] pin specifies pin number to set.
 * @param[in] mode specifies interrupt trigger mode.
 * @return    To indicate whether this function call success or not. 
 *            If the return value is #DNA_GPIO_STATUS_OK, it means success;
 *            If the return value is #DNA_GPIO_STATUS_INVALID_PARAMETER, it means a wrong parameter(except for pin number) is given, the parameter must be verified;
 *            If the return value is #DNA_GPIO_STATUS_ERROR_PIN, it means a wrong pin number is given, the parameter must be verified;
 *            If the return value is #DNA_GPIO_STATUS_ERROR, it means failure.
 * @warning
 */
int dna_gpio_set_eint_mode(int pin, int mode);

/**
 * @brief This function is mainly used to set GPIO external interrupt debounce time..
 * @param[in] pin specifies pin number to initialize.
 * @param[in] debounce_ms is the EINT number's hardware debounce time in milliseconds
 * @return    To indicate whether this function call success or not. 
 *            If the return value is #DNA_GPIO_STATUS_OK, it means success;
 *            If the return value is #DNA_GPIO_STATUS_INVALID_PARAMETER, it means a wrong parameter(except for pin number) is given, the parameter must be verified;
 *            If the return value is #DNA_GPIO_STATUS_ERROR_PIN, it means a wrong pin number is given, the parameter must be verified;
 *            If the return value is #DNA_GPIO_STATUS_ERROR, it means failure.
 * @warning
 */
int dna_gpio_set_eint_debounce_time(int pin, int ms);
#endif

#ifdef __cplusplus
}
#endif

#endif

