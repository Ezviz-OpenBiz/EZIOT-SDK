/*
  Copyright (c) 2009-2017 Dave Gamble and bscJSON contributors

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifndef bsbscJSON__h
#define bsbscJSON__h

#ifdef __cplusplus
extern "C"
{
#endif

#if !defined(__WINDOWS__) && (defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32))
#define __WINDOWS__
#endif

#ifdef __WINDOWS__

  /* When compiling for windows, we specify a specific calling convention to avoid issues where we are being called from a project with a different default calling convention.  For windows you have 3 define options:

bsbscJSON_HIDE_SYMBOLS - Define this in the case where you don't want to ever dllexport symbols
bsbscJSON_EXPORT_SYMBOLS - Define this on library build when you want to dllexport symbols (default)
bsbscJSON_IMPORT_SYMBOLS - Define this if you want to dllimport symbol

For *nix builds that support visibility attribute, you can define similar behavior by

setting default visibility to hidden by adding
-fvisibility=hidden (for gcc)
or
-xldscope=hidden (for sun cc)
to CFLAGS

then using the bscJSON_API_VISIBILITY flag to "export" the same symbols the way bscJSON_EXPORT_SYMBOLS does

*/

#define bscJSON_CDECL __cdecl
#define bscJSON_STDCALL __stdcall

/* export symbols by default, this is necessary for copy pasting the C and header file */
#if !defined(bscJSON_HIDE_SYMBOLS) && !defined(bscJSON_IMPORT_SYMBOLS) && !defined(bscJSON_EXPORT_SYMBOLS)
#define bscJSON_EXPORT_SYMBOLS
#endif

#if defined(bscJSON_HIDE_SYMBOLS)
#define bscJSON_PUBLIC(type) type bscJSON_STDCALL
#elif defined(bscJSON_EXPORT_SYMBOLS)
#define bscJSON_PUBLIC(type) __declspec(dllexport) type bscJSON_STDCALL
#elif defined(bscJSON_IMPORT_SYMBOLS)
#define bscJSON_PUBLIC(type) __declspec(dllimport) type bscJSON_STDCALL
#endif
#else /* !__WINDOWS__ */
#define bscJSON_CDECL
#define bscJSON_STDCALL

#if (defined(__GNUC__) || defined(__SUNPRO_CC) || defined(__SUNPRO_C)) && defined(bscJSON_API_VISIBILITY)
#define bscJSON_PUBLIC(type) __attribute__((visibility("default"))) type
#else
#define bscJSON_PUBLIC(type) type
#endif
#endif

/* project version */
#define bscJSON_VERSION_MAJOR 1
#define bscJSON_VERSION_MINOR 7
#define bscJSON_VERSION_PATCH 12

#include <stddef.h>

/* bscJSON Types: */
#define bscJSON_Invalid (0)
#define bscJSON_False (1 << 0)
#define bscJSON_True (1 << 1)
#define bscJSON_NULL (1 << 2)
#define bscJSON_Number (1 << 3)
#define bscJSON_String (1 << 4)
#define bscJSON_Array (1 << 5)
#define bscJSON_Object (1 << 6)
#define bscJSON_Raw (1 << 7) /* raw json */

#define bscJSON_IsReference 256
#define bscJSON_StringIsConst 512

  /* The bscJSON structure: */
  typedef struct bscJSON
  {
    /* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
    struct bscJSON *next;
    struct bscJSON *prev;
    /* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */
    struct bscJSON *child;

    /* The type of the item, as above. */
    int type;

    /* The item's string, if type==bscJSON_String  and type == bscJSON_Raw */
    char *valuestring;
    /* writing to valueint is DEPRECATED, use bscJSON_SetNumberValue instead */
    int valueint;
    /* The item's number, if type==bscJSON_Number */
    double valuedouble;

    /* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
    char *string;
  } bscJSON;

  typedef struct bscJSON_Hooks
  {
    /* malloc/free are CDECL on Windows regardless of the default calling convention of the compiler, so ensure the hooks allow passing those functions directly. */
    void *(bscJSON_CDECL *malloc_fn)(size_t sz);
    void(bscJSON_CDECL *free_fn)(void *ptr);
  } bscJSON_Hooks;

  typedef int bscJSON_bool;

/* Limits how deeply nested arrays/objects can be before bscJSON rejects to parse them.
 * This is to prevent stack overflows. */
