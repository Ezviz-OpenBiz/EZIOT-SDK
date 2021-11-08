/******************************************************************************
 * qLibc
 *
 * Copyright (c) 2010-2015 Seungyoung Kim.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

/**
 * @file ezlist.c Doubly Linked-list implementation.
 *
 * ezlist container is a doubly Linked-List implementation.
 * ezlist provides uniformly named methods to add, get, pop and remove an
 * element at the beginning and end of the list. These operations allow ezlist
 * to be used as a stack, queue, or double-ended queue.
 *
 * @code
 *  [Conceptional Data Structure Diagram]
 *
 *  last~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
 *                                                          |
 *          +-----------+  doubly  +-----------+  doubly  +-|---------+
 *  first~~~|~>   0   <~|~~~~~~~~~~|~>   1   <~|~~~~~~~~~~|~>   N     |
 *          +-----|-----+  linked  +-----|-----+  linked  +-----|-----+
 *                |                      |                      |
 *          +-----v---------------+      |                +-----v-----+
 *          | DATA A              |      |                | DATA N    |
 *          +---------------------+      |                +-----------+
 *                 +---------------------v------------------+
 *                 | DATA B                                 |
 *                 +----------------------------------------+
 * @endcode
 *
 * @code
 *  // create a list.
 *  ezlist_t *list = ezlist(ezlist_THREADSAFE);
 *
 *  // insert elements
 *  list->addlast(list, "e1", sizeof("e1"));
 *  list->addlast(list, "e2", sizeof("e2"));
 *  list->addlast(list, "e3", sizeof("e3"));
 *
 *  // get
 *  char *e1 = (char*)list->getfirst(list, NULL, true));    // malloced
 *  char *e3  = (char*)list->getat(list, -1, NULL, false)); // no malloc
 *  (...omit...)
 *  free(e1);
 *
 *  // pop (get and remove)
 *  char *e2 = (char*)list->popat(list, 1, NULL)); // get malloced copy
 *  (...omit...)
 *  free(e2);
 *
 *  // debug output
 *  list->debug(list, stdout, true);
 *
 *  // traversal
 *  ezlist_obj_t obj;
 *  memset((void*)&obj, 0, sizeof(obj)); // must be cleared before call
 *  list->lock(list);
 *  while (list->getnext(list, &obj, false) == true) {
 *    printf("DATA=%s, SIZE=%zu\n", (char*)obj.data, obj.size);
 *  }
 *  list->unlock(list);
 *
 *  // free object
 *  list->free(list);
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/errno.h>
#include "ezconfig.h"
#include "mcuconfig.h"
#include "hal_thread.h"
#include "ezlist.h"

#ifndef _DOXYGEN_SKIP

static void *get_at(ezlist_t *list, int index, size_t *size, bool newmem, bool remove);
static ezlist_obj_t *get_obj(ezlist_t *list, int index);
static bool remove_obj(ezlist_t *list, ezlist_obj_t *obj);

#endif

#define WAIT_FOREVER (~0UL)
#define ezdev_pthreadMutexInit() hal_thread_mutex_create()
#define ezdev_pthreadMutexLock(mutex, wait) hal_thread_mutex_lock((mutex))
#define ezdev_pthreadMutexUnlock(mutex) hal_thread_mutex_unlock((mutex))
#define ezdev_pthreadMutexDestroy(mutex) hal_thread_mutex_destroy((mutex))

/**
 * Create new ezlist_t linked-list container
 *
 * @param options   combination of initialization options.
 *
 * @return a pointer of malloced ezlist_t container, otherwise returns NULL.
 * @retval errno will be set in error condition.
 *  -ENOMEM : Memory allocation failure.
 *
 * @code
 *  ezlist_t *list = ezlist(0);
 * @endcode
 *
 * @note
 *   Available options:
 *   - ezlist_THREADSAFE - make it thread-safe.
 */
