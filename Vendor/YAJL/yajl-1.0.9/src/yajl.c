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

#include "api/yajl_parse.h"
#include "yajl_lex.h"
#include "yajl_parser.h"
#include "yajl_alloc.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

const char *
rk_yajl_status_to_string(rk_yajl_status stat)
{
    const char * statStr = "unknown";
    switch (stat) {
        case rk_yajl_status_ok:
            statStr = "ok, no error";
            break;
        case rk_yajl_status_client_canceled:
            statStr = "client canceled parse";
            break;
        case rk_yajl_status_insufficient_data:
            statStr = "eof was met before the parse could complete";
            break;
        case rk_yajl_status_error:
            statStr = "parse error";
            break;
    }
    return statStr;
}

rk_yajl_handle
rk_yajl_alloc(const rk_yajl_callbacks * callbacks,
           const rk_yajl_parser_config * config,
           const rk_yajl_alloc_funcs * afs,
           void * ctx)
{
    unsigned int allowComments = 0;
    unsigned int validateUTF8 = 0;
    rk_yajl_handle hand = NULL;
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

    hand = (rk_yajl_handle) YA_MALLOC(afs, sizeof(struct rk_yajl_handle_t));

    /* copy in pointers to allocation routines */
    memcpy((void *) &(hand->alloc), (void *) afs, sizeof(rk_yajl_alloc_funcs));

    if (config != NULL) {
        allowComments = config->allowComments;
        validateUTF8 = config->checkUTF8;
    }

    hand->callbacks = callbacks;
    hand->ctx = ctx;
    hand->lexer = rk_yajl_lex_alloc(&(hand->alloc), allowComments, validateUTF8);
    hand->bytesConsumed = 0;
    hand->decodeBuf = rk_yajl_buf_alloc(&(hand->alloc));
    rk_yajl_bs_init(hand->stateStack, &(hand->alloc));

    rk_yajl_bs_push(hand->stateStack, rk_yajl_state_start);    

    return hand;
}

void
rk_yajl_free(rk_yajl_handle handle)
{
    rk_yajl_bs_free(handle->stateStack);
    rk_yajl_buf_free(handle->decodeBuf);
    rk_yajl_lex_free(handle->lexer);
    YA_FREE(&(handle->alloc), handle);
}

rk_yajl_status
rk_yajl_parse(rk_yajl_handle hand, const unsigned char * jsonText,
           unsigned int jsonTextLen)
{
    rk_yajl_status status;
    status = rk_yajl_do_parse(hand, jsonText, jsonTextLen);
    return status;
}

rk_yajl_status
rk_yajl_parse_complete(rk_yajl_handle hand)
{
    /* The particular case we want to handle is a trailing number.
     * Further input consisting of digits could cause our interpretation
     * of the number to change (buffered "1" but "2" comes in).
     * A very simple approach to this is to inject whitespace to terminate
     * any number in the lex buffer.
     */
    return rk_yajl_parse(hand, (const unsigned char *)" ", 1);
}

unsigned char *
rk_yajl_get_error(rk_yajl_handle hand, int verbose,
               const unsigned char * jsonText, unsigned int jsonTextLen)
{
    return rk_yajl_render_error_string(hand, jsonText, jsonTextLen, verbose);
}

unsigned int
rk_yajl_get_bytes_consumed(rk_yajl_handle hand)
{
    if (!hand) return 0;
    else return hand->bytesConsumed;
}


void
rk_yajl_free_error(rk_yajl_handle hand, unsigned char * str)
{
    /* use memory allocation functions if set */
    YA_FREE(&(hand->alloc), str);
}

/* XXX: add utility routines to parse from file */