#ifndef bscJSON_NESTING_LIMIT
#define bscJSON_NESTING_LIMIT 1000
#endif

  /* returns the version of bscJSON as a string */
  bscJSON_PUBLIC(const char *) bscJSON_Version(void);

  /* Supply malloc, realloc and free functions to bscJSON */
  bscJSON_PUBLIC(void) bscJSON_InitHooks(bscJSON_Hooks *hooks);

  /* Memory Management: the caller is always responsible to free the results from all variants of bscJSON_Parse (with bscJSON_Delete) and bscJSON_Print (with stdlib free, bscJSON_Hooks.free_fn, or bscJSON_free as appropriate). The exception is bscJSON_PrintPreallocated, where the caller has full responsibility of the buffer. */
  /* Supply a block of JSON, and this returns a bscJSON object you can interrogate. */
  bscJSON_PUBLIC(bscJSON *) bscJSON_Parse(const char *value);
  /* ParseWithOpts allows you to require (and check) that the JSON is null terminated, and to retrieve the pointer to the final byte parsed. */
  /* If you supply a ptr in return_parse_end and parsing fails, then return_parse_end will contain a pointer to the error so will match bscJSON_GetErrorPtr(). */
  bscJSON_PUBLIC(bscJSON *) bscJSON_ParseWithOpts(const char *value, const char **return_parse_end, bscJSON_bool require_null_terminated);

  /* Render a bscJSON entity to text for transfer/storage. */
  bscJSON_PUBLIC(char *) bscJSON_Print(const bscJSON *item);
  /* Render a bscJSON entity to text for transfer/storage without any formatting. */
  bscJSON_PUBLIC(char *) bscJSON_PrintUnformatted(const bscJSON *item);
  /* Render a bscJSON entity to text using a buffered strategy. prebuffer is a guess at the final size. guessing well reduces reallocation. fmt=0 gives unformatted, =1 gives formatted */
  bscJSON_PUBLIC(char *) bscJSON_PrintBuffered(const bscJSON *item, int prebuffer, bscJSON_bool fmt);
  /* Render a bscJSON entity to text using a buffer already allocated in memory with given length. Returns 1 on success and 0 on failure. */
  /* NOTE: bscJSON is not always 100% accurate in estimating how much memory it will use, so to be safe allocate 5 bytes more than you actually need */
  bscJSON_PUBLIC(bscJSON_bool) bscJSON_PrintPreallocated(bscJSON *item, char *buffer, const int length, const bscJSON_bool format);
  /* Delete a bscJSON entity and all subentities. */
  bscJSON_PUBLIC(void) bscJSON_Delete(bscJSON *c);

  /* Returns the number of items in an array (or object). */
  bscJSON_PUBLIC(int) bscJSON_GetArraySize(const bscJSON *array);
  /* Retrieve item number "index" from array "array". Returns NULL if unsuccessful. */
  bscJSON_PUBLIC(bscJSON *) bscJSON_GetArrayItem(const bscJSON *array, int index);
  /* Get item "string" from object. Case insensitive. */
  bscJSON_PUBLIC(bscJSON *) bscJSON_GetObjectItem(const bscJSON *const object, const char *const string);
  bscJSON_PUBLIC(bscJSON *) bscJSON_GetObjectItemCaseSensitive(const bscJSON *const object, const char *const string);
  bscJSON_PUBLIC(bscJSON_bool) bscJSON_HasObjectItem(const bscJSON *object, const char *string);
  /* For analysing failed parses. This returns a pointer to the parse error. You'll probably need to look a few chars back to make sense of it. Defined when bscJSON_Parse() returns 0. 0 when bscJSON_Parse() succeeds. */
  bscJSON_PUBLIC(const char *) bscJSON_GetErrorPtr(void);

  /* Check if the item is a string and return its valuestring */
  bscJSON_PUBLIC(char *) bscJSON_GetStringValue(bscJSON *item);

  /* These functions check the type of an item */
  bscJSON_PUBLIC(bscJSON_bool) bscJSON_IsInvalid(const bscJSON *const item);
  bscJSON_PUBLIC(bscJSON_bool) bscJSON_IsFalse(const bscJSON *const item);
  bscJSON_PUBLIC(bscJSON_bool) bscJSON_IsTrue(const bscJSON *const item);
  bscJSON_PUBLIC(bscJSON_bool) bscJSON_IsBool(const bscJSON *const item);
  bscJSON_PUBLIC(bscJSON_bool) bscJSON_IsNull(const bscJSON *const item);
  bscJSON_PUBLIC(bscJSON_bool) bscJSON_IsNumber(const bscJSON *const item);
  bscJSON_PUBLIC(bscJSON_bool) bscJSON_IsString(const bscJSON *const item);
  bscJSON_PUBLIC(bscJSON_bool) bscJSON_IsArray(const bscJSON *const item);
  bscJSON_PUBLIC(bscJSON_bool) bscJSON_IsObject(const bscJSON *const item);
  bscJSON_PUBLIC(bscJSON_bool) bscJSON_IsRaw(const bscJSON *const item);

  /* These calls create a bscJSON item of the appropriate type. */
  bscJSON_PUBLIC(bscJSON *) bscJSON_CreateNull(void);
  bscJSON_PUBLIC(bscJSON *) bscJSON_CreateTrue(void);
  bscJSON_PUBLIC(bscJSON *) bscJSON_CreateFalse(void);
  bscJSON_PUBLIC(bscJSON *) bscJSON_CreateBool(bscJSON_bool boolean);
  bscJSON_PUBLIC(bscJSON *) bscJSON_CreateNumber(double num);
  bscJSON_PUBLIC(bscJSON *) bscJSON_CreateString(const char *string);
  /* raw json */
  bscJSON_PUBLIC(bscJSON *) bscJSON_CreateRaw(const char *raw);
  bscJSON_PUBLIC(bscJSON *) bscJSON_CreateArray(void);
  bscJSON_PUBLIC(bscJSON *) bscJSON_CreateObject(void);

  /* Create a string where valuestring references a string so
 * it will not be freed by bscJSON_Delete */
  bscJSON_PUBLIC(bscJSON *) bscJSON_CreateStringReference(const char *string);
  /* Create an object/arrray that only references it's elements so
 * they will not be freed by bscJSON_Delete */
  bscJSON_PUBLIC(bscJSON *) bscJSON_CreateObjectReference(const bscJSON *child);
  bscJSON_PUBLIC(bscJSON *) bscJSON_CreateArrayReference(const bscJSON *child);

  /* These utilities create an Array of count items. */
  bscJSON_PUBLIC(bscJSON *) bscJSON_CreateIntArray(const int *numbers, int count);
  bscJSON_PUBLIC(bscJSON *) bscJSON_CreateFloatArray(const float *numbers, int count);
  bscJSON_PUBLIC(bscJSON *) bscJSON_CreateDoubleArray(const double *numbers, int count);
  bscJSON_PUBLIC(bscJSON *) bscJSON_CreateStringArray(const char **strings, int count);

  /* Append item to the specified array/object. */
  bscJSON_PUBLIC(void) bscJSON_AddItemToArray(bscJSON *array, bscJSON *item);
  bscJSON_PUBLIC(void) bscJSON_AddItemToObject(bscJSON *object, const char *string, bscJSON *item);
  /* Use this when string is definitely const (i.e. a literal, or as good as), and will definitely survive the bscJSON object.
 * WARNING: When this function was used, make sure to always check that (item->type & bscJSON_StringIsConst) is zero before
 * writing to `item->string` */
  bscJSON_PUBLIC(void) bscJSON_AddItemToObjectCS(bscJSON *object, const char *string, bscJSON *item);
  /* Append reference to item to the specified array/object. Use this when you want to add an existing bscJSON to a new bscJSON, but don't want to corrupt your existing bscJSON. */
  bscJSON_PUBLIC(void) bscJSON_AddItemReferenceToArray(bscJSON *array, bscJSON *item);
  bscJSON_PUBLIC(void) bscJSON_AddItemReferenceToObject(bscJSON *object, const char *string, bscJSON *item);

  /* Remove/Detatch items from Arrays/Objects. */
  bscJSON_PUBLIC(bscJSON *) bscJSON_DetachItemViaPointer(bscJSON *parent, bscJSON *const item);
  bscJSON_PUBLIC(bscJSON *) bscJSON_DetachItemFromArray(bscJSON *array, int which);
  bscJSON_PUBLIC(void) bscJSON_DeleteItemFromArray(bscJSON *array, int which);
  bscJSON_PUBLIC(bscJSON *) bscJSON_DetachItemFromObject(bscJSON *object, const char *string);
  bscJSON_PUBLIC(bscJSON *) bscJSON_DetachItemFromObjectCaseSensitive(bscJSON *object, const char *string);
  bscJSON_PUBLIC(void) bscJSON_DeleteItemFromObject(bscJSON *object, const char *string);
  bscJSON_PUBLIC(void) bscJSON_DeleteItemFromObjectCaseSensitive(bscJSON *object, const char *string);

  /* Update array items. */
  bscJSON_PUBLIC(void) bscJSON_InsertItemInArray(bscJSON *array, int which, bscJSON *newitem); /* Shifts pre-existing items to the right. */
  bscJSON_PUBLIC(bscJSON_bool) bscJSON_ReplaceItemViaPointer(bscJSON *const parent, bscJSON *const item, bscJSON *replacement);
  bscJSON_PUBLIC(void) bscJSON_ReplaceItemInArray(bscJSON *array, int which, bscJSON *newitem);
  bscJSON_PUBLIC(void) bscJSON_ReplaceItemInObject(bscJSON *object, const char *string, bscJSON *newitem);
  bscJSON_PUBLIC(void) bscJSON_ReplaceItemInObjectCaseSensitive(bscJSON *object, const char *string, bscJSON *newitem);

  /* Duplicate a bscJSON item */
  bscJSON_PUBLIC(bscJSON *) bscJSON_Duplicate(const bscJSON *item, bscJSON_bool recurse);
  /* Duplicate will create a new, identical bscJSON item to the one you pass, in new memory that will
need to be released. With recurse!=0, it will duplicate any children connected to the item.
The item->next and ->prev pointers are always zero on return from Duplicate. */
  /* Recursively compare two bscJSON items for equality. If either a or b is NULL or invalid, they will be considered unequal.
 * case_sensitive determines if object keys are treated case sensitive (1) or case insensitive (0) */
  bscJSON_PUBLIC(bscJSON_bool) bscJSON_Compare(const bscJSON *const a, const bscJSON *const b, const bscJSON_bool case_sensitive);

  bscJSON_PUBLIC(void) bscJSON_Minify(char *json);

  /* Helper functions for creating and adding items to an object at the same time.
 * They return the added item or NULL on failure. */
  bscJSON_PUBLIC(bscJSON *) bscJSON_AddNullToObject(bscJSON *const object, const char *const name);
  bscJSON_PUBLIC(bscJSON *) bscJSON_AddTrueToObject(bscJSON *const object, const char *const name);
  bscJSON_PUBLIC(bscJSON *) bscJSON_AddFalseToObject(bscJSON *const object, const char *const name);
  bscJSON_PUBLIC(bscJSON *) bscJSON_AddBoolToObject(bscJSON *const object, const char *const name, const bscJSON_bool boolean);
  bscJSON_PUBLIC(bscJSON *) bscJSON_AddNumberToObject(bscJSON *const object, const char *const name, const double number);
  bscJSON_PUBLIC(bscJSON *) bscJSON_AddStringToObject(bscJSON *const object, const char *const name, const char *const string);
  bscJSON_PUBLIC(bscJSON *) bscJSON_AddRawToObject(bscJSON *const object, const char *const name, const char *const raw);
  bscJSON_PUBLIC(bscJSON *) bscJSON_AddObjectToObject(bscJSON *const object, const char *const name, bscJSON *const s);
  bscJSON_PUBLIC(bscJSON *) bscJSON_AddArrayToObject(bscJSON *const object, const char *const name, bscJSON *const s);
  bscJSON_PUBLIC(bscJSON *) bscJSON_AddObjectToArray(bscJSON *const object, const char *const name, bscJSON *const s);

/* When assigning an integer value, it needs to be propagated to valuedouble too. */
#define bscJSON_SetIntValue(object, number) ((object) ? (object)->valueint = (object)->valuedouble = (number) : (number))
  /* helper for the bscJSON_SetNumberValue macro */
  bscJSON_PUBLIC(double) bscJSON_SetNumberHelper(bscJSON *object, double number);
#define bscJSON_SetNumberValue(object, number) ((object != NULL) ? bscJSON_SetNumberHelper(object, (double)number) : (number))

/* Macro for iterating over an array or object */
#define bscJSON_ArrayForEach(element, array) for (element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)

  /* malloc/free objects using the malloc/free functions that have been set with bscJSON_InitHooks */
  bscJSON_PUBLIC(void *) bscJSON_malloc(size_t size);
  bscJSON_PUBLIC(void) bscJSON_free(void *object);

#ifdef __cplusplus
}
#endif

#endif
