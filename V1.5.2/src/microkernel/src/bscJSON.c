/*
  Copyright (c) 2009-2017 Dave Gamble and bsbscJSON contributors

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

/* bsbscJSON */
/* JSON parser in C. */

/* disable warnings about old C89 functions in MSVC */
#if !defined(_CRT_SECURE_NO_DEPRECATE) && defined(_MSC_VER)
#define _CRT_SECURE_NO_DEPRECATE
#endif

#ifdef __GNUC__
#pragma GCC visibility push(default)
#endif
#if defined(_MSC_VER)
#pragma warning(push)
/* disable warning about single line comments in system headers */
#pragma warning(disable : 4001)
#endif

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <float.h>
#include "ezconfig.h"
#include "mcuconfig.h"

#ifdef ENABLE_LOCALES
#include <locale.h>
#endif

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
#ifdef __GNUC__
#pragma GCC visibility pop
#endif

#include "bscJSON.h"

/* define our own boolean type */
#ifdef true
#undef true
#endif
#define true ((bscJSON_bool)1)

#ifdef false
#undef false
#endif
#define false ((bscJSON_bool)0) 

typedef struct
{
    const unsigned char *json; 
    size_t position;
} error;
static error global_error = {NULL, 0};

bscJSON_PUBLIC(const char *) bscJSON_GetErrorPtr(void)
{
    return (const char *)(global_error.json + global_error.position);
}

bscJSON_PUBLIC(char *) bscJSON_GetStringValue(bscJSON *item)
{
    if (!bscJSON_IsString(item))
    {
        return NULL;
    }

    return item->valuestring;
}

/* This is a safeguard to prevent copy-pasters from using incompatible C and header files */
#if (bscJSON_VERSION_MAJOR != 1) || (bscJSON_VERSION_MINOR != 7) || (bscJSON_VERSION_PATCH != 12)
#error bscJSON.h and bscJSON.c have different versions. Make sure that both have the same.
#endif

bscJSON_PUBLIC(const char *) bscJSON_Version(void)
{
    static char version[15];
    sprintf(version, "%i.%i.%i", bscJSON_VERSION_MAJOR, bscJSON_VERSION_MINOR, bscJSON_VERSION_PATCH);

    return version;
}

/* Case insensitive string comparison, doesn't consider two NULL pointers equal though */
static int case_insensitive_strcmp(const unsigned char *string1, const unsigned char *string2)
{
    if ((string1 == NULL) || (string2 == NULL))
    {
        return 1;
    }

    if (string1 == string2)
    {
        return 0;
    }

    for (; tolower(*string1) == tolower(*string2); (void)string1++, string2++)
    {
        if (*string1 == '\0')
        {
            return 0;
        }
    }

    return tolower(*string1) - tolower(*string2);
}

typedef struct internal_hooks
{
    void *(bscJSON_CDECL *allocate)(size_t size);
    void(bscJSON_CDECL *deallocate)(void *pointer);
    void *(bscJSON_CDECL *reallocate)(void *pointer, size_t size);
} internal_hooks;

#if defined(_MSC_VER)
/* work around MSVC error C2322: '...' address of dillimport '...' is not static */
static void *bscJSON_CDECL internal_malloc(size_t size)
{
    return malloc(size);
}
static void bscJSON_CDECL internal_free(void *pointer)
{
    free(pointer);
}
static void *bscJSON_CDECL internal_realloc(void *pointer, size_t size)
{
    return realloc(pointer, size);
}
#else
#define internal_malloc malloc
#define internal_free free
#define internal_realloc realloc
#endif

/* strlen of character literals resolved at compile time */
#define static_strlen(string_literal) (sizeof(string_literal) - sizeof(""))

static internal_hooks global_hooks = {internal_malloc, internal_free, internal_realloc};

static unsigned char *bscJSON_strdup(const unsigned char *string, const internal_hooks *const hooks)
{
    size_t length = 0;
    unsigned char *copy = NULL;

    if (string == NULL)
    {
        return NULL;
    }

    length = strlen((const char *)string) + sizeof("");
    copy = (unsigned char *)hooks->allocate(length);
    if (copy == NULL)
    {
        return NULL;
    }
    memcpy(copy, string, length);

    return copy;
}

bscJSON_PUBLIC(void) bscJSON_InitHooks(bscJSON_Hooks *hooks)
{
    if (hooks == NULL)
    {
        /* Reset hooks */
        global_hooks.allocate = malloc;
        global_hooks.deallocate = free;
        global_hooks.reallocate = realloc;
        return;
    }

    global_hooks.allocate = malloc;
    if (hooks->malloc_fn != NULL)
    {
        global_hooks.allocate = hooks->malloc_fn;
    }

    global_hooks.deallocate = free;
    if (hooks->free_fn != NULL)
    {
        global_hooks.deallocate = hooks->free_fn;
    }

    /* use realloc only if both free and malloc are used */
    global_hooks.reallocate = NULL;
    if ((global_hooks.allocate == malloc) && (global_hooks.deallocate == free))
    {
        global_hooks.reallocate = realloc;
    }
}

/* Internal constructor. */
static bscJSON *bscJSON_New_Item(const internal_hooks *const hooks)
{
    bscJSON *node = (bscJSON *)hooks->allocate(sizeof(bscJSON));
    if (node)
    {
        memset(node, '\0', sizeof(bscJSON));
    }

    return node;
}

/* Delete a bscJSON structure. */
bscJSON_PUBLIC(void) bscJSON_Delete(bscJSON *item)
{
    bscJSON *next = NULL;
    while (item != NULL)
    {
        next = item->next;
        if (!(item->type & bscJSON_IsReference) && (item->child != NULL))
        {
            bscJSON_Delete(item->child);
        }
        if (!(item->type & bscJSON_IsReference) && (item->valuestring != NULL))
        {
            global_hooks.deallocate(item->valuestring);
        }
        if (!(item->type & bscJSON_StringIsConst) && (item->string != NULL))
        {
            global_hooks.deallocate(item->string);
        }
        global_hooks.deallocate(item);
        item = next;
    }
}

/* get the decimal point character of the current locale */
static unsigned char get_decimal_point(void)
{
#ifdef ENABLE_LOCALES
    struct lconv *lconv = localeconv();
    return (unsigned char)lconv->decimal_point[0];
#else
    return '.';
#endif
}

typedef struct
{
    const unsigned char *content;
    size_t length;
    size_t offset;
    size_t depth; /* How deeply nested (in arrays/objects) is the input at the current offset. */
    internal_hooks hooks;
} parse_buffer;

/* check if the given size is left to read in a given parse buffer (starting with 1) */
#define can_read(buffer, size) ((buffer != NULL) && (((buffer)->offset + size) <= (buffer)->length))
/* check if the buffer can be accessed at the given index (starting with 0) */
#define can_access_at_index(buffer, index) ((buffer != NULL) && (((buffer)->offset + index) < (buffer)->length))
#define cannot_access_at_index(buffer, index) (!can_access_at_index(buffer, index))
/* get a pointer to the buffer at the position */
#define buffer_at_offset(buffer) ((buffer)->content + (buffer)->offset)

/* Parse the input text to generate a number, and populate the result into item. */
static bscJSON_bool parse_number(bscJSON *const item, parse_buffer *const input_buffer)
{
    double number = 0;
    unsigned char *after_end = NULL;
    unsigned char number_c_string[64];
    unsigned char decimal_point = get_decimal_point();
    size_t i = 0;

    if ((input_buffer == NULL) || (input_buffer->content == NULL))
    {
        return false;
    }

    /* copy the number into a temporary buffer and replace '.' with the decimal point
     * of the current locale (for strtod)
     * This also takes care of '\0' not necessarily being available for marking the end of the input */
    for (i = 0; (i < (sizeof(number_c_string) - 1)) && can_access_at_index(input_buffer, i); i++)
    {
        switch (buffer_at_offset(input_buffer)[i])
        {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '+':
        case '-':
        case 'e':
        case 'E':
            number_c_string[i] = buffer_at_offset(input_buffer)[i];
            break;

        case '.':
            number_c_string[i] = decimal_point;
            break;

        default:
            goto loop_end;
        }
    }
loop_end:
    number_c_string[i] = '\0';

    number = strtod((const char *)number_c_string, (char **)&after_end);
    if (number_c_string == after_end)
    {
        return false; /* parse_error */
    }

    item->valuedouble = number;

    /* use saturation in case of overflow */
    if (number >= INT_MAX)
    {
        item->valueint = INT_MAX;
    }
    else if (number <= (double)INT_MIN)
    {
        item->valueint = INT_MIN;
    }
    else
    {
        item->valueint = (int)number;
    }

    item->type = bscJSON_Number;

    input_buffer->offset += (size_t)(after_end - number_c_string);
    return true;
}

/* don't ask me, but the original bscJSON_SetNumberValue returns an integer or double */
bscJSON_PUBLIC(double) bscJSON_SetNumberHelper(bscJSON *object, double number)
{
    if (number >= INT_MAX)
    {
        object->valueint = INT_MAX;
    }
    else if (number <= (double)INT_MIN)
    {
        object->valueint = INT_MIN;
    }
    else
    {
        object->valueint = (int)number;
    }

    return object->valuedouble = number;
}

