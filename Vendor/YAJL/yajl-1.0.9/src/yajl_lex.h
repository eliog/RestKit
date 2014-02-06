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

#ifndef __YAJL_LEX_H__
#define __YAJL_LEX_H__

#include "api/yajl_common.h"

typedef enum {
    rk_yajl_tok_bool,         
    rk_yajl_tok_colon,
    rk_yajl_tok_comma,     
    rk_yajl_tok_eof,
    rk_yajl_tok_error,
    rk_yajl_tok_left_brace,     
    rk_yajl_tok_left_bracket,
    rk_yajl_tok_null,         
    rk_yajl_tok_right_brace,     
    rk_yajl_tok_right_bracket,

    /* we differentiate between integers and doubles to allow the
     * parser to interpret the number without re-scanning */
    rk_yajl_tok_integer, 
    rk_yajl_tok_double, 

    /* we differentiate between strings which require further processing,
     * and strings that do not */
    rk_yajl_tok_string,
    rk_yajl_tok_string_with_escapes,

    /* comment tokens are not currently returned to the parser, ever */
    rk_yajl_tok_comment
} rk_yajl_tok;

typedef struct rk_yajl_lexer_t * rk_yajl_lexer;

rk_yajl_lexer rk_yajl_lex_alloc(rk_yajl_alloc_funcs * alloc,
                          unsigned int allowComments,
                          unsigned int validateUTF8);

void rk_yajl_lex_free(rk_yajl_lexer lexer);

/**
 * run/continue a lex. "offset" is an input/output parameter.
 * It should be initialized to zero for a
 * new chunk of target text, and upon subsetquent calls with the same
 * target text should passed with the value of the previous invocation.
 *
 * the client may be interested in the value of offset when an error is
 * returned from the lexer.  This allows the client to render useful
n * error messages.
 *
 * When you pass the next chunk of data, context should be reinitialized
 * to zero.
 * 
 * Finally, the output buffer is usually just a pointer into the jsonText,
 * however in cases where the entity being lexed spans multiple chunks,
 * the lexer will buffer the entity and the data returned will be
 * a pointer into that buffer.
 *
 * This behavior is abstracted from client code except for the performance
 * implications which require that the client choose a reasonable chunk
 * size to get adequate performance.
 */
rk_yajl_tok rk_yajl_lex_lex(rk_yajl_lexer lexer, const unsigned char * jsonText,
                      unsigned int jsonTextLen, unsigned int * offset,
                      const unsigned char ** outBuf, unsigned int * outLen);

/** have a peek at the next token, but don't move the lexer forward */
rk_yajl_tok rk_yajl_lex_peek(rk_yajl_lexer lexer, const unsigned char * jsonText,
                       unsigned int jsonTextLen, unsigned int offset);


typedef enum {
    rk_yajl_lex_e_ok = 0,
    rk_yajl_lex_string_invalid_utf8,
    rk_yajl_lex_string_invalid_escaped_char,
    rk_yajl_lex_string_invalid_json_char,
    rk_yajl_lex_string_invalid_hex_char,
    rk_yajl_lex_invalid_char,
    rk_yajl_lex_invalid_string,
    rk_yajl_lex_missing_integer_after_decimal,
    rk_yajl_lex_missing_integer_after_exponent,
    rk_yajl_lex_missing_integer_after_minus,
    rk_yajl_lex_unallowed_comment
} rk_yajl_lex_error;

const char * rk_yajl_lex_error_to_string(rk_yajl_lex_error error);

/** allows access to more specific information about the lexical
 *  error when rk_yajl_lex_lex returns rk_yajl_tok_error. */
rk_yajl_lex_error rk_yajl_lex_get_error(rk_yajl_lexer lexer);

/** get the current offset into the most recently lexed json string. */
unsigned int rk_yajl_lex_current_offset(rk_yajl_lexer lexer);

/** get the number of lines lexed by this lexer instance */
unsigned int rk_yajl_lex_current_line(rk_yajl_lexer lexer);

/** get the number of chars lexed by this lexer instance since the last
 *  \n or \r */
unsigned int rk_yajl_lex_current_char(rk_yajl_lexer lexer);

#endif
