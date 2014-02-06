/*
 * Copyright 2010, Lloyd Hilaiel.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 * 
 *  3. Neither the name of Lloyd Hilaiel nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */ 

#include "api/yajl_gen.h"
#include "yajl_buf.h"
#include "yajl_encode.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

typedef enum {
    rk_yajl_gen_start,
    rk_yajl_gen_map_start,
    rk_yajl_gen_map_key,
    rk_yajl_gen_map_val,
    rk_yajl_gen_array_start,
    rk_yajl_gen_in_array,
    rk_yajl_gen_complete,
    rk_yajl_gen_error
} rk_yajl_gen_state;

struct rk_yajl_gen_t 
{
    unsigned int depth;
    unsigned int pretty;
    const char * indentString;
    rk_yajl_gen_state state[YAJL_MAX_DEPTH];
    rk_yajl_print_t print;
    void * ctx; /* rk_yajl_buf */
    /* memory allocation routines */
    rk_yajl_alloc_funcs alloc;
};

rk_yajl_gen
rk_yajl_gen_alloc(const rk_yajl_gen_config * config,
               const rk_yajl_alloc_funcs * afs)
{
    return rk_yajl_gen_alloc2(NULL, config, afs, NULL);
}

rk_yajl_gen
rk_yajl_gen_alloc2(const rk_yajl_print_t callback,
                const rk_yajl_gen_config * config,
                const rk_yajl_alloc_funcs * afs,
                void * ctx)
{
    rk_yajl_gen g = NULL;
    rk_yajl_alloc_funcs afsBuffer;

    /* first order of business is to set up memory allocation routines */
    if (afs != NULL) {
        if (afs->malloc == NULL || afs->realloc == NULL || afs->free == NULL)
        {
            return NULL;
        }
    } else {
        rk_yajl_set_default_alloc_funcs(&afsBuffer);
        afs = &afsBuffer;
    }

    g = (rk_yajl_gen) YA_MALLOC(afs, sizeof(struct rk_yajl_gen_t));
    memset((void *) g, 0, sizeof(struct rk_yajl_gen_t));
    /* copy in pointers to allocation routines */
    memcpy((void *) &(g->alloc), (void *) afs, sizeof(rk_yajl_alloc_funcs));

    if (config) {
        g->pretty = config->beautify;
        g->indentString = config->indentString ? config->indentString : "  ";
    }

    if (callback) {
        g->print = callback;
        g->ctx = ctx;
    } else {
        g->print = (rk_yajl_print_t)&rk_yajl_buf_append;
        g->ctx = rk_yajl_buf_alloc(&(g->alloc));
    }

    return g;
}

void
rk_yajl_gen_free(rk_yajl_gen g)
{
    if (g->print == (rk_yajl_print_t)&rk_yajl_buf_append) rk_yajl_buf_free((rk_yajl_buf)g->ctx);
    YA_FREE(&(g->alloc), g);
}

#define INSERT_SEP \
    if (g->state[g->depth] == rk_yajl_gen_map_key ||               \
        g->state[g->depth] == rk_yajl_gen_in_array) {              \
        g->print(g->ctx, ",", 1);                               \
        if (g->pretty) g->print(g->ctx, "\n", 1);               \
    } else if (g->state[g->depth] == rk_yajl_gen_map_val) {        \
        g->print(g->ctx, ":", 1);                               \
        if (g->pretty) g->print(g->ctx, " ", 1);                \
   } 

#define INSERT_WHITESPACE                                               \
    if (g->pretty) {                                                    \
        if (g->state[g->depth] != rk_yajl_gen_map_val) {                   \
            unsigned int _i;                                            \
            for (_i=0;_i<g->depth;_i++)                                 \
                g->print(g->ctx, g->indentString,                       \
                         strlen(g->indentString));                      \
        }                                                               \
    }

#define ENSURE_NOT_KEY \
    if (g->state[g->depth] == rk_yajl_gen_map_key) {   \
        return rk_yajl_gen_keys_must_be_strings;       \
    }                                               \

/* check that we're not complete, or in error state.  in a valid state
 * to be generating */
#define ENSURE_VALID_STATE \
    if (g->state[g->depth] == rk_yajl_gen_error) {   \
        return rk_yajl_gen_in_error_state;\
    } else if (g->state[g->depth] == rk_yajl_gen_complete) {   \
        return rk_yajl_gen_generation_complete;                \
    }

#define INCREMENT_DEPTH \
    if (++(g->depth) >= YAJL_MAX_DEPTH) return rk_yajl_max_depth_exceeded;

#define APPENDED_ATOM \
    switch (g->state[g->depth]) {                   \
        case rk_yajl_gen_start:                        \
            g->state[g->depth] = rk_yajl_gen_complete; \
            break;                                  \
        case rk_yajl_gen_map_start:                    \
        case rk_yajl_gen_map_key:                      \
            g->state[g->depth] = rk_yajl_gen_map_val;  \
            break;                                  \
        case rk_yajl_gen_array_start:                  \
            g->state[g->depth] = rk_yajl_gen_in_array; \
            break;                                  \
        case rk_yajl_gen_map_val:                      \
            g->state[g->depth] = rk_yajl_gen_map_key;  \
            break;                                  \
        default:                                    \
            break;                                  \
    }                                               \

#define FINAL_NEWLINE                                        \
    if (g->pretty && g->state[g->depth] == rk_yajl_gen_complete) \
        g->print(g->ctx, "\n", 1);        
    