typedef struct
{
    unsigned char *buffer;
    size_t length;
    size_t offset;
    size_t depth; /* current nesting depth (for formatted printing) */
    bscJSON_bool noalloc;
    bscJSON_bool format; /* is this print a formatted print */
    internal_hooks hooks;
} printbuffer;

/* realloc printbuffer if necessary to have at least "needed" bytes more */
static unsigned char *ensure(printbuffer *const p, size_t needed)
{
    unsigned char *newbuffer = NULL;
    size_t newsize = 0;

    if ((p == NULL) || (p->buffer == NULL))
    {
        return NULL;
    }

    if ((p->length > 0) && (p->offset >= p->length))
    {
        /* make sure that offset is valid */
        return NULL;
    }

    if (needed > INT_MAX)
    {
        /* sizes bigger than INT_MAX are currently not supported */
        return NULL;
    }

    needed += p->offset + 1;
    if (needed <= p->length)
    {
        return p->buffer + p->offset;
    }

    if (p->noalloc)
    {
        return NULL;
    }

    /* calculate new buffer size */
    if (needed > (INT_MAX / 2))
    {
        /* overflow of int, use INT_MAX if possible */
        if (needed <= INT_MAX)
        {
            newsize = INT_MAX;
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        newsize = needed * 2;
    }

    if (p->hooks.reallocate != NULL)
    {
        /* reallocate with realloc if available */
        newbuffer = (unsigned char *)p->hooks.reallocate(p->buffer, newsize);
        if (newbuffer == NULL)
        {
            p->hooks.deallocate(p->buffer);
            p->length = 0;
            p->buffer = NULL;

            return NULL;
        }
    }
    else
    {
        /* otherwise reallocate manually */
        newbuffer = (unsigned char *)p->hooks.allocate(newsize);
        if (!newbuffer)
        {
            p->hooks.deallocate(p->buffer);
            p->length = 0;
            p->buffer = NULL;

            return NULL;
        }
        if (newbuffer)
        {
            memcpy(newbuffer, p->buffer, p->offset + 1);
        }
        p->hooks.deallocate(p->buffer);
    }
    p->length = newsize;
    p->buffer = newbuffer;

    return newbuffer + p->offset;
}

/* calculate the new length of the string in a printbuffer and update the offset */
static void update_offset(printbuffer *const buffer)
{
    const unsigned char *buffer_pointer = NULL;
    if ((buffer == NULL) || (buffer->buffer == NULL))
    {
        return;
    }
    buffer_pointer = buffer->buffer + buffer->offset;

    buffer->offset += strlen((const char *)buffer_pointer);
}

/* Render the number nicely from the given item into a string. */
static bscJSON_bool print_number(const bscJSON *const item, printbuffer *const output_buffer)
{
    unsigned char *output_pointer = NULL;
    double d = item->valuedouble;
    int length = 0;
    size_t i = 0;
    unsigned char number_buffer[26]; /* temporary buffer to print the number into */
    unsigned char decimal_point = get_decimal_point();
    double test;

    if (output_buffer == NULL)
    {
        return false;
    }

    /* This checks for NaN and Infinity */
    if ((d * 0) != 0)
    {
        length = sprintf((char *)number_buffer, "null");
    }
    else
    {
        /* Try 15 decimal places of precision to avoid nonsignificant nonzero digits */
        if (fabs(((double)item->valueint)-d)<=DBL_EPSILON && d<=INT_MAX && d>=INT_MIN)
        {
            length = sprintf((char *)number_buffer, "%d", (int)d);
        }
        else
        {
            length = sprintf((char *)number_buffer, "%1.15f", d);
        }
        if (0 >= length)
        {
            length = strlen(number_buffer);
        }

// #else
        // length = sprintf((char *)number_buffer, "%1.15g", d);
        // /* Check whether the original double can be recovered */
        // if ((sscanf((char *)number_buffer, "%lg", &test) != 1) || ((double)test != d))
        // {
        //     /* If not, print with 17 decimal places of precision */
        //     length = sprintf((char *)number_buffer, "%1.17g", d);
        // }
// #endif
    }

    /* sprintf failed or buffer overrun occurred */
    if ((length < 0) || (length > (int)(sizeof(number_buffer) - 1)))
    {
        return false;
    }

    /* reserve appropriate space in the output */
    output_pointer = ensure(output_buffer, (size_t)length + sizeof(""));
    if (output_pointer == NULL)
    {
        return false;
    }

    /* copy the printed number to the output and replace locale
     * dependent decimal point with '.' */
    for (i = 0; i < ((size_t)length); i++)
    {
        if (number_buffer[i] == decimal_point)
        {
            output_pointer[i] = '.';
            continue;
        }

        output_pointer[i] = number_buffer[i];
    }
    output_pointer[i] = '\0';

    output_buffer->offset += (size_t)length;

    return true;
}

/* parse 4 digit hexadecimal number */
static unsigned parse_hex4(const unsigned char *const input)
{
    unsigned int h = 0;
    size_t i = 0;

    for (i = 0; i < 4; i++)
    {
        /* parse digit */
        if ((input[i] >= '0') && (input[i] <= '9'))
        {
            h += (unsigned int)input[i] - '0';
        }
        else if ((input[i] >= 'A') && (input[i] <= 'F'))
        {
            h += (unsigned int)10 + input[i] - 'A';
        }
        else if ((input[i] >= 'a') && (input[i] <= 'f'))
        {
            h += (unsigned int)10 + input[i] - 'a';
        }
        else /* invalid */
        {
            return 0;
        }

        if (i < 3)
        {
            /* shift left to make place for the next nibble */
            h = h << 4;
        }
    }

    return h;
}

/* converts a UTF-16 literal to UTF-8
 * A literal can be one or two sequences of the form \uXXXX */
static unsigned char utf16_literal_to_utf8(const unsigned char *const input_pointer, const unsigned char *const input_end, unsigned char **output_pointer)
{
    long unsigned int codepoint = 0;
    unsigned int first_code = 0;
    const unsigned char *first_sequence = input_pointer;
    unsigned char utf8_length = 0;
    unsigned char utf8_position = 0;
    unsigned char sequence_length = 0;
    unsigned char first_byte_mark = 0;

    if ((input_end - first_sequence) < 6)
    {
        /* input ends unexpectedly */
        goto fail;
    }

    /* get the first utf16 sequence */
    first_code = parse_hex4(first_sequence + 2);

    /* check that the code is valid */
    if (((first_code >= 0xDC00) && (first_code <= 0xDFFF)))
    {
        goto fail;
    }

    /* UTF16 surrogate pair */
    if ((first_code >= 0xD800) && (first_code <= 0xDBFF))
    {
        const unsigned char *second_sequence = first_sequence + 6;
        unsigned int second_code = 0;
        sequence_length = 12; /* \uXXXX\uXXXX */

        if ((input_end - second_sequence) < 6)
        {
            /* input ends unexpectedly */
            goto fail;
        }

        if ((second_sequence[0] != '\\') || (second_sequence[1] != 'u'))
        {
            /* missing second half of the surrogate pair */
            goto fail;
        }

        /* get the second utf16 sequence */
        second_code = parse_hex4(second_sequence + 2);
        /* check that the code is valid */
        if ((second_code < 0xDC00) || (second_code > 0xDFFF))
        {
            /* invalid second half of the surrogate pair */
            goto fail;
        }

        /* calculate the unicode codepoint from the surrogate pair */
        codepoint = 0x10000 + (((first_code & 0x3FF) << 10) | (second_code & 0x3FF));
    }
    else
    {
        sequence_length = 6; /* \uXXXX */
        codepoint = first_code;
    }

    /* encode as UTF-8
     * takes at maximum 4 bytes to encode:
     * 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
    if (codepoint < 0x80)
    {
        /* normal ascii, encoding 0xxxxxxx */
        utf8_length = 1;
    }
    else if (codepoint < 0x800)
    {
        /* two bytes, encoding 110xxxxx 10xxxxxx */
        utf8_length = 2;
        first_byte_mark = 0xC0; /* 11000000 */
    }
    else if (codepoint < 0x10000)
    {
        /* three bytes, encoding 1110xxxx 10xxxxxx 10xxxxxx */
        utf8_length = 3;
        first_byte_mark = 0xE0; /* 11100000 */
    }
    else if (codepoint <= 0x10FFFF)
    {
        /* four bytes, encoding 1110xxxx 10xxxxxx 10xxxxxx 10xxxxxx */
        utf8_length = 4;
        first_byte_mark = 0xF0; /* 11110000 */
    }
    else
    {
        /* invalid unicode codepoint */
        goto fail;
    }

    /* encode as utf8 */
    for (utf8_position = (unsigned char)(utf8_length - 1); utf8_position > 0; utf8_position--)
    {
        /* 10xxxxxx */
        (*output_pointer)[utf8_position] = (unsigned char)((codepoint | 0x80) & 0xBF);
        codepoint >>= 6;
    }
    /* encode first byte */
    if (utf8_length > 1)
    {
        (*output_pointer)[0] = (unsigned char)((codepoint | first_byte_mark) & 0xFF);
    }
    else
    {
        (*output_pointer)[0] = (unsigned char)(codepoint & 0x7F);
    }

    *output_pointer += utf8_length;

    return sequence_length;

fail:
    return 0;
}

