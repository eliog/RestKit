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
 * \file rk_yajl_parse.h
 * Interface to YAJL's JSON parsing facilities.
 */

#include "yajl_common.h"

#ifndef __YAJL_PARSE_H__
#define __YAJL_PARSE_H__

#ifdef __cplusplus
extern "C" {
#endif    
    /** error codes returned from this interface */
    typedef enum {
        /** no error was encountered */
        rk_yajl_status_ok,
        /** a client callback returned zero, stopping the parse */
        rk_yajl_status_client_canceled,
        /** The parse cannot yet complete because more json input text
         *  is required, call rk_yajl_parse with the next buffer of input text.
         *  (pertinent only when stream parsing) */
        rk_yajl_status_insufficient_data,
        /** An error occured during the parse.  Call rk_yajl_get_error for
         *  more information about the encountered error */
        rk_yajl_status_error
    } rk_yajl_status;

    /** attain a human readable, english, string for an error */
    YAJL_API const char * rk_yajl_status_to_string(rk_yajl_status code);

    /** an opaque handle to a parser */
    typedef struct rk_yajl_handle_t * rk_yajl_handle;

    /** rk_yajl is an event driven parser.  this means as json elements are
     *  parsed, you are called back to do something with the data.  The
     *  functions in this table indicate the various events for which
     *  you will be called back.  Each callback accepts a "context"
     *  pointer, this is a void * that is passed into the rk_yajl_parse
     *  function which the client code may use to pass around context.
     *
     *  All callbacks return an integer.  If non-zero, the parse will
     *  continue.  If zero, the parse will be canceled and
     *  rk_yajl_status_client_canceled will be returned from the parse.
     *
     *  Note about handling of numbers:
     *    rk_yajl will only convert numbers that can be represented in a double
     *    or a long int.  All other numbers will be passed to the client
     *    in string form using the rk_yajl_number callback.  Furthermore, if
     *    rk_yajl_number is not NULL, it will always be used to return numbers,
     *    that is rk_yajl_integer and rk_yajl_double will be ignored.  If
     *    rk_yajl_number is NULL but one of rk_yajl_integer or rk_yajl_double are
     *    defined, parsing of a number larger than is representable
     *    in a double or long int will result in a parse error.
     */
    typedef struct {
        int (* rk_yajl_null)(void * ctx);
        int (* rk_yajl_boolean)(void * ctx, int boolVal);
        int (* rk_yajl_integer)(void * ctx, long integerVal);
        int (* rk_yajl_double)(void * ctx, double doubleVal);
        /** A callback which passes the string representation of the number
         *  back to the client.  Will be used for all numbers when present */
        int (* rk_yajl_number)(void * ctx, const char * numberVal,
                            unsigned int numberLen);

        /** strings are returned as pointers into the JSON text when,
         * possible, as a result, they are _not_ null padded */
        int (* rk_yajl_string)(void * ctx, const unsigned char * stringVal,
                            unsigned int stringLen);

        int (* rk_yajl_start_map)(void * ctx);
        int (* rk_yajl_map_key)(void * ctx, const unsigned char * key,
                             unsigned int stringLen);
        int (* rk_yajl_end_map)(void * ctx);        

        int (* rk_yajl_start_array)(void * ctx);
        int (* rk_yajl_end_array)(void * ctx);        
    } rk_yajl_callbacks;
    
    /** configuration structure for the generator */
    typedef struct {
        /** if nonzero, javascript style comments will be allowed in
         *  the json input, both slash star and slash slash */
        unsigned int allowComments;
        /** if nonzero, invalid UTF8 strings will cause a parse
         *  error */
        unsigned int checkUTF8;
    } rk_yajl_parser_config;

    /** allocate a parser handle
     *  \param callbacks  a rk_yajl callbacks structure specifying the
     *                    functions to call when different JSON entities
     *                    are encountered in the input text.  May be NULL,
     *                    which is only useful for validation.
     *  \param config     configuration parameters for the parse.
     *  \param ctx        a context pointer that will be passed to callbacks.
     */
    YAJL_API rk_yajl_handle rk_yajl_alloc(const rk_yajl_callbacks * callbacks,
                                    const rk_yajl_parser_config * config,
                                    const rk_yajl_alloc_funcs * allocFuncs,
                                    void * ctx);

    /** free a parser handle */    
    YAJL_API void rk_yajl_free(rk_yajl_handle handle);

    /** Parse some json!
     *  \param hand - a handle to the json parser allocated with rk_yajl_alloc
     *  \param jsonText - a pointer to the UTF8 json text to be parsed
     *  \param jsonTextLength - the length, in bytes, of input text
     */
    YAJL_API rk_yajl_status rk_yajl_parse(rk_yajl_handle hand,
                                    const unsigned char * jsonText,
                                    unsigned int jsonTextLength);

    /** Parse any remaining buffered json.
     *  Since rk_yajl is a stream-based parser, without an explicit end of
     *  input, rk_yajl sometimes can't decide if content at the end of the
     *  stream is valid or not.  For example, if "1" has been fed in,
     *  rk_yajl can't know whether another digit is next or some character
     *  that would terminate the integer token.
     *
     *  \param hand - a handle to the json parser allocated with rk_yajl_alloc
     */
    YAJL_API rk_yajl_status rk_yajl_parse_complete(rk_yajl_handle hand);
    
    /** get an error string describing the state of the
     *  parse.
     *
     *  If verbose is non-zero, the message will include the JSON
     *  text where the error occured, along with an arrow pointing to
     *  the specific char.
     *
     *  \returns A dynamically allocated string will be returned which should
     *  be freed with rk_yajl_free_error 
     */
    YAJL_API unsigned char * rk_yajl_get_error(rk_yajl_handle hand, int verbose,
                                            const unsigned char * jsonText,
                                            unsigned int jsonTextLength);

    /**
     * get the amount of data consumed from the last chunk passed to YAJL.
     *
     * In the case of a successful parse this can help you understand if
     * the entire buffer was consumed (which will allow you to handle
     * "junk at end of input". 
     * 
     * In the event an error is encountered during parsing, this function
     * affords the client a way to get the offset into the most recent
     * chunk where the error occured.  0 will be returned if no error
     * was encountered.
     */
    YAJL_API unsigned int rk_yajl_get_bytes_consumed(rk_yajl_handle hand);

    /** free an error returned from rk_yajl_get_error */
    YAJL_API void rk_yajl_free_error(rk_yajl_handle hand, unsigned char * str);

#ifdef __cplusplus
}
#endif    

#endif
