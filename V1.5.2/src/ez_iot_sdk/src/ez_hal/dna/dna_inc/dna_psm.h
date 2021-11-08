/*
 *  dna_psm.h -- provide dna-system persistent storage module (PSM) operation interface.
 *  Every object using KEY-VALUE mode stored.
 *
 *  Support two types of storage management:
 *
 *     @ Reset cann't erase (such as feedid)
 *     @ Reset can be erased (such as ssid/psk/device private-key)
 *
 *  ORIGINAL AUTHOR: Xu Chun (chun.xu@broadlink.com.cn)
 *
 *  Copyright (c) 2016 Broadlink Corporation
 */

#ifndef __DNA_PSM_H
#define __DNA_PSM_H

#ifdef __cplusplus
    extern "C" {
#endif

/* dna-system PSM storage management type code */
typedef enum {
    DNA_PSM_RESET_UNABLE_ERASE       =  0,   /*!< Reset cann't erase (No permission to delete existing object) */
    DNA_PSM_RESET_ABLE_ERASE,                /*!< Reset can be erased (Can delete any object) */
    DNA_PSM_USER_SPACE,						 /* this space is defined in user application */
    DNA_PSM_MAX,
} dna_psm_type_e;

/*
*  dna-system PSM initialization.
*
*  @psm_type: psm storage management type code, refer to dna_psm_type_e
*
*  Return 0 means initialize success, if return <0 means occurred error.
*/
int dna_psm_keyvalue_init(unsigned int psm_type);

/*
*  dna-system PSM partition format.
*  This API allows the caller to erase and initialize the PSM 
*  partition. all the existing objects will be erased.
*
*  @psm_type: psm storage management type code, refer to dna_psm_type_e
*
*  Notice: this API will block if other readers/writers are currently using the PSM
*
*  Return DNA_SUCCESS Format operation was successful.
*/
int dna_psm_keyvalue_format(unsigned int psm_type);

/*
*  dna-system PSM set keyvalue.
*
*  @psm_type: psm storage management type code, refer to dna_psm_type_e
*  @key: key name
*  @value: value buffer
*  @len: value valid length
*
*  Return 0 means set keyvalue success, if return <0 means occurred error.
*/
int dna_psm_set_keyvalue(
        unsigned int psm_type,
        const char * key, const void * value, unsigned int len);

/*
*  dna-system PSM get keyvalue.
*
*  @psm_type: psm storage management type code, refer to dna_psm_type_e
*  @key: key name
*  @value: value buffer
*  @maxlen: value buffer max length
*
*  Return value valid length, if return <0 means occurred error.
*/
int dna_psm_get_keyvalue(
        unsigned int psm_type,
        const char * key, void * value, unsigned int maxlen);

/*
*  dna-system PSM delete (or erase) keyvalue.
*
*  @psm_type: psm storage management type code, refer to dna_psm_type_e
*  @key: key name
*
*  Return 0 means delete object success, if return <0 means occurred error.
*/
int dna_psm_del_keyvalue(unsigned int psm_type, const char * key);

/*
*  dna-system PSM uninitialize.
*
*  @psm_type: psm storage management type code, refer to dna_psm_type_e
*
*  Return 0 means uninitialize success, if return <0 means occurred error.
*/
int dna_psm_keyvalue_deinit(unsigned int psm_type);

#ifdef __cplusplus
}
#endif

#endif