/* Parse the input text into an unescaped cinput, and populate item. */
static bscJSON_bool parse_string(bscJSON *const item, parse_buffer *const input_buffer)
{
    const unsigned char *input_pointer = buffer_at_offset(input_buffer) + 1;
    const unsigned char *input_end = buffer_at_offset(input_buffer) + 1;
    unsigned char *output_pointer = NULL;
    unsigned char *output = NULL;

    /* not a string */
    if (buffer_at_offset(input_buffer)[0] != '\"')
    {
        goto fail;
    }

    {
        /* calculate approximate size of the output (overestimate) */
        size_t allocation_length = 0;
        size_t skipped_bytes = 0;
        while (((size_t)(input_end - input_buffer->content) < input_buffer->length) && (*input_end != '\"'))
        {
            /* is escape sequence */
            if (input_end[0] == '\\')
            {
                if ((size_t)(input_end + 1 - input_buffer->content) >= input_buffer->length)
                {
                    /* prevent buffer overflow when last input character is a backslash */
                    goto fail;
                }
                skipped_bytes++;
                input_end++;
            }
            input_end++;
        }
        if (((size_t)(input_end - input_buffer->content) >= input_buffer->length) || (*input_end != '\"'))
        {
            goto fail; /* string ended unexpectedly */
        }

        /* This is at most how much we need for the output */
        allocation_length = (size_t)(input_end - buffer_at_offset(input_buffer)) - skipped_bytes;
        output = (unsigned char *)input_buffer->hooks.allocate(allocation_length + sizeof(""));
        if (output == NULL)
        {
            goto fail; /* allocation failure */
        }
    }

    output_pointer = output;
    /* loop through the string literal */
    while (input_pointer < input_end)
    {
        if (*input_pointer != '\\')
        {
            *output_pointer++ = *input_pointer++;
        }
        /* escape sequence */
        else
        {
            unsigned char sequence_length = 2;
            if ((input_end - input_pointer) < 1)
            {
                goto fail;
            }

            switch (input_pointer[1])
            {
            case 'b':
                *output_pointer++ = '\b';
                break;
            case 'f':
                *output_pointer++ = '\f';
                break;
            case 'n':
                *output_pointer++ = '\n';
                break;
            case 'r':
                *output_pointer++ = '\r';
                break;
            case 't':
                *output_pointer++ = '\t';
                break;
            case '\"':
            case '\\':
            case '/':
                *output_pointer++ = input_pointer[1];
                break;

            /* UTF-16 literal */
            case 'u':
                sequence_length = utf16_literal_to_utf8(input_pointer, input_end, &output_pointer);
                if (sequence_length == 0)
                {
                    /* failed to convert UTF16-literal to UTF-8 */
                    goto fail;
                }
                break;

            default:
                goto fail;
            }
            input_pointer += sequence_length;
        }
    }

    /* zero terminate the output */
    *output_pointer = '\0';

    item->type = bscJSON_String;
    item->valuestring = (char *)output;

    input_buffer->offset = (size_t)(input_end - input_buffer->content);
    input_buffer->offset++;

    return true;

fail:
    if (output != NULL)
    {
        input_buffer->hooks.deallocate(output);
    }

    if (input_pointer != NULL)
    {
        input_buffer->offset = (size_t)(input_pointer - input_buffer->content);
    }

    return false;
}

/* Render the cstring provided to an escaped version that can be printed. */
static bscJSON_bool print_string_ptr(const unsigned char *const input, printbuffer *const output_buffer)
{
    const unsigned char *input_pointer = NULL;
    unsigned char *output = NULL;
    unsigned char *output_pointer = NULL;
    size_t output_length = 0;
    /* numbers of additional characters needed for escaping */
    size_t escape_characters = 0;

    if (output_buffer == NULL)
    {
        return false;
    }

    /* empty string */
    if (input == NULL)
    {
        output = ensure(output_buffer, sizeof("\"\""));
        if (output == NULL)
        {
            return false;
        }
        strcpy((char *)output, "\"\"");

        return true;
    }

    /* set "flag" to 1 if something needs to be escaped */
    for (input_pointer = input; *input_pointer; input_pointer++)
    {
        switch (*input_pointer)
        {
        case '\"':
        case '\\':
        case '\b':
        case '\f':
        case '\n':
        case '\r':
        case '\t':
            /* one character escape sequence */
            escape_characters++;
            break;
        default:
            if (*input_pointer < 32)
            {
                /* UTF-16 escape sequence uXXXX */
                escape_characters += 5;
            }
            break;
        }
    }
    output_length = (size_t)(input_pointer - input) + escape_characters;

    output = ensure(output_buffer, output_length + sizeof("\"\""));
    if (output == NULL)
    {
        return false;
    }

    /* no characters have to be escaped */
    if (escape_characters == 0)
    {
        output[0] = '\"';
        memcpy(output + 1, input, output_length);
        output[output_length + 1] = '\"';
        output[output_length + 2] = '\0';

        return true;
    }

    output[0] = '\"';
    output_pointer = output + 1;
    /* copy the string */
    for (input_pointer = input; *input_pointer != '\0'; (void)input_pointer++, output_pointer++)
    {
        if ((*input_pointer > 31) && (*input_pointer != '\"') && (*input_pointer != '\\'))
        {
            /* normal character, copy */
            *output_pointer = *input_pointer;
        }
        else
        {
            /* character needs to be escaped */
            *output_pointer++ = '\\';
            switch (*input_pointer)
            {
            case '\\':
                *output_pointer = '\\';
                break;
            case '\"':
                *output_pointer = '\"';
                break;
            case '\b':
                *output_pointer = 'b';
                break;
            case '\f':
                *output_pointer = 'f';
                break;
            case '\n':
                *output_pointer = 'n';
                break;
            case '\r':
                *output_pointer = 'r';
                break;
            case '\t':
                *output_pointer = 't';
                break;
            default:
                /* escape and print as unicode codepoint */
                sprintf((char *)output_pointer, "u%04x", *input_pointer);
                output_pointer += 4;
                break;
            }
        }
    }
    output[output_length + 1] = '\"';
    output[output_length + 2] = '\0';

    return true;
}

/* Invoke print_string_ptr (which is useful) on an item. */
static bscJSON_bool print_string(const bscJSON *const item, printbuffer *const p)
{
    return print_string_ptr((unsigned char *)item->valuestring, p);
}

/* Predeclare these prototypes. */
static bscJSON_bool parse_value(bscJSON *const item, parse_buffer *const input_buffer);
static bscJSON_bool print_value(const bscJSON *const item, printbuffer *const output_buffer);
static bscJSON_bool parse_array(bscJSON *const item, parse_buffer *const input_buffer);
static bscJSON_bool print_array(const bscJSON *const item, printbuffer *const output_buffer);
static bscJSON_bool parse_object(bscJSON *const item, parse_buffer *const input_buffer);
static bscJSON_bool print_object(const bscJSON *const item, printbuffer *const output_buffer);

/* Utility to jump whitespace and cr/lf */
static parse_buffer *buffer_skip_whitespace(parse_buffer *const buffer)
{
    if ((buffer == NULL) || (buffer->content == NULL))
    {
        return NULL;
    }

    while (can_access_at_index(buffer, 0) && (buffer_at_offset(buffer)[0] <= 32))
    {
        buffer->offset++;
    }

    if (buffer->offset == buffer->length)
    {
        buffer->offset--;
    }

    return buffer;
}

/* skip the UTF-8 BOM (byte order mark) if it is at the beginning of a buffer */
static parse_buffer *skip_utf8_bom(parse_buffer *const buffer)
{
    if ((buffer == NULL) || (buffer->content == NULL) || (buffer->offset != 0))
    {
        return NULL;
    }

    if (can_access_at_index(buffer, 4) && (strncmp((const char *)buffer_at_offset(buffer), "\xEF\xBB\xBF", 3) == 0))
    {
        buffer->offset += 3;
    }

    return buffer;
}

/* Parse an object - create a new root, and populate. */
bscJSON_PUBLIC(bscJSON *) bscJSON_ParseWithOpts(const char *value, const char **return_parse_end, bscJSON_bool require_null_terminated)
{
    parse_buffer buffer = {0, 0, 0, 0, {0, 0, 0}};
    bscJSON *item = NULL;

    /* reset error position */
    global_error.json = NULL;
    global_error.position = 0;

    if (value == NULL)
    {
        goto fail;
    }

    buffer.content = (const unsigned char *)value;
    buffer.length = strlen((const char *)value) + sizeof("");
    buffer.offset = 0;
    buffer.hooks = global_hooks;

    item = bscJSON_New_Item(&global_hooks);
    if (item == NULL) /* memory fail */
    {
        goto fail;
    }

    if (!parse_value(item, buffer_skip_whitespace(skip_utf8_bom(&buffer))))
    {
        /* parse failure. ep is set. */
        goto fail;
    }

    /* if we require null-terminated JSON without appended garbage, skip and then check for a null terminator */
    if (require_null_terminated)
    {
        buffer_skip_whitespace(&buffer);
        if ((buffer.offset >= buffer.length) || buffer_at_offset(&buffer)[0] != '\0')
        {
            goto fail;
        }
    }
    if (return_parse_end)
    {
        *return_parse_end = (const char *)buffer_at_offset(&buffer);
    }

    return item;

fail:
    if (item != NULL)
    {
        bscJSON_Delete(item);
    }

    if (value != NULL)
    {
        error local_error;
        local_error.json = (const unsigned char *)value;
        local_error.position = 0;

        if (buffer.offset < buffer.length)
        {
            local_error.position = buffer.offset;
        }
        else if (buffer.length > 0)
        {
            local_error.position = buffer.length - 1;
        }

        if (return_parse_end != NULL)
        {
            *return_parse_end = (const char *)local_error.json + local_error.position;
        }

        global_error = local_error;
    }

    return NULL;
}

