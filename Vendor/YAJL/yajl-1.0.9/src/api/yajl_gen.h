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

/**
 * \file rk_yajl_gen.h
 * Interface to YAJL's JSON generation facilities.
 */

#include "yajl_common.h"

#ifndef __YAJL_GEN_H__
#define __YAJL_GEN_H__

#ifdef __cplusplus
extern "C" {
#endif    
    /** generator status codes */
    typedef enum {
        /** no error */
        rk_yajl_gen_status_ok = 0,
        /** at a point where a map key is generated, a function other than
         *  rk_yajl_gen_string was called */
        rk_yajl_gen_keys_must_be_strings,
        /** YAJL's maximum generation depth was exceeded.  see
         *  YAJL_MAX_DEPTH */
        rk_yajl_max_depth_exceeded,
        /** A generator function (rk_yajl_gen_XXX) was called while in an error
         *  state */
        rk_yajl_gen_in_error_state,
        /** A complete JSON document has been generated */
        rk_yajl_gen_generation_complete,                
        /** rk_yajl_gen_double was passed an invalid floating point value
         *  (infinity or NaN). */
        rk_yajl_gen_invalid_number,
        /** A print callback was passed in, so there is no internal
         * buffer to get from */
        rk_yajl_gen_no_buf
    } rk_yajl_gen_status;

    /** an opaque handle to a generator */
    typedef struct rk_yajl_gen_t * rk_yajl_gen;

    /** a callback used for "printing" the results. */
    typedef void (*rk_yajl_print_t)(void * ctx,
                                 const char * str,
                                 unsigned int len);

    /** configuration structure for the generator */
    typedef struct {
        /** generate indented (beautiful) output */
        unsigned int beautify;
        /** an opportunity to define an indent string.  such as \\t or
         *  some number of spaces.  default is four spaces '    '.  This
         *  member is only relevant when beautify is true */
        const char * indentString;
    } rk_yajl_gen_config;

    /** allocate a generator handle
     *  \param config a pointer to a structure containing parameters which
     *                configure the behavior of the json generator
     *  \param allocFuncs an optional pointer to a structure which allows
     *                    the client to overide the memory allocation
     *                    used by rk_yajl.  May be NULL, in which case
     *                    malloc/free/realloc will be used.
     *
     *  \returns an allocated handle on success, NULL on failure (bad params)
     */
    YAJL_API rk_yajl_gen rk_yajl_gen_alloc(const rk_yajl_gen_config * config,
                                     const rk_yajl_alloc_funcs * allocFuncs);

    /** allocate a generator handle that will print to the specified
     *  callback rather than storing the results in an internal buffer.
     *  \param callback   a pointer to a printer function.  May be NULL
     *                    in which case, the results will be store in an
     *                    internal buffer.
     *  \param config     a pointer to a structure containing parameters
     *                    which configure the behavior of the json
     *                    generator.
     *  \param allocFuncs an optional pointer to a structure which allows
     *                    the client to overide the memory allocation
     *                    used by rk_yajl.  May be NULL, in which case
     *                    malloc/free/realloc will be used.
     *  \param ctx        a context pointer that will be passed to the
     *                    printer callback.
     *
     *  \returns an allocated handle on success, NULL on failure (bad params)
     */
    YAJL_API rk_yajl_gen rk_yajl_gen_alloc2(const rk_yajl_print_t callback,
                                      const rk_yajl_gen_config * config,
                                      const rk_yajl_alloc_funcs * allocFuncs,
                                      void * ctx);

    /** free a generator handle */    
    YAJL_API void rk_yajl_gen_free(rk_yajl_gen handle);

    YAJL_API rk_yajl_gen_status rk_yajl_gen_integer(rk_yajl_gen hand, long int number);
    /** generate a floating point number.  number may not be infinity or
     *  NaN, as these have no representation in JSON.  In these cases the
     *  generator will return 'rk_yajl_gen_invalid_number' */
    YAJL_API rk_yajl_gen_status rk_yajl_gen_double(rk_yajl_gen hand, double number);
    YAJL_API rk_yajl_gen_status rk_yajl_gen_number(rk_yajl_gen hand,
                                             const char * num,
                                             unsigned int len);
    YAJL_API rk_yajl_gen_status rk_yajl_gen_string(rk_yajl_gen hand,
                                             const unsigned char * str,
                                             unsigned int len);
    YAJL_API rk_yajl_gen_status rk_yajl_gen_null(rk_yajl_gen hand);
    YAJL_API rk_yajl_gen_status rk_yajl_gen_bool(rk_yajl_gen hand, int boolean);    
    YAJL_API rk_yajl_gen_status rk_yajl_gen_map_open(rk_yajl_gen hand);
    YAJL_API rk_yajl_gen_status rk_yajl_gen_map_close(rk_yajl_gen hand);
    YAJL_API rk_yajl_gen_status rk_yajl_gen_array_open(rk_yajl_gen hand);
    YAJL_API rk_yajl_gen_status rk_yajl_gen_array_close(rk_yajl_gen hand);

    /** access the null terminated generator buffer.  If incrementally
     *  outputing JSON, one should call rk_yajl_gen_clear to clear the
     *  buffer.  This allows stream generation. */
    YAJL_API rk_yajl_gen_status rk_yajl_gen_get_buf(rk_yajl_gen hand,
                                              const unsigned char ** buf,
                                              unsigned int * len);

    /** clear rk_yajl's output buffer, but maintain all internal generation
     *  state.  This function will not "reset" the generator state, and is
     *  intended to enable incremental JSON outputing. */
    YAJL_API void rk_yajl_gen_clear(rk_yajl_gen hand);

#ifdef __cplusplus
}
#endif    

#endif