ezlist_t *ezlist(int options) {
    ezlist_t *list = (ezlist_t *) calloc(1, sizeof(ezlist_t));
    if (list == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    // handle options.
    if (options & ezlist_THREADSAFE) {
        list->mutex = ezdev_pthreadMutexInit();
        if (list->mutex == NULL) {
            errno = ENOMEM;
            free(list);
            return NULL;
        }
    }

    // member methods
    // list->setsize = ezlist_setsize;

    // list->addfirst = ezlist_addfirst;
    // list->addlast = ezlist_addlast;
    // list->addat = ezlist_addat;

    // list->getfirst = ezlist_getfirst;
    // list->getlast = ezlist_getlast;
    // list->getat = ezlist_getat;
    // list->getnext = ezlist_getnext;

    // list->popfirst = ezlist_popfirst;
    // list->poplast = ezlist_poplast;
    // list->popat = ezlist_popat;

    // list->removefirst = ezlist_removefirst;
    // list->removelast = ezlist_removelast;
    // list->removeat = ezlist_removeat;

    // list->reverse = ezlist_reverse;
    // list->clear = ezlist_clear;

    // list->size = ezlist_size;
    // list->datasize = ezlist_datasize;

    // list->toarray = ezlist_toarray;
    // list->tostring = ezlist_tostring;

    // list->lock = ezlist_lock;
    // list->unlock = ezlist_unlock;

    // list->free = ezlist_free;

    return list;
}

/**
 * ezlist->setsize(): Limit maximum number of elements allowed in this list.
 *
 * @param list  ezlist_t container pointer.
 * @param max   maximum number of elements. 0 means no limit.
 *
 * @return previous maximum number.
 *
 * @note
 *  The default maximum number of elements is unlimited.
 */
size_t ezlist_setsize(ezlist_t *list, size_t max) {
    ezlist_lock(list);
    size_t old = list->max;
    list->max = max;
    ezlist_unlock(list);
    return old;
}

/**
 * ezlist->addfirst(): Inserts a element at the beginning of this list.
 *
 * @param list  ezlist_t container pointer.
 * @param data  a pointer which points data memory.
 * @param size  size of the data.
 *
 * @return true if successful, otherwise returns false.
 * @retval errno will be set in error condition.
 *  - ENOBUFS : List full. Only happens when this list has set to have limited
 *              number of elements.
 *  - EINVAL  : Invalid argument.
 *  - ENOMEM  : Memory allocation failure.
 *
 * @code
 *  // create a sample object.
 *  struct my_obj obj;
 *
 *  // create a list and add the sample object.
 *  ezlist_t *list = ezlist();
 *  list->addfirst(list, &obj, sizeof(struct my_obj));
 * @endcode
 */
bool ezlist_addfirst(ezlist_t *list, const void *data, size_t size) {
    return ezlist_addat(list, 0, data, size);
}

/**
 * ezlist->addlast(): Appends a element to the end of this list.
 *
 * @param list  ezlist_t container pointer.
 * @param data  a pointer which points data memory.
 * @param size  size of the data.
 *
 * @return true if successful, otherwise returns false.
 * @retval errno will be set in error condition.
 *  - ENOBUFS : List full. Only happens when this list has set to have limited
 *              number of elements.
 *  - EINVAL  : Invalid argument.
 *  - ENOMEM  : Memory allocation failure.
 */
bool ezlist_addlast(ezlist_t *list, const void *data, size_t size) {
    return ezlist_addat(list, -1, data, size);
}

/**
 * ezlist->addat(): Inserts a element at the specified position in this
 * list.
 *
 * @param list   ezlist_t container pointer.
 * @param index  index at which the specified element is to be inserted.
 * @param data   a pointer which points data memory.
 * @param size   size of the data.
 *
 * @return true if successful, otherwise returns false.
 * @retval errno will be set in error condition.
 *  - ENOBUFS : List full. Only happens when this list has set to have limited
 *              number of elements.
 *  - ERANGE  : Index out of range.
 *  - EINVAL  : Invalid argument.
 *  - ENOMEM  : Memory allocation failure.
 *
 * @code
 *                     first           last      new
 *  Linked-list        [ A ]<=>[ B ]<=>[ C ]?==?[   ]
 *  (positive index)     0       1       2        3
 *  (negative index)    -3      -2      -1
 * @endcode
 *
 * @code
 *  ezlist_t *list = ezlist();
 *  list->addat(list, 0, &obj, sizeof(obj));  // same as addfirst().
 *  list->addat(list, -1, &obj, sizeof(obj)); // same as addlast().
 * @endcode
 *
 * @note
 *  Index starts from 0.
 */
bool ezlist_addat(ezlist_t *list, int index, const void *data, size_t size) {
    // check arguments
    if (data == NULL || size <= 0) {
        errno = EINVAL;
        return false;
    }

    ezlist_lock(list);

    // check maximum number of allowed elements if set
    if (list->max > 0 && list->num >= list->max) {
        errno = ENOBUFS;
        ezlist_unlock(list);
        return false;
    }

    // adjust index
    if (index < 0)
        index = (list->num + index) + 1;  // -1 is same as addlast()
    if (index < 0 || index > list->num) {
        // out of bound
        ezlist_unlock(list);
        errno = ERANGE;
        return false;
    }

    // duplicate object
    void *dup_data = malloc(size);
    if (dup_data == NULL) {
        ezlist_unlock(list);
        errno = ENOMEM;
        return false;
    }
    memcpy(dup_data, data, size);

    // make new object list
    ezlist_obj_t *obj = (ezlist_obj_t *) malloc(sizeof(ezlist_obj_t));
    if (obj == NULL) {
        free(dup_data);
        ezlist_unlock(list);
        errno = ENOMEM;
        return false;
    }
    obj->data = dup_data;
    obj->size = size;
    obj->prev = NULL;
    obj->next = NULL;

    // make link
    if (index == 0) {
        // add at first
        obj->next = list->first;
        if (obj->next != NULL)
            obj->next->prev = obj;
        list->first = obj;
        if (list->last == NULL)
            list->last = obj;
    } else if (index == list->num) {
        // add after last
        obj->prev = list->last;
        if (obj->prev != NULL)
            obj->prev->next = obj;
        list->last = obj;
        if (list->first == NULL)
            list->first = obj;
    } else {
        // add at the middle of list
        ezlist_obj_t *tgt = get_obj(list, index);
        if (tgt == NULL) {
            // should not be happened.
            free(dup_data);
            free(obj);
            ezlist_unlock(list);
            errno = EAGAIN;
            return false;
        }

        // insert obj
        tgt->prev->next = obj;
        obj->prev = tgt->prev;
        obj->next = tgt;
        tgt->prev = obj;
    }

    list->datasum += size;
    list->num++;

    ezlist_unlock(list);

    return true;
}

/**
 * ezlist->getfirst(): Returns the first element in this list.
 *
 * @param list    ezlist_t container pointer.
 * @param size    if size is not NULL, element size will be stored.
 * @param newmem  whether or not to allocate memory for the element.
 *
 * @return a pointer of element, otherwise returns NULL.
 * @retval errno will be set in error condition.
 *  - ENOENT : List is empty.
 *  - ENOMEM : Memory allocation failure.
 *
 * @code
 *  size_t size;
 *  void *data = list->getfirst(list, &size, true);
 *  if (data != NULL) {
 *    (...omit...)
 *    free(data);
 *  }
 * @endcode
 */
void *ezlist_getfirst(ezlist_t *list, size_t *size, bool newmem) {
    return ezlist_getat(list, 0, size, newmem);
}

/**
 * ezlist->getlast(): Returns the last element in this list.
 *
 * @param list    ezlist_t container pointer.
 * @param size    if size is not NULL, element size will be stored.
 * @param newmem  whether or not to allocate memory for the element.
 *
 * @return a pointer of element, otherwise returns NULL.
 * @retval errno will be set in error condition.
 *        ENOENT : List is empty.
 *        ENOMEM : Memory allocation failure.
 */
void *ezlist_getlast(ezlist_t *list, size_t *size, bool newmem) {
    return ezlist_getat(list, -1, size, newmem);
}

/**
 * ezlist->getat(): Returns the element at the specified position in this
 * list.
 *
 * @param list    ezlist_t container pointer.
 * @param index   index at which the specified element is to be inserted
 * @param size    if size is not NULL, element size will be stored.
 * @param newmem  whether or not to allocate memory for the element.
 *
 * @return a pointer of element, otherwise returns NULL.
 * @retval errno
 * @retval errno will be set in error condition.
 *  -ERANGE : Index out of range.
 *  -ENOMEM : Memory allocation failure.
 *
 * @code
 *                     first           last
 *  Linked-list        [ A ]<=>[ B ]<=>[ C ]
 *  (positive index)     0       1       2
 *  (negative index)    -3      -2      -1
 * @endcode
 *
 * @note
 *  Negative index can be used for addressing a element from the end in this
 *  stack. For example, index -1 is same as getlast() and index 0 is same as
 *  getfirst();
 */
void *ezlist_getat(ezlist_t *list, int index, size_t *size, bool newmem) {
    return get_at(list, index, size, newmem, false);
}

/**
 * ezlist->popfirst(): Returns and remove the first element in this list.
 *
 * @param list  ezlist_t container pointer.
 * @param size  if size is not NULL, element size will be stored.
 *
 * @return a pointer of malloced element, otherwise returns NULL.
 * @retval errno will be set in error condition.
 *  -ENOENT : List is empty.
 *  -ENOMEM : Memory allocation failure.
 */
void *ezlist_popfirst(ezlist_t *list, size_t *size) {
    return ezlist_popat(list, 0, size);
}

/**
 * ezlist->getlast(): Returns and remove the last element in this list.
 *
 * @param list  ezlist_t container pointer.
 * @param size  if size is not NULL, element size will be stored.
 *
 * @return a pointer of malloced element, otherwise returns NULL.
 * @retval errno will be set in error condition.
 *  -ENOENT : List is empty.
 *  -ENOMEM : Memory allocation failure.
 */
void *ezlist_poplast(ezlist_t *list, size_t *size) {
    return ezlist_popat(list, -1, size);
}

/**
 * ezlist->popat(): Returns and remove the element at the specified
 * position in this list.
 *
 * @param list   ezlist_t container pointer.
 * @param index  index at which the specified element is to be inserted
 * @param size   if size is not NULL, element size will be stored.
 *
 * @return a pointer of malloced element, otherwise returns NULL.
 * @retval errno will be set in error condition.
 *  -ERANGE : Index out of range.
 *  -ENOMEM : Memory allocation failure.
 *
 * @code
 *                     first           last
 *  Linked-list        [ A ]<=>[ B ]<=>[ C ]
 *  (positive index)     0       1       2
 *  (negative index)    -3      -2      -1
 * @endcode
 *
 * @note
 *  Negative index can be used for addressing a element from the end in this
 *  stack. For example, index -1 is same as poplast() and index 0 is same as
 *  popfirst();
 */
void *ezlist_popat(ezlist_t *list, int index, size_t *size) {
    return get_at(list, index, size, true, true);
}

/**
 * ezlist->removefirst(): Removes the first element in this list.
 *
 * @param list  ezlist_t container pointer.
 *
 * @return a number of removed objects.
 * @retval errno will be set in error condition.
 *  -ENOENT : List is empty.
 */
bool ezlist_removefirst(ezlist_t *list) {
    return ezlist_removeat(list, 0);
}

/**
 * ezlist->removelast(): Removes the last element in this list.
 *
 * @param list  ezlist_t container pointer.
 *
 * @return a number of removed objects.
 * @retval errno will be set in error condition.
 *  -ENOENT : List is empty.
 */
bool ezlist_removelast(ezlist_t *list) {
    return ezlist_removeat(list, -1);
}

/**
 * ezlist->removeat(): Removes the element at the specified position in
 * this list.
 *
 * @param list   ezlist_t container pointer.
 * @param index  index at which the specified element is to be removed.
 *
 * @return a number of removed objects.
 * @retval errno will be set in error condition.
 *  -ERANGE : Index out of range.
 */
bool ezlist_removeat(ezlist_t *list, int index) {
    ezlist_lock(list);

    // get object pointer
    ezlist_obj_t *obj = get_obj(list, index);
    if (obj == NULL) {
        ezlist_unlock(list);
        return false;
    }

    bool ret = remove_obj(list, obj);

    ezlist_unlock(list);

    return ret;
}

/**
 * ezlist->getnext(): Get next element in this list.
 *
 * @param list    ezlist_t container pointer.
 * @param obj     found data will be stored in this structure
 * @param newmem  whether or not to allocate memory for the element.
 *
 * @return true if found otherwise returns false
 * @retval errno will be set in error condition.
 *  -ENOENT : No next element.
 *  -ENOMEM : Memory allocation failure.
 *
 * @note
 *  obj should be initialized with 0 by using memset() before first call.
 *  If newmem flag is true, user should de-allocate obj.name and obj.data
 *  resources.
 *
 * @code
 *  ezlist_t *list = ezlist();
 *  (...add data into list...)
 *
 *  ezlist_obj_t obj;
 *  memset((void*)&obj, 0, sizeof(obj)); // must be cleared before call
 *  list->lock(list);   // can be omitted in single thread model.
 *  while (list->getnext(list, &obj, false) == true) {
 *   printf("DATA=%s, SIZE=%zu\n", (char*)obj.data, obj.size);
 *  }
 *  list->unlock(list); // release lock.
 * @endcode
 */
bool ezlist_getnext(ezlist_t *list, ezlist_obj_t *obj, bool newmem) {
    if (obj == NULL)
        return false;

    ezlist_lock(list);

    ezlist_obj_t *cont = NULL;
    if (obj->size == 0)
        cont = list->first;
    else
        cont = obj->next;

    if (cont == NULL) {
        errno = ENOENT;
        ezlist_unlock(list);
        return false;
    }

    bool ret = false;
    while (cont != NULL) {
        if (newmem == true) {
            obj->data = malloc(cont->size);
            if (obj->data == NULL)
                break;

            memcpy(obj->data, cont->data, cont->size);
        } else {
            obj->data = cont->data;
        }
        obj->size = cont->size;
        obj->prev = cont->prev;
        obj->next = cont->next;

        ret = true;
        break;
    }

    ezlist_unlock(list);
    return ret;
}

/**
 * ezlist->size(): Returns the number of elements in this list.
 *
 * @param list  ezlist_t container pointer.
 *
 * @return the number of elements in this list.
 */
size_t ezlist_size(ezlist_t *list) {
    return list->num;
}

/**
 * ezlist->size(): Returns the sum of total element size.
 *
 * @param list  ezlist_t container pointer.
 *
 * @return the sum of total element size.
 */
size_t ezlist_datasize(ezlist_t *list) {
    return list->datasum;
}

/**
 * ezlist->reverse(): Reverse the order of elements.
 *
 * @param list  ezlist_t container pointer.
 */
void ezlist_reverse(ezlist_t *list) {
    ezlist_lock(list);
    ezlist_obj_t *obj;
    for (obj = list->first; obj;) {
        ezlist_obj_t *next = obj->next;
        obj->next = obj->prev;
        obj->prev = next;
        obj = next;
    }

    obj = list->first;
    list->first = list->last;
    list->last = obj;

    ezlist_unlock(list);
}

/**
 * ezlist->clear(): Removes all of the elements from this list.
 *
 * @param list  ezlist_t container pointer.
 */
void ezlist_clear(ezlist_t *list) {
    ezlist_lock(list);
    ezlist_obj_t *obj;
    for (obj = list->first; obj;) {
        ezlist_obj_t *next = obj->next;
        free(obj->data);
        free(obj);
        obj = next;
    }

    list->num = 0;
    list->datasum = 0;
    list->first = NULL;
    list->last = NULL;
    ezlist_unlock(list);
}

/**
 * ezlist->toarray(): Returns the serialized chunk containing all the
 * elements in this list.
 *
 * @param list  ezlist_t container pointer.
 * @param size  if size is not NULL, chunk size will be stored.
 *
 * @return a malloced pointer,
 *  otherwise(if there is no data to merge) returns NULL.
 * @retval errno will be set in error condition.
 *  -ENOENT : List is empty.
 *  -ENOMEM : Memory allocation failure.
 */
void *ezlist_toarray(ezlist_t *list, size_t *size) {
    if (list->num <= 0) {
        if (size != NULL)
            *size = 0;
        errno = ENOENT;
        return NULL;
    }

    ezlist_lock(list);

    void *chunk = malloc(list->datasum);
    if (chunk == NULL) {
        ezlist_unlock(list);
        errno = ENOMEM;
        return NULL;
    }
    void *dp = chunk;

    ezlist_obj_t *obj;
    for (obj = list->first; obj; obj = obj->next) {
        memcpy(dp, obj->data, obj->size);
        dp += obj->size;
    }
    ezlist_unlock(list);

    if (size != NULL)
        *size = list->datasum;
    return chunk;
}

/**
 * ezlist->tostring(): Returns a string representation of this list,
 * containing string representation of each element.
 *
 * @param list  ezlist_t container pointer.
 *
 * @return a malloced pointer,
 *  otherwise(if there is no data to merge) returns NULL.
 * @retval errno will be set in error condition.
 *  -ENOENT : List is empty.
 *  -ENOMEM : Memory allocation failure.
 *
 * @note
 *  Return string is always terminated by '\0'.
 */
char *ezlist_tostring(ezlist_t *list) {
    if (list->num <= 0) {
        errno = ENOENT;
        return NULL;
    }

    ezlist_lock(list);

    void *chunk = malloc(list->datasum + 1);
    if (chunk == NULL) {
        ezlist_unlock(list);
        errno = ENOMEM;
        return NULL;
    }
    void *dp = chunk;

    ezlist_obj_t *obj;
    for (obj = list->first; obj; obj = obj->next) {
        size_t size = obj->size;
        // do not copy tailing '\0'
        if (*(char *) (obj->data + (size - 1)) == '\0')
            size -= 1;
        memcpy(dp, obj->data, size);
        dp += size;
    }
    *((char *) dp) = '\0';
    ezlist_unlock(list);

    return (char *) chunk;
}

/**
 * ezlist->lock(): Enters critical section.
 *
 * @param list  ezlist_t container pointer.
 *
 * @note
 *  From user side, normally locking operation is only needed when traverse all
 *  elements using ezlist->getnext().
 */
void ezlist_lock(ezlist_t *list) {
    ezdev_pthreadMutexLock(list->mutex, WAIT_FOREVER);
}

/**
 * ezlist->unlock(): Leaves critical section.
 *
 * @param list  ezlist_t container pointer.
 */
void ezlist_unlock(ezlist_t *list) {
    ezdev_pthreadMutexUnlock(list->mutex);
}

/**
 * ezlist->free(): Free ezlist_t.
 *
 * @param list  ezlist_t container pointer.
 */
void ezlist_free(ezlist_t *list) {
    ezlist_clear(list);
    ezdev_pthreadMutexDestroy(list->mutex);

    free(list);
}

#ifndef _DOXYGEN_SKIP

static void *get_at(ezlist_t *list, int index, size_t *size, bool newmem,
bool remove) {
    ezlist_lock(list);

    // get object pointer
    ezlist_obj_t *obj = get_obj(list, index);
    if (obj == NULL) {
        ezlist_unlock(list);
        return false;
    }

    // copy data
    void *data;
    if (newmem == true) {
        data = malloc(obj->size);
        if (data == NULL) {
            ezlist_unlock(list);
            errno = ENOMEM;
            return false;
        }
        memcpy(data, obj->data, obj->size);
    } else {
        data = obj->data;
    }
    if (size != NULL)
        *size = obj->size;

    // remove if necessary
    if (remove == true) {
        if (remove_obj(list, obj) == false) {
            if (newmem == true)
                free(data);
            data = NULL;
        }
    }

    ezlist_unlock(list);

    return data;
}

static ezlist_obj_t *get_obj(ezlist_t *list, int index) {
    // index adjustment
    if (index < 0)
        index = list->num + index;
    if (index >= list->num) {
        errno = ERANGE;
        return NULL;
    }

    // detect faster scan direction
    bool backward;
    ezlist_obj_t *obj;
    int listidx;
    if (index < list->num / 2) {
        backward = false;
        obj = list->first;
        listidx = 0;
    } else {
        backward = true;
        obj = list->last;
        listidx = list->num - 1;
    }

    // find object
    while (obj != NULL) {
        if (listidx == index)
            return obj;

        if (backward == false) {
            obj = obj->next;
            listidx++;
        } else {
            obj = obj->prev;
            listidx--;
        }
    }

    // never reach here
    errno = ENOENT;
    return NULL;
}

static bool remove_obj(ezlist_t *list, ezlist_obj_t *obj) {
    if (obj == NULL)
        return false;

    // chain prev and next elements
    if (obj->prev == NULL)
        list->first = obj->next;
    else
        obj->prev->next = obj->next;
    if (obj->next == NULL)
        list->last = obj->prev;
    else
        obj->next->prev = obj->prev;

    // adjust counter
    list->datasum -= obj->size;
    list->num--;

    // release obj
    free(obj->data);
    free(obj);

    return true;
}

#endif /* _DOXYGEN_SKIP */