/* Default options for bscJSON_Parse */
bscJSON_PUBLIC(bscJSON *) bscJSON_Parse(const char *value)
{
    return bscJSON_ParseWithOpts(value, 0, 0);
}

#define bscJSON_min(a, b) ((a < b) ? a : b)

static unsigned char *print(const bscJSON *const item, bscJSON_bool format, const internal_hooks *const hooks)
{
    static const size_t default_buffer_size = 256;
    printbuffer buffer[1];
    unsigned char *printed = NULL;

    memset(buffer, 0, sizeof(buffer));

    /* create buffer */
    buffer->buffer = (unsigned char *)hooks->allocate(default_buffer_size);
    buffer->length = default_buffer_size;
    buffer->format = format;
    buffer->hooks = *hooks;
    if (buffer->buffer == NULL)
    {
        goto fail;
    }

    /* print the value */
    if (!print_value(item, buffer))
    {
        goto fail;
    }
    update_offset(buffer);

    /* check if reallocate is available */
    if (hooks->reallocate != NULL)
    {
        printed = (unsigned char *)hooks->reallocate(buffer->buffer, buffer->offset + 1);
        if (printed == NULL)
        {
            goto fail;
        }
        buffer->buffer = NULL;
    }
    else /* otherwise copy the JSON over to a new buffer */
    {
        printed = (unsigned char *)hooks->allocate(buffer->offset + 1);
        if (printed == NULL)
        {
            goto fail;
        }
        memcpy(printed, buffer->buffer, bscJSON_min(buffer->length, buffer->offset + 1));
        printed[buffer->offset] = '\0'; /* just to be sure */

        /* free the buffer */
        hooks->deallocate(buffer->buffer);
    }

    return printed;

fail:
    if (buffer->buffer != NULL)
    {
        hooks->deallocate(buffer->buffer);
    }

    if (printed != NULL)
    {
        hooks->deallocate(printed);
    }

    return NULL;
}

/* Render a bscJSON item/entity/structure to text. */
bscJSON_PUBLIC(char *) bscJSON_Print(const bscJSON *item)
{
    return (char *)print(item, true, &global_hooks);
}

bscJSON_PUBLIC(char *) bscJSON_PrintUnformatted(const bscJSON *item)
{
    return (char *)print(item, false, &global_hooks);
}

bscJSON_PUBLIC(char *) bscJSON_PrintBuffered(const bscJSON *item, int prebuffer, bscJSON_bool fmt)
{
    printbuffer p = {0, 0, 0, 0, 0, 0, {0, 0, 0}};

    if (prebuffer < 0)
    {
        return NULL;
    }

    p.buffer = (unsigned char *)global_hooks.allocate((size_t)prebuffer);
    if (!p.buffer)
    {
        return NULL;
    }

    p.length = (size_t)prebuffer;
    p.offset = 0;
    p.noalloc = false;
    p.format = fmt;
    p.hooks = global_hooks;

    if (!print_value(item, &p))
    {
        global_hooks.deallocate(p.buffer);
        return NULL;
    }

    return (char *)p.buffer;
}

bscJSON_PUBLIC(bscJSON_bool) bscJSON_PrintPreallocated(bscJSON *item, char *buf, const int len, const bscJSON_bool fmt)
{
    printbuffer p = {0, 0, 0, 0, 0, 0, {0, 0, 0}};

    if ((len < 0) || (buf == NULL))
    {
        return false;
    }

    p.buffer = (unsigned char *)buf;
    p.length = (size_t)len;
    p.offset = 0;
    p.noalloc = true;
    p.format = fmt;
    p.hooks = global_hooks;

    return print_value(item, &p);
}

/* Parser core - when encountering text, process appropriately. */
static bscJSON_bool parse_value(bscJSON *const item, parse_buffer *const input_buffer)
{
    if ((input_buffer == NULL) || (input_buffer->content == NULL))
    {
        return false; /* no input */
    }

    /* parse the different types of values */
    /* null */
    if (can_read(input_buffer, 4) && (strncmp((const char *)buffer_at_offset(input_buffer), "null", 4) == 0))
    {
        item->type = bscJSON_NULL;
        input_buffer->offset += 4;
        return true;
    }
    /* false */
    if (can_read(input_buffer, 5) && (strncmp((const char *)buffer_at_offset(input_buffer), "false", 5) == 0))
    {
        item->type = bscJSON_False;
        input_buffer->offset += 5;
        return true;
    }
    /* true */
    if (can_read(input_buffer, 4) && (strncmp((const char *)buffer_at_offset(input_buffer), "true", 4) == 0))
    {
        item->type = bscJSON_True;
        item->valueint = 1;
        input_buffer->offset += 4;
        return true;
    }
    /* string */
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '\"'))
    {
        return parse_string(item, input_buffer);
    }
    /* number */
    if (can_access_at_index(input_buffer, 0) && ((buffer_at_offset(input_buffer)[0] == '-') || ((buffer_at_offset(input_buffer)[0] >= '0') && (buffer_at_offset(input_buffer)[0] <= '9'))))
    {
        return parse_number(item, input_buffer);
    }
    /* array */
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '['))
    {
        return parse_array(item, input_buffer);
    }
    /* object */
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '{'))
    {
        return parse_object(item, input_buffer);
    }

    return false;
}

/* Render a value to text. */
static bscJSON_bool print_value(const bscJSON *const item, printbuffer *const output_buffer)
{
    unsigned char *output = NULL;

    if ((item == NULL) || (output_buffer == NULL))
    {
        return false;
    }

    switch ((item->type) & 0xFF)
    {
    case bscJSON_NULL:
        output = ensure(output_buffer, 5);
        if (output == NULL)
        {
            return false;
        }
        strcpy((char *)output, "null");
        return true;

    case bscJSON_False:
        output = ensure(output_buffer, 6);
        if (output == NULL)
        {
            return false;
        }
        strcpy((char *)output, "false");
        return true;

    case bscJSON_True:
        output = ensure(output_buffer, 5);
        if (output == NULL)
        {
            return false;
        }
        strcpy((char *)output, "true");
        return true;

    case bscJSON_Number:
        return print_number(item, output_buffer);

    case bscJSON_Raw:
    {
        size_t raw_length = 0;
        if (item->valuestring == NULL)
        {
            return false;
        }

        raw_length = strlen(item->valuestring) + sizeof("");
        output = ensure(output_buffer, raw_length);
        if (output == NULL)
        {
            return false;
        }
        memcpy(output, item->valuestring, raw_length);
        return true;
    }

    case bscJSON_String:
        return print_string(item, output_buffer);

    case bscJSON_Array:
        return print_array(item, output_buffer);

    case bscJSON_Object:
        return print_object(item, output_buffer);

    default:
        return false;
    }
}

/* Build an array from input text. */
static bscJSON_bool parse_array(bscJSON *const item, parse_buffer *const input_buffer)
{
    bscJSON *head = NULL; /* head of the linked list */
    bscJSON *current_item = NULL;

    if (input_buffer->depth >= bscJSON_NESTING_LIMIT)
    {
        return false; /* to deeply nested */
    }
    input_buffer->depth++;

    if (buffer_at_offset(input_buffer)[0] != '[')
    {
        /* not an array */
        goto fail;
    }

    input_buffer->offset++;
    buffer_skip_whitespace(input_buffer);
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == ']'))
    {
        /* empty array */
        goto success;
    }

    /* check if we skipped to the end of the buffer */
    if (cannot_access_at_index(input_buffer, 0))
    {
        input_buffer->offset--;
        goto fail;
    }

    /* step back to character in front of the first element */
    input_buffer->offset--;
    /* loop through the comma separated array elements */
    do
    {
        /* allocate next item */
        bscJSON *new_item = bscJSON_New_Item(&(input_buffer->hooks));
        if (new_item == NULL)
        {
            goto fail; /* allocation failure */
        }

        /* attach next item to list */
        if (head == NULL)
        {
            /* start the linked list */
            current_item = head = new_item;
        }
        else
        {
            /* add to the end and advance */
            current_item->next = new_item;
            new_item->prev = current_item;
            current_item = new_item;
        }

        /* parse next value */
        input_buffer->offset++;
        buffer_skip_whitespace(input_buffer);
        if (!parse_value(current_item, input_buffer))
        {
            goto fail; /* failed to parse value */
        }
        buffer_skip_whitespace(input_buffer);
    } while (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == ','));

    if (cannot_access_at_index(input_buffer, 0) || buffer_at_offset(input_buffer)[0] != ']')
    {
        goto fail; /* expected end of array */
    }

