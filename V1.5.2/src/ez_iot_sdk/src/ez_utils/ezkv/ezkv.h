 /*******************************************************************************
* Copyright © 2017-2021 Ezviz Inc.
*
* All rights reserved. This program and the accompanying materials
* are made available under the terms of the Eclipse Public License v1.0
* and Eclipse Distribution License v1.0 which accompany this distribution.
*
* The Eclipse Public License is available at
*    http://www.eclipse.org/legal/epl-v10.html
* and the Eclipse Distribution License is available at
*   http://www.eclipse.org/org/documents/edl-v10.php.
* 
* Brief:
* 
* 
* Change Logs:
* Date           Author       Notes
* 2021-01-20     xurongjun
*******************************************************************************/
#ifndef _EZKV_H_
#define _EZKV_H_

#include "ez_iot.h"

#define EZ_BASIC_KEY_DEVID "devid"
#define EZ_BASIC_KEY_MASTERKEY "masterkey"
#define EZ_BASIC_KEY_USERID "userid"
#define EZ_TSL_KEY_TSLMAP "tslmap"
#define EZ_TSL_KEY_TSL_PREFIX "tslpf"
#define EZ_HUB_KEY_HUBLIST "hublist"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * The KV database initialization.
     *
     * @param default_kv the default KV
     * @return kv_err_e
     */
    kv_err_e ez_kv_init();

    /**
     * Set a raw KV. If it value is NULL, delete it.
     * If not find it in flash, then create it.
     *
     * @param key KV name
     * @param value KV value
     * @param length KV value size
     * @return kv_err_e
     */
    kv_err_e ez_kv_raw_set(const int8_t *key, int8_t *value, uint32_t length);

    /**
     * Get a raw KV value by key name.
     *
     * @param key KV name
     * @param value KV value
     * @param length KV value length.If it value is NULL, get the length and return 0(success).
     * @return kv_err_e
     */
    kv_err_e ez_kv_raw_get(const int8_t *key, int8_t *value, uint32_t *length);

    /**
     * Delete an KV.
     *
     * @param key KV name
     * @return kv_err_e
     */
    kv_err_e ez_kv_del(const int8_t *key);

    /**
     * Delete the KV pair by its prefix.
     *
     * @param key KV name prefix
     * @return kv_err_e
     */
    kv_err_e ez_kv_del_by_prefix(const int8_t *key_prefix);

    /**
     * Print all KV.
     */
    void ez_kv_print(void);

    /**
     * The KV database finalization.
     */
    void ez_kv_deinit(void);

#ifdef __cplusplus
}
#endif

#endif