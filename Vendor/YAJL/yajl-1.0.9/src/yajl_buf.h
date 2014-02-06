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

#ifndef __YAJL_BUF_H__
#define __YAJL_BUF_H__

#include "api/yajl_common.h"
#include "yajl_alloc.h"

/*
 * Implementation/performance notes.  If this were moved to a header
 * only implementation using #define's where possible we might be 
 * able to sqeeze a little performance out of the guy by killing function
 * call overhead.  YMMV.
 */

/**
 * rk_yajl_buf is a buffer with exponential growth.  the buffer ensures that
 * you are always null padded.
 */
typedef struct rk_yajl_buf_t * rk_yajl_buf;

/* allocate a new buffer */
rk_yajl_buf rk_yajl_buf_alloc(rk_yajl_alloc_funcs * alloc);

/* free the buffer */
void rk_yajl_buf_free(rk_yajl_buf buf);

/* append a number of bytes to the buffer */
void rk_yajl_buf_append(rk_yajl_buf buf, const void * data, unsigned int len);

/* empty the buffer */
void rk_yajl_buf_clear(rk_yajl_buf buf);

/* get a pointer to the beginning of the buffer */
const unsigned char * rk_yajl_buf_data(rk_yajl_buf buf);

/* get the length of the buffer */
unsigned int rk_yajl_buf_len(rk_yajl_buf buf);

/* truncate the buffer */
void rk_yajl_buf_truncate(rk_yajl_buf buf, unsigned int len);

#endif