success:
    input_buffer->depth--;

    item->type = bscJSON_Array;
    item->child = head;

    input_buffer->offset++;

    return true;

fail:
    if (head != NULL)
    {
        bscJSON_Delete(head);
    }

    return false;
}

/* Render an array to text */
static bscJSON_bool print_array(const bscJSON *const item, printbuffer *const output_buffer)
{
    unsigned char *output_pointer = NULL;
    size_t length = 0;
    bscJSON *current_element = item->child;

    if (output_buffer == NULL)
    {
        return false;
    }

    /* Compose the output array. */
    /* opening square bracket */
    output_pointer = ensure(output_buffer, 1);
    if (output_pointer == NULL)
    {
        return false;
    }

    *output_pointer = '[';
    output_buffer->offset++;
    output_buffer->depth++;

    while (current_element != NULL)
    {
        if (!print_value(current_element, output_buffer))
        {
            return false;
        }
        update_offset(output_buffer);
        if (current_element->next)
        {
            length = (size_t)(output_buffer->format ? 2 : 1);
            output_pointer = ensure(output_buffer, length + 1);
            if (output_pointer == NULL)
            {
                return false;
            }
            *output_pointer++ = ',';
            if (output_buffer->format)
            {
                *output_pointer++ = ' ';
            }
            *output_pointer = '\0';
            output_buffer->offset += length;
        }
        current_element = current_element->next;
    }

    output_pointer = ensure(output_buffer, 2);
    if (output_pointer == NULL)
    {
        return false;
    }
    *output_pointer++ = ']';
    *output_pointer = '\0';
    output_buffer->depth--;

    return true;
}

/* Build an object from the text. */
static bscJSON_bool parse_object(bscJSON *const item, parse_buffer *const input_buffer)
{
    bscJSON *head = NULL; /* linked list head */
    bscJSON *current_item = NULL;

    if (input_buffer->depth >= bscJSON_NESTING_LIMIT)
    {
        return false; /* to deeply nested */
    }
    input_buffer->depth++;

    if (cannot_access_at_index(input_buffer, 0) || (buffer_at_offset(input_buffer)[0] != '{'))
    {
        goto fail; /* not an object */
    }

    input_buffer->offset++;
    buffer_skip_whitespace(input_buffer);
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '}'))
    {
        goto success; /* empty object */
    }

    /* check if we skipped to the end of the buffer */
    if (cannot_access_at_index(input_buffer, 0))
    {
        input_buffer->offset--;
        goto fail;
    }

    /* step back to character in front of the first element */
    input_buffer->offset--;
    /* loop through the comma separated array elements */
    do
    {
        /* allocate next item */
        bscJSON *new_item = bscJSON_New_Item(&(input_buffer->hooks));
        if (new_item == NULL)
        {
            goto fail; /* allocation failure */
        }

        /* attach next item to list */
        if (head == NULL)
        {
            /* start the linked list */
            current_item = head = new_item;
        }
        else
        {
            /* add to the end and advance */
            current_item->next = new_item;
            new_item->prev = current_item;
            current_item = new_item;
        }

        /* parse the name of the child */
        input_buffer->offset++;
        buffer_skip_whitespace(input_buffer);
        if (!parse_string(current_item, input_buffer))
        {
            goto fail; /* failed to parse name */
        }
        buffer_skip_whitespace(input_buffer);

        /* swap valuestring and string, because we parsed the name */
        current_item->string = current_item->valuestring;
        current_item->valuestring = NULL;

        if (cannot_access_at_index(input_buffer, 0) || (buffer_at_offset(input_buffer)[0] != ':'))
        {
            goto fail; /* invalid object */
        }

        /* parse the value */
        input_buffer->offset++;
        buffer_skip_whitespace(input_buffer);
        if (!parse_value(current_item, input_buffer))
        {
            goto fail; /* failed to parse value */
        }
        buffer_skip_whitespace(input_buffer);
    } while (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == ','));

    if (cannot_access_at_index(input_buffer, 0) || (buffer_at_offset(input_buffer)[0] != '}'))
    {
        goto fail; /* expected end of object */
    }

success:
    input_buffer->depth--;

    item->type = bscJSON_Object;
    item->child = head;

    input_buffer->offset++;
    return true;

fail:
    if (head != NULL)
    {
        bscJSON_Delete(head);
    }

    return false;
}

/* Render an object to text. */
static bscJSON_bool print_object(const bscJSON *const item, printbuffer *const output_buffer)
{
    unsigned char *output_pointer = NULL;
    size_t length = 0;
    bscJSON *current_item = item->child;

    if (output_buffer == NULL)
    {
        return false;
    }

    /* Compose the output: */
    length = (size_t)(output_buffer->format ? 2 : 1); /* fmt: {\n */
    output_pointer = ensure(output_buffer, length + 1);
    if (output_pointer == NULL)
    {
        return false;
    }

    *output_pointer++ = '{';
    output_buffer->depth++;
    if (output_buffer->format)
    {
        *output_pointer++ = '\n';
    }
    output_buffer->offset += length;

    while (current_item)
    {
        if (output_buffer->format)
        {
            size_t i;
            output_pointer = ensure(output_buffer, output_buffer->depth);
            if (output_pointer == NULL)
            {
                return false;
            }
            for (i = 0; i < output_buffer->depth; i++)
            {
                *output_pointer++ = '\t';
            }
            output_buffer->offset += output_buffer->depth;
        }

        /* print key */
        if (!print_string_ptr((unsigned char *)current_item->string, output_buffer))
        {
            return false;
        }
        update_offset(output_buffer);

        length = (size_t)(output_buffer->format ? 2 : 1);
        output_pointer = ensure(output_buffer, length);
        if (output_pointer == NULL)
        {
            return false;
        }
        *output_pointer++ = ':';
        if (output_buffer->format)
        {
            *output_pointer++ = '\t';
        }
        output_buffer->offset += length;

        /* print value */
        if (!print_value(current_item, output_buffer))
        {
            return false;
        }
        update_offset(output_buffer);

        /* print comma if not last */
        length = ((size_t)(output_buffer->format ? 1 : 0) + (size_t)(current_item->next ? 1 : 0));
        output_pointer = ensure(output_buffer, length + 1);
        if (output_pointer == NULL)
        {
            return false;
        }
        if (current_item->next)
        {
            *output_pointer++ = ',';
        }

        if (output_buffer->format)
        {
            *output_pointer++ = '\n';
        }
        *output_pointer = '\0';
        output_buffer->offset += length;

        current_item = current_item->next;
    }

    output_pointer = ensure(output_buffer, output_buffer->format ? (output_buffer->depth + 1) : 2);
    if (output_pointer == NULL)
    {
        return false;
    }
    if (output_buffer->format)
    {
        size_t i;
        for (i = 0; i < (output_buffer->depth - 1); i++)
        {
            *output_pointer++ = '\t';
        }
    }
    *output_pointer++ = '}';
    *output_pointer = '\0';
    output_buffer->depth--;

    return true;
}

/* Get Array size/item / object item. */
bscJSON_PUBLIC(int) bscJSON_GetArraySize(const bscJSON *array)
{
    bscJSON *child = NULL;
    size_t size = 0;

    if (array == NULL)
    {
        return 0;
    }

    child = array->child;

    while (child != NULL)
    {
        size++;
        child = child->next;
    }

    /* FIXME: Can overflow here. Cannot be fixed without breaking the API */

    return (int)size;
}

static bscJSON *get_array_item(const bscJSON *array, size_t index)
{
    bscJSON *current_child = NULL;

    if (array == NULL)
    {
        return NULL;
    }

    current_child = array->child;
    while ((current_child != NULL) && (index > 0))
    {
        index--;
        current_child = current_child->next;
    }

    return current_child;
}

bscJSON_PUBLIC(bscJSON *) bscJSON_GetArrayItem(const bscJSON *array, int index)
{
    if (index < 0)
    {
        return NULL;
    }

    return get_array_item(array, (size_t)index);
}

static bscJSON *get_object_item(const bscJSON *const object, const char *const name, const bscJSON_bool case_sensitive)
{
    bscJSON *current_element = NULL;

    if ((object == NULL) || (name == NULL))
    {
        return NULL;
    }

    current_element = object->child;
    if (case_sensitive)
    {
        while ((current_element != NULL) && (current_element->string != NULL) && (strcmp(name, current_element->string) != 0))
        {
            current_element = current_element->next;
        }
    }
    else
    {
        while ((current_element != NULL) && (case_insensitive_strcmp((const unsigned char *)name, (const unsigned char *)(current_element->string)) != 0))
        {
            current_element = current_element->next;
        }
    }

    if ((current_element == NULL) || (current_element->string == NULL))
    {
        return NULL;
    }

    return current_element;
}

bscJSON_PUBLIC(bscJSON *) bscJSON_GetObjectItem(const bscJSON *const object, const char *const string)
{
    return get_object_item(object, string, false);
}

bscJSON_PUBLIC(bscJSON *) bscJSON_GetObjectItemCaseSensitive(const bscJSON *const object, const char *const string)
{
    return get_object_item(object, string, true);
}

