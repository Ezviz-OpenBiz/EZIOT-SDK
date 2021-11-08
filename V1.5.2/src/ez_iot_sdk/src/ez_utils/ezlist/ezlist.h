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
* 
*******************************************************************************/

#ifndef EZLIST_H
#define EZLIST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /* types */
    typedef struct ezlist_s ezlist_t;
    typedef struct ezlist_obj_s ezlist_obj_t;

    enum
    {
        ezlist_THREADSAFE = (0x01) /*!< make it thread-safe */
    };

    /* member functions
    *
    * All the member functions can be accessed in both ways:
    *  - tbl->addlast(tbl, ...);  // easier to switch the container type to other kinds.
    *  - ezlist_addlast(tbl, ...); // where avoiding pointer overhead is preferred.
    */
    extern ezlist_t *ezlist(int options); /*!< ezlist constructor */
    extern size_t ezlist_setsize(ezlist_t *list, size_t max);

    extern bool ezlist_addfirst(ezlist_t *list, const void *data, size_t size);
    extern bool ezlist_addlast(ezlist_t *list, const void *data, size_t size);
    extern bool ezlist_addat(ezlist_t *list, int index, const void *data, size_t size);

    extern void *ezlist_getfirst(ezlist_t *list, size_t *size, bool newmem);
    extern void *ezlist_getlast(ezlist_t *list, size_t *size, bool newmem);
    extern void *ezlist_getat(ezlist_t *list, int index, size_t *size, bool newmem);

    extern void *ezlist_popfirst(ezlist_t *list, size_t *size);
    extern void *ezlist_poplast(ezlist_t *list, size_t *size);
    extern void *ezlist_popat(ezlist_t *list, int index, size_t *size);

    extern bool ezlist_removefirst(ezlist_t *list);
    extern bool ezlist_removelast(ezlist_t *list);
    extern bool ezlist_removeat(ezlist_t *list, int index);

    extern bool ezlist_getnext(ezlist_t *list, ezlist_obj_t *obj, bool newmem);

    extern size_t ezlist_size(ezlist_t *list);
    extern size_t ezlist_datasize(ezlist_t *list);
    extern void ezlist_reverse(ezlist_t *list);
    extern void ezlist_clear(ezlist_t *list);

    extern void *ezlist_toarray(ezlist_t *list, size_t *size);
    extern char *ezlist_tostring(ezlist_t *list);

    extern void ezlist_lock(ezlist_t *list);
    extern void ezlist_unlock(ezlist_t *list);

    extern void ezlist_free(ezlist_t *list);

    /**
     * ezlist container object
     */
    struct ezlist_s
    {
        /* encapsulated member functions */
        // size_t (*setsize)(ezlist_t *list, size_t max);

        // bool (*addfirst)(ezlist_t *list, const void *data, size_t size);
        // bool (*addlast)(ezlist_t *list, const void *data, size_t size);
        // bool (*addat)(ezlist_t *list, int index, const void *data, size_t size);

        // void *(*getfirst)(ezlist_t *list, size_t *size, bool newmem);
        // void *(*getlast)(ezlist_t *list, size_t *size, bool newmem);
        // void *(*getat)(ezlist_t *list, int index, size_t *size, bool newmem);

        // void *(*popfirst)(ezlist_t *list, size_t *size);
        // void *(*poplast)(ezlist_t *list, size_t *size);
        // void *(*popat)(ezlist_t *list, int index, size_t *size);

        // bool (*removefirst)(ezlist_t *list);
        // bool (*removelast)(ezlist_t *list);
        // bool (*removeat)(ezlist_t *list, int index);

        // bool (*getnext)(ezlist_t *list, ezlist_obj_t *obj, bool newmem);

        // void (*reverse)(ezlist_t *list);
        // void (*clear)(ezlist_t *list);

        // size_t (*size)(ezlist_t *list);
        // size_t (*datasize)(ezlist_t *list);

        // void *(*toarray)(ezlist_t *list, size_t *size);
        // char *(*tostring)(ezlist_t *list);

        // void (*lock)(ezlist_t *list);
        // void (*unlock)(ezlist_t *list);

        // void (*free)(ezlist_t *list);

        /* private variables - do not access directly */
        void *mutex;    /*!< initialized when ezlist_OPT_THREADSAFE is given */
        size_t num;     /*!< number of elements */
        size_t max;     /*!< maximum number of elements. 0 means no limit */
        size_t datasum; /*!< total sum of data size, does not include name size */

        ezlist_obj_t *first; /*!< first object pointer */
        ezlist_obj_t *last;  /*!< last object pointer */
    };

    /**
 * ezlist node data structure.
 */
    struct ezlist_obj_s
    {
        void *data;  /*!< data */
        size_t size; /*!< data size */

        ezlist_obj_t *prev; /*!< previous link */
        ezlist_obj_t *next; /*!< next link */
    };

#ifdef __cplusplus
}
#endif

#endif /* ezlist_H */