rk_yajl_gen_status
rk_yajl_gen_integer(rk_yajl_gen g, long int number)
{
    char i[32];
    ENSURE_VALID_STATE; ENSURE_NOT_KEY; INSERT_SEP; INSERT_WHITESPACE;
    sprintf(i, "%ld", number);
    g->print(g->ctx, i, strlen(i));
    APPENDED_ATOM;
    FINAL_NEWLINE;
    return rk_yajl_gen_status_ok;
}

#ifdef WIN32
#include <float.h>
#define isnan _isnan
#define isinf !_finite
#endif

rk_yajl_gen_status
rk_yajl_gen_double(rk_yajl_gen g, double number)
{
    char i[32];
    ENSURE_VALID_STATE; ENSURE_NOT_KEY; 
    if (isnan(number) || isinf(number)) return rk_yajl_gen_invalid_number;
    INSERT_SEP; INSERT_WHITESPACE;
    sprintf(i, "%g", number);
    g->print(g->ctx, i, strlen(i));
    APPENDED_ATOM;
    FINAL_NEWLINE;
    return rk_yajl_gen_status_ok;
}

rk_yajl_gen_status
rk_yajl_gen_number(rk_yajl_gen g, const char * s, unsigned int l)
{
    ENSURE_VALID_STATE; ENSURE_NOT_KEY; INSERT_SEP; INSERT_WHITESPACE;
    g->print(g->ctx, s, l);
    APPENDED_ATOM;
    FINAL_NEWLINE;
    return rk_yajl_gen_status_ok;
}

rk_yajl_gen_status
rk_yajl_gen_string(rk_yajl_gen g, const unsigned char * str,
                unsigned int len)
{
    ENSURE_VALID_STATE; INSERT_SEP; INSERT_WHITESPACE;
    g->print(g->ctx, "\"", 1);
    rk_yajl_string_encode2(g->print, g->ctx, str, len);
    g->print(g->ctx, "\"", 1);
    APPENDED_ATOM;
    FINAL_NEWLINE;
    return rk_yajl_gen_status_ok;
}

rk_yajl_gen_status
rk_yajl_gen_null(rk_yajl_gen g)
{
    ENSURE_VALID_STATE; ENSURE_NOT_KEY; INSERT_SEP; INSERT_WHITESPACE;
    g->print(g->ctx, "null", strlen("null"));
    APPENDED_ATOM;
    FINAL_NEWLINE;
    return rk_yajl_gen_status_ok;
}

rk_yajl_gen_status
rk_yajl_gen_bool(rk_yajl_gen g, int boolean)
{
    const char * val = boolean ? "true" : "false";

	ENSURE_VALID_STATE; ENSURE_NOT_KEY; INSERT_SEP; INSERT_WHITESPACE;
    g->print(g->ctx, val, strlen(val));
    APPENDED_ATOM;
    FINAL_NEWLINE;
    return rk_yajl_gen_status_ok;
}

rk_yajl_gen_status
rk_yajl_gen_map_open(rk_yajl_gen g)
{
    ENSURE_VALID_STATE; ENSURE_NOT_KEY; INSERT_SEP; INSERT_WHITESPACE;
    INCREMENT_DEPTH; 
    
    g->state[g->depth] = rk_yajl_gen_map_start;
    g->print(g->ctx, "{", 1);
    if (g->pretty) g->print(g->ctx, "\n", 1);
    FINAL_NEWLINE;
    return rk_yajl_gen_status_ok;
}

rk_yajl_gen_status
rk_yajl_gen_map_close(rk_yajl_gen g)
{
    ENSURE_VALID_STATE; 
    (g->depth)--;
    if (g->pretty) g->print(g->ctx, "\n", 1);
    APPENDED_ATOM;
    INSERT_WHITESPACE;
    g->print(g->ctx, "}", 1);
    FINAL_NEWLINE;
    return rk_yajl_gen_status_ok;
}

rk_yajl_gen_status
rk_yajl_gen_array_open(rk_yajl_gen g)
{
    ENSURE_VALID_STATE; ENSURE_NOT_KEY; INSERT_SEP; INSERT_WHITESPACE;
    INCREMENT_DEPTH; 
    g->state[g->depth] = rk_yajl_gen_array_start;
    g->print(g->ctx, "[", 1);
    if (g->pretty) g->print(g->ctx, "\n", 1);
    FINAL_NEWLINE;
    return rk_yajl_gen_status_ok;
}

rk_yajl_gen_status
rk_yajl_gen_array_close(rk_yajl_gen g)
{
    ENSURE_VALID_STATE;
    if (g->pretty) g->print(g->ctx, "\n", 1);
    (g->depth)--;
    APPENDED_ATOM;
    INSERT_WHITESPACE;
    g->print(g->ctx, "]", 1);
    FINAL_NEWLINE;
    return rk_yajl_gen_status_ok;
}

rk_yajl_gen_status
rk_yajl_gen_get_buf(rk_yajl_gen g, const unsigned char ** buf,
                 unsigned int * len)
{
    if (g->print != (rk_yajl_print_t)&rk_yajl_buf_append) return rk_yajl_gen_no_buf;
    *buf = rk_yajl_buf_data((rk_yajl_buf)g->ctx);
    *len = rk_yajl_buf_len((rk_yajl_buf)g->ctx);
    return rk_yajl_gen_status_ok;
}

void
rk_yajl_gen_clear(rk_yajl_gen g)
{
    if (g->print == (rk_yajl_print_t)&rk_yajl_buf_append) rk_yajl_buf_clear((rk_yajl_buf)g->ctx);
}