bscJSON_PUBLIC(bscJSON_bool) bscJSON_HasObjectItem(const bscJSON *object, const char *string)
{
    return bscJSON_GetObjectItem(object, string) ? 1 : 0;
}

/* Utility for array list handling. */
static void suffix_object(bscJSON *prev, bscJSON *item)
{
    prev->next = item;
    item->prev = prev;
}

/* Utility for handling references. */
static bscJSON *create_reference(const bscJSON *item, const internal_hooks *const hooks)
{
    bscJSON *reference = NULL;
    if (item == NULL)
    {
        return NULL;
    }

    reference = bscJSON_New_Item(hooks);
    if (reference == NULL)
    {
        return NULL;
    }

    memcpy(reference, item, sizeof(bscJSON));
    reference->string = NULL;
    reference->type |= bscJSON_IsReference;
    reference->next = reference->prev = NULL;
    return reference;
}

static bscJSON_bool add_item_to_array(bscJSON *array, bscJSON *item)
{
    bscJSON *child = NULL;

    if ((item == NULL) || (array == NULL))
    {
        return false;
    }

    child = array->child;

    if (child == NULL)
    {
        /* list is empty, start new one */
        array->child = item;
    }
    else
    {
        /* append to the end */
        while (child->next)
        {
            child = child->next;
        }
        suffix_object(child, item);
    }

    return true;
}

/* Add item to array/object. */
bscJSON_PUBLIC(void) bscJSON_AddItemToArray(bscJSON *array, bscJSON *item)
{
    add_item_to_array(array, item);
}

#if defined(__clang__) || (defined(__GNUC__) && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 5))))
#pragma GCC diagnostic push
#endif
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif
/* helper function to cast away const */
static void *cast_away_const(const void *string)
{
    return (void *)string;
}
#if defined(__clang__) || (defined(__GNUC__) && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 5))))
#pragma GCC diagnostic pop
#endif

static bscJSON_bool add_item_to_object(bscJSON *const object, const char *const string, bscJSON *const item, const internal_hooks *const hooks, const bscJSON_bool constant_key)
{
    char *new_key = NULL;
    int new_type = bscJSON_Invalid;

    if ((object == NULL) || (string == NULL) || (item == NULL))
    {
        return false;
    }

    if (constant_key)
    {
        new_key = (char *)cast_away_const(string);
        new_type = item->type | bscJSON_StringIsConst;
    }
    else
    {
        new_key = (char *)bscJSON_strdup((const unsigned char *)string, hooks);
        if (new_key == NULL)
        {
            return false;
        }

        new_type = item->type & ~bscJSON_StringIsConst;
    }

    if (!(item->type & bscJSON_StringIsConst) && (item->string != NULL))
    {
        hooks->deallocate(item->string);
    }

    item->string = new_key;
    item->type = new_type;

    return add_item_to_array(object, item);
}

bscJSON_PUBLIC(void) bscJSON_AddItemToObject(bscJSON *object, const char *string, bscJSON *item)
{
    add_item_to_object(object, string, item, &global_hooks, false);
}

/* Add an item to an object with constant string as key */
bscJSON_PUBLIC(void) bscJSON_AddItemToObjectCS(bscJSON *object, const char *string, bscJSON *item)
{
    add_item_to_object(object, string, item, &global_hooks, true);
}

bscJSON_PUBLIC(void) bscJSON_AddItemReferenceToArray(bscJSON *array, bscJSON *item)
{
    if (array == NULL)
    {
        return;
    }

    add_item_to_array(array, create_reference(item, &global_hooks));
}

bscJSON_PUBLIC(void) bscJSON_AddItemReferenceToObject(bscJSON *object, const char *string, bscJSON *item)
{
    if ((object == NULL) || (string == NULL))
    {
        return;
    }

    add_item_to_object(object, string, create_reference(item, &global_hooks), &global_hooks, false);
}

bscJSON_PUBLIC(bscJSON *) bscJSON_AddNullToObject(bscJSON *const object, const char *const name)
{
    bscJSON *null = bscJSON_CreateNull();
    if (add_item_to_object(object, name, null, &global_hooks, false))
    {
        return null;
    }

    bscJSON_Delete(null);
    return NULL;
}

bscJSON_PUBLIC(bscJSON *) bscJSON_AddTrueToObject(bscJSON *const object, const char *const name)
{
    bscJSON *true_item = bscJSON_CreateTrue();
    if (add_item_to_object(object, name, true_item, &global_hooks, false))
    {
        return true_item;
    }

    bscJSON_Delete(true_item);
    return NULL;
}

bscJSON_PUBLIC(bscJSON *) bscJSON_AddFalseToObject(bscJSON *const object, const char *const name)
{
    bscJSON *false_item = bscJSON_CreateFalse();
    if (add_item_to_object(object, name, false_item, &global_hooks, false))
    {
        return false_item;
    }

    bscJSON_Delete(false_item);
    return NULL;
}

bscJSON_PUBLIC(bscJSON *) bscJSON_AddBoolToObject(bscJSON *const object, const char *const name, const bscJSON_bool boolean)
{
    bscJSON *bool_item = bscJSON_CreateBool(boolean);
    if (add_item_to_object(object, name, bool_item, &global_hooks, false))
    {
        return bool_item;
    }

    bscJSON_Delete(bool_item);
    return NULL;
}

bscJSON_PUBLIC(bscJSON *) bscJSON_AddNumberToObject(bscJSON *const object, const char *const name, const double number)
{
    bscJSON *number_item = bscJSON_CreateNumber(number);
    if (add_item_to_object(object, name, number_item, &global_hooks, false))
    {
        return number_item;
    }

    bscJSON_Delete(number_item);
    return NULL;
}

bscJSON_PUBLIC(bscJSON *) bscJSON_AddStringToObject(bscJSON *const object, const char *const name, const char *const string)
{
    bscJSON *string_item = bscJSON_CreateString(string);
    if (add_item_to_object(object, name, string_item, &global_hooks, false))
    {
        return string_item;
    }

    bscJSON_Delete(string_item);
    return NULL;
}

bscJSON_PUBLIC(bscJSON *) bscJSON_AddRawToObject(bscJSON *const object, const char *const name, const char *const raw)
{
    bscJSON *raw_item = bscJSON_CreateRaw(raw);
    if (add_item_to_object(object, name, raw_item, &global_hooks, false))
    {
        return raw_item;
    }

    bscJSON_Delete(raw_item);
    return NULL;
}

bscJSON_PUBLIC(bscJSON *) bscJSON_AddObjectToObject(bscJSON *const object, const char *const name, bscJSON *const s)
{
    if (add_item_to_object(object, name, s, &global_hooks, false))
    {
        return s;
    }

    return NULL;
}

bscJSON_PUBLIC(bscJSON *) bscJSON_AddArrayToObject(bscJSON *const object, const char *const name, bscJSON *const s)
{
    if (add_item_to_object(object, name, s, &global_hooks, false))
    {
        return s;
    }

    return NULL;
}

bscJSON_PUBLIC(bscJSON *) bscJSON_AddObjectToArray(bscJSON *const object, const char *const name, bscJSON *const s)
{
    if (add_item_to_object(object, name, s, &global_hooks, false))
    {
        return s;
    }

    return NULL;
}

bscJSON_PUBLIC(bscJSON *) bscJSON_DetachItemViaPointer(bscJSON *parent, bscJSON *const item)
{
    if ((parent == NULL) || (item == NULL))
    {
        return NULL;
    }

    if (item->prev != NULL)
    {
        /* not the first element */
        item->prev->next = item->next;
    }
    if (item->next != NULL)
    {
        /* not the last element */
        item->next->prev = item->prev;
    }

    if (item == parent->child)
    {
        /* first element */
        parent->child = item->next;
    }
    /* make sure the detached item doesn't point anywhere anymore */
    item->prev = NULL;
    item->next = NULL;

    return item;
}

bscJSON_PUBLIC(bscJSON *) bscJSON_DetachItemFromArray(bscJSON *array, int which)
{
    if (which < 0)
    {
        return NULL;
    }

    return bscJSON_DetachItemViaPointer(array, get_array_item(array, (size_t)which));
}

bscJSON_PUBLIC(void) bscJSON_DeleteItemFromArray(bscJSON *array, int which)
{
    bscJSON_Delete(bscJSON_DetachItemFromArray(array, which));
}

bscJSON_PUBLIC(bscJSON *) bscJSON_DetachItemFromObject(bscJSON *object, const char *string)
{
    bscJSON *to_detach = bscJSON_GetObjectItem(object, string);

    return bscJSON_DetachItemViaPointer(object, to_detach);
}

bscJSON_PUBLIC(bscJSON *) bscJSON_DetachItemFromObjectCaseSensitive(bscJSON *object, const char *string)
{
    bscJSON *to_detach = bscJSON_GetObjectItemCaseSensitive(object, string);

    return bscJSON_DetachItemViaPointer(object, to_detach);
}

bscJSON_PUBLIC(void) bscJSON_DeleteItemFromObject(bscJSON *object, const char *string)
{
    bscJSON_Delete(bscJSON_DetachItemFromObject(object, string));
}

bscJSON_PUBLIC(void) bscJSON_DeleteItemFromObjectCaseSensitive(bscJSON *object, const char *string)
{
    bscJSON_Delete(bscJSON_DetachItemFromObjectCaseSensitive(object, string));
}

/* Replace array/object items with new ones. */
bscJSON_PUBLIC(void) bscJSON_InsertItemInArray(bscJSON *array, int which, bscJSON *newitem)
{
    bscJSON *after_inserted = NULL;

    if (which < 0)
    {
        return;
    }

    after_inserted = get_array_item(array, (size_t)which);
    if (after_inserted == NULL)
    {
        add_item_to_array(array, newitem);
        return;
    }

    newitem->next = after_inserted;
    newitem->prev = after_inserted->prev;
    after_inserted->prev = newitem;
    if (after_inserted == array->child)
    {
        array->child = newitem;
    }
    else
    {
        newitem->prev->next = newitem;
    }
}

bscJSON_PUBLIC(bscJSON_bool) bscJSON_ReplaceItemViaPointer(bscJSON *const parent, bscJSON *const item, bscJSON *replacement)
{
    if ((parent == NULL) || (replacement == NULL) || (item == NULL))
    {
        return false;
    }

    if (replacement == item)
    {
        return true;
    }

    replacement->next = item->next;
    replacement->prev = item->prev;

    if (replacement->next != NULL)
    {
        replacement->next->prev = replacement;
    }
    if (replacement->prev != NULL)
    {
        replacement->prev->next = replacement;
    }
    if (parent->child == item)
    {
        parent->child = replacement;
    }

    item->next = NULL;
    item->prev = NULL;
    bscJSON_Delete(item);

    return true;
}

bscJSON_PUBLIC(void) bscJSON_ReplaceItemInArray(bscJSON *array, int which, bscJSON *newitem)
{
    if (which < 0)
    {
        return;
    }

    bscJSON_ReplaceItemViaPointer(array, get_array_item(array, (size_t)which), newitem);
}

static bscJSON_bool replace_item_in_object(bscJSON *object, const char *string, bscJSON *replacement, bscJSON_bool case_sensitive)
{
    if ((replacement == NULL) || (string == NULL))
    {
        return false;
    }

    /* replace the name in the replacement */
    if (!(replacement->type & bscJSON_StringIsConst) && (replacement->string != NULL))
    {
        bscJSON_free(replacement->string);
    }
    replacement->string = (char *)bscJSON_strdup((const unsigned char *)string, &global_hooks);
    replacement->type &= ~bscJSON_StringIsConst;

    bscJSON_ReplaceItemViaPointer(object, get_object_item(object, string, case_sensitive), replacement);

    return true;
}

bscJSON_PUBLIC(void) bscJSON_ReplaceItemInObject(bscJSON *object, const char *string, bscJSON *newitem)
{
    replace_item_in_object(object, string, newitem, false);
}

bscJSON_PUBLIC(void) bscJSON_ReplaceItemInObjectCaseSensitive(bscJSON *object, const char *string, bscJSON *newitem)
{
    replace_item_in_object(object, string, newitem, true);
}

/* Create basic types: */
bscJSON_PUBLIC(bscJSON *) bscJSON_CreateNull(void)
{
    bscJSON *item = bscJSON_New_Item(&global_hooks);
    if (item)
    {
        item->type = bscJSON_NULL;
    }

    return item;
}

bscJSON_PUBLIC(bscJSON *) bscJSON_CreateTrue(void)
{
    bscJSON *item = bscJSON_New_Item(&global_hooks);
    if (item)
    {
        item->type = bscJSON_True;
    }

    return item;
}

bscJSON_PUBLIC(bscJSON *) bscJSON_CreateFalse(void)
{
    bscJSON *item = bscJSON_New_Item(&global_hooks);
    if (item)
    {
        item->type = bscJSON_False;
    }

    return item;
}

bscJSON_PUBLIC(bscJSON *) bscJSON_CreateBool(bscJSON_bool b)
{
    bscJSON *item = bscJSON_New_Item(&global_hooks);
    if (item)
    {
        item->type = b ? bscJSON_True : bscJSON_False;
    }

    return item;
}

bscJSON_PUBLIC(bscJSON *) bscJSON_CreateNumber(double num)
{
    bscJSON *item = bscJSON_New_Item(&global_hooks);
    if (item)
    {
        item->type = bscJSON_Number;
        item->valuedouble = num;

        /* use saturation in case of overflow */
        if (num >= INT_MAX)
        {
            item->valueint = INT_MAX;
        }
        else if (num <= (double)INT_MIN)
        {
            item->valueint = INT_MIN;
        }
        else
        {
            item->valueint = (int)num;
        }
    }

    return item;
}

bscJSON_PUBLIC(bscJSON *) bscJSON_CreateString(const char *string)
{
    bscJSON *item = bscJSON_New_Item(&global_hooks);
    if (item)
    {
        item->type = bscJSON_String;
        item->valuestring = (char *)bscJSON_strdup((const unsigned char *)string, &global_hooks);
        if (!item->valuestring)
        {
            bscJSON_Delete(item);
            return NULL;
        }
    }

    return item;
}

bscJSON_PUBLIC(bscJSON *) bscJSON_CreateStringReference(const char *string)
{
    bscJSON *item = bscJSON_New_Item(&global_hooks);
    if (item != NULL)
    {
        item->type = bscJSON_String | bscJSON_IsReference;
        item->valuestring = (char *)cast_away_const(string);
    }

    return item;
}

bscJSON_PUBLIC(bscJSON *) bscJSON_CreateObjectReference(const bscJSON *child)
{
    bscJSON *item = bscJSON_New_Item(&global_hooks);
    if (item != NULL)
    {
        item->type = bscJSON_Object | bscJSON_IsReference;
        item->child = (bscJSON *)cast_away_const(child);
    }

    return item;
}

bscJSON_PUBLIC(bscJSON *) bscJSON_CreateArrayReference(const bscJSON *child)
{
    bscJSON *item = bscJSON_New_Item(&global_hooks);
    if (item != NULL)
    {
        item->type = bscJSON_Array | bscJSON_IsReference;
        item->child = (bscJSON *)cast_away_const(child);
    }

    return item;
}

bscJSON_PUBLIC(bscJSON *) bscJSON_CreateRaw(const char *raw)
{
    bscJSON *item = bscJSON_New_Item(&global_hooks);
    if (item)
    {
        item->type = bscJSON_Raw;
        item->valuestring = (char *)bscJSON_strdup((const unsigned char *)raw, &global_hooks);
        if (!item->valuestring)
        {
            bscJSON_Delete(item);
            return NULL;
        }
    }

    return item;
}

bscJSON_PUBLIC(bscJSON *) bscJSON_CreateArray(void)
{
    bscJSON *item = bscJSON_New_Item(&global_hooks);
    if (item)
    {
        item->type = bscJSON_Array;
    }

    return item;
}

bscJSON_PUBLIC(bscJSON *) bscJSON_CreateObject(void)
{
    bscJSON *item = bscJSON_New_Item(&global_hooks);
    if (item)
    {
        item->type = bscJSON_Object;
    }

    return item;
}

/* Create Arrays: */
bscJSON_PUBLIC(bscJSON *) bscJSON_CreateIntArray(const int *numbers, int count)
{
    size_t i = 0;
    bscJSON *n = NULL;
    bscJSON *p = NULL;
    bscJSON *a = NULL;

    if ((count < 0) || (numbers == NULL))
    {
        return NULL;
    }

    a = bscJSON_CreateArray();
    for (i = 0; a && (i < (size_t)count); i++)
    {
        n = bscJSON_CreateNumber(numbers[i]);
        if (!n)
        {
            bscJSON_Delete(a);
            return NULL;
        }
        if (!i)
        {
            a->child = n;
        }
        else
        {
            suffix_object(p, n);
        }
        p = n;
    }

    return a;
}

bscJSON_PUBLIC(bscJSON *) bscJSON_CreateFloatArray(const float *numbers, int count)
{
    size_t i = 0;
    bscJSON *n = NULL;
    bscJSON *p = NULL;
    bscJSON *a = NULL;

    if ((count < 0) || (numbers == NULL))
    {
        return NULL;
    }

    a = bscJSON_CreateArray();

    for (i = 0; a && (i < (size_t)count); i++)
    {
        n = bscJSON_CreateNumber((double)numbers[i]);
        if (!n)
        {
            bscJSON_Delete(a);
            return NULL;
        }
        if (!i)
        {
            a->child = n;
        }
        else
        {
            suffix_object(p, n);
        }
        p = n;
    }

    return a;
}

bscJSON_PUBLIC(bscJSON *) bscJSON_CreateDoubleArray(const double *numbers, int count)
{
    size_t i = 0;
    bscJSON *n = NULL;
    bscJSON *p = NULL;
    bscJSON *a = NULL;

    if ((count < 0) || (numbers == NULL))
    {
        return NULL;
    }

    a = bscJSON_CreateArray();

    for (i = 0; a && (i < (size_t)count); i++)
    {
        n = bscJSON_CreateNumber(numbers[i]);
        if (!n)
        {
            bscJSON_Delete(a);
            return NULL;
        }
        if (!i)
        {
            a->child = n;
        }
        else
        {
            suffix_object(p, n);
        }
        p = n;
    }

    return a;
}

bscJSON_PUBLIC(bscJSON *) bscJSON_CreateStringArray(const char **strings, int count)
{
    size_t i = 0;
    bscJSON *n = NULL;
    bscJSON *p = NULL;
    bscJSON *a = NULL;

    if ((count < 0) || (strings == NULL))
    {
        return NULL;
    }

    a = bscJSON_CreateArray();

    for (i = 0; a && (i < (size_t)count); i++)
    {
        n = bscJSON_CreateString(strings[i]);
        if (!n)
        {
            bscJSON_Delete(a);
            return NULL;
        }
        if (!i)
        {
            a->child = n;
        }
        else
        {
            suffix_object(p, n);
        }
        p = n;
    }

    return a;
}

/* Duplication */
bscJSON_PUBLIC(bscJSON *) bscJSON_Duplicate(const bscJSON *item, bscJSON_bool recurse)
{
    bscJSON *newitem = NULL;
    bscJSON *child = NULL;
    bscJSON *next = NULL;
    bscJSON *newchild = NULL;

    /* Bail on bad ptr */
    if (!item)
    {
        goto fail;
    }
    /* Create new item */
    newitem = bscJSON_New_Item(&global_hooks);
    if (!newitem)
    {
        goto fail;
    }
    /* Copy over all vars */
    newitem->type = item->type & (~bscJSON_IsReference);
    newitem->valueint = item->valueint;
    newitem->valuedouble = item->valuedouble;
    if (item->valuestring)
    {
        newitem->valuestring = (char *)bscJSON_strdup((unsigned char *)item->valuestring, &global_hooks);
        if (!newitem->valuestring)
        {
            goto fail;
        }
    }
    if (item->string)
    {
        newitem->string = (item->type & bscJSON_StringIsConst) ? item->string : (char *)bscJSON_strdup((unsigned char *)item->string, &global_hooks);
        if (!newitem->string)
        {
            goto fail;
        }
    }
    /* If non-recursive, then we're done! */
    if (!recurse)
    {
        return newitem;
    }
    /* Walk the ->next chain for the child. */
    child = item->child;
    while (child != NULL)
    {
        newchild = bscJSON_Duplicate(child, true); /* Duplicate (with recurse) each item in the ->next chain */
        if (!newchild)
        {
            goto fail;
        }
        if (next != NULL)
        {
            /* If newitem->child already set, then crosswire ->prev and ->next and move on */
            next->next = newchild;
            newchild->prev = next;
            next = newchild;
        }
        else
        {
            /* Set newitem->child and move to it */
            newitem->child = newchild;
            next = newchild;
        }
        child = child->next;
    }

    return newitem;

fail:
    if (newitem != NULL)
    {
        bscJSON_Delete(newitem);
    }

    return NULL;
}

static void skip_oneline_comment(char **input)
{
    *input += static_strlen("//");

    for (; (*input)[0] != '\0'; ++(*input))
    {
        if ((*input)[0] == '\n')
        {
            *input += static_strlen("\n");
            return;
        }
    }
}

static void skip_multiline_comment(char **input)
{
    *input += static_strlen("/*");

    for (; (*input)[0] != '\0'; ++(*input))
    {
        if (((*input)[0] == '*') && ((*input)[1] == '/'))
        {
            *input += static_strlen("*/");
            return;
        }
    }
}

static void minify_string(char **input, char **output)
{
    (*output)[0] = (*input)[0];
    *input += static_strlen("\"");
    *output += static_strlen("\"");

    for (; (*input)[0] != '\0'; (void)++(*input), ++(*output))
    {
        (*output)[0] = (*input)[0];

        if ((*input)[0] == '\"')
        {
            (*output)[0] = '\"';
            *input += static_strlen("\"");
            *output += static_strlen("\"");
            return;
        }
        else if (((*input)[0] == '\\') && ((*input)[1] == '\"'))
        {
            (*output)[1] = (*input)[1];
            *input += static_strlen("\"");
            *output += static_strlen("\"");
        }
    }
}

bscJSON_PUBLIC(void) bscJSON_Minify(char *json)
{
    char *into = json;

    if (json == NULL)
    {
        return;
    }

    while (json[0] != '\0')
    {
        switch (json[0])
        {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            json++;
            break;

        case '/':
            if (json[1] == '/')
            {
                skip_oneline_comment(&json);
            }
            else if (json[1] == '*')
            {
                skip_multiline_comment(&json);
            }
            else
            {
                json++;
            }
            break;

        case '\"':
            minify_string(&json, (char **)&into);
            break;

        default:
            into[0] = json[0];
            json++;
            into++;
        }
    }

    /* and null-terminate. */
    *into = '\0';
}

bscJSON_PUBLIC(bscJSON_bool) bscJSON_IsInvalid(const bscJSON *const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xFF) == bscJSON_Invalid;
}

bscJSON_PUBLIC(bscJSON_bool) bscJSON_IsFalse(const bscJSON *const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xFF) == bscJSON_False;
}

bscJSON_PUBLIC(bscJSON_bool) bscJSON_IsTrue(const bscJSON *const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xff) == bscJSON_True;
}

bscJSON_PUBLIC(bscJSON_bool) bscJSON_IsBool(const bscJSON *const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & (bscJSON_True | bscJSON_False)) != 0;
}
bscJSON_PUBLIC(bscJSON_bool) bscJSON_IsNull(const bscJSON *const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xFF) == bscJSON_NULL;
}

bscJSON_PUBLIC(bscJSON_bool) bscJSON_IsNumber(const bscJSON *const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xFF) == bscJSON_Number;
}

bscJSON_PUBLIC(bscJSON_bool) bscJSON_IsString(const bscJSON *const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xFF) == bscJSON_String;
}

bscJSON_PUBLIC(bscJSON_bool) bscJSON_IsArray(const bscJSON *const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xFF) == bscJSON_Array;
}

bscJSON_PUBLIC(bscJSON_bool) bscJSON_IsObject(const bscJSON *const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xFF) == bscJSON_Object;
}

bscJSON_PUBLIC(bscJSON_bool) bscJSON_IsRaw(const bscJSON *const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xFF) == bscJSON_Raw;
}

bscJSON_PUBLIC(bscJSON_bool) bscJSON_Compare(const bscJSON *const a, const bscJSON *const b, const bscJSON_bool case_sensitive)
{
    if ((a == NULL) || (b == NULL) || ((a->type & 0xFF) != (b->type & 0xFF)) || bscJSON_IsInvalid(a))
    {
        return false;
    }

    /* check if type is valid */
    switch (a->type & 0xFF)
    {
    case bscJSON_False:
    case bscJSON_True:
    case bscJSON_NULL:
    case bscJSON_Number:
    case bscJSON_String:
    case bscJSON_Raw:
    case bscJSON_Array:
    case bscJSON_Object:
        break;

    default:
        return false;
    }

    /* identical objects are equal */
    if (a == b)
    {
        return true;
    }

    switch (a->type & 0xFF)
    {
    /* in these cases and equal type is enough */
    case bscJSON_False:
    case bscJSON_True:
    case bscJSON_NULL:
        return true;

    case bscJSON_Number:
        if (a->valuedouble == b->valuedouble)
        {
            return true;
        }
        return false;

    case bscJSON_String:
    case bscJSON_Raw:
        if ((a->valuestring == NULL) || (b->valuestring == NULL))
        {
            return false;
        }
        if (strcmp(a->valuestring, b->valuestring) == 0)
        {
            return true;
        }

        return false;

    case bscJSON_Array:
    {
        bscJSON *a_element = a->child;
        bscJSON *b_element = b->child;

        for (; (a_element != NULL) && (b_element != NULL);)
        {
            if (!bscJSON_Compare(a_element, b_element, case_sensitive))
            {
                return false;
            }

            a_element = a_element->next;
            b_element = b_element->next;
        }

        /* one of the arrays is longer than the other */
        if (a_element != b_element)
        {
            return false;
        }

        return true;
    }

    case bscJSON_Object:
    {
        bscJSON *a_element = NULL;
        bscJSON *b_element = NULL;
        bscJSON_ArrayForEach(a_element, a)
        {
            /* TODO This has O(n^2) runtime, which is horrible! */
            b_element = get_object_item(b, a_element->string, case_sensitive);
            if (b_element == NULL)
            {
                return false;
            }

            if (!bscJSON_Compare(a_element, b_element, case_sensitive))
            {
                return false;
            }
        }

        /* doing this twice, once on a and b to prevent true comparison if a subset of b
             * TODO: Do this the proper way, this is just a fix for now */
        bscJSON_ArrayForEach(b_element, b)
        {
            a_element = get_object_item(a, b_element->string, case_sensitive);
            if (a_element == NULL)
            {
                return false;
            }

            if (!bscJSON_Compare(b_element, a_element, case_sensitive))
            {
                return false;
            }
        }

        return true;
    }

    default:
        return false;
    }
}

bscJSON_PUBLIC(void *) bscJSON_malloc(size_t size)
{
    return global_hooks.allocate(size);
}

bscJSON_PUBLIC(void) bscJSON_free(void *object)
{
    global_hooks.deallocate(object);
}
