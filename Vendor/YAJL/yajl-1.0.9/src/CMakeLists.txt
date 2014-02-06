# Copyright 2010, Lloyd Hilaiel.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
# 
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
# 
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in
#     the documentation and/or other materials provided with the
#     distribution.
# 
#  3. Neither the name of Lloyd Hilaiel nor the names of its
#     contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
# IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

SET (SRCS rk_yajl.c rk_yajl_lex.c rk_yajl_parser.c rk_yajl_buf.c
          rk_yajl_encode.c rk_yajl_gen.c rk_yajl_alloc.c)
SET (HDRS rk_yajl_parser.h rk_yajl_lex.h rk_yajl_buf.h rk_yajl_encode.h rk_yajl_alloc.h)
SET (PUB_HDRS api/rk_yajl_parse.h api/rk_yajl_gen.h api/rk_yajl_common.h)

# useful when fixing lexer bugs.
#ADD_DEFINITIONS(-DYAJL_LEXER_DEBUG)

# Ensure defined when building YAJL (as opposed to using it from
# another project).  Used to ensure correct function export when
# building win32 DLL.
ADD_DEFINITIONS(-DYAJL_BUILD)

# set up some paths
SET (libDir ${CMAKE_CURRENT_BINARY_DIR}/../${YAJL_DIST_NAME}/lib)
SET (incDir ${CMAKE_CURRENT_BINARY_DIR}/../${YAJL_DIST_NAME}/include/rk_yajl)

# set the output path for libraries
SET(LIBRARY_OUTPUT_PATH ${libDir})

ADD_LIBRARY(rk_yajl_s STATIC ${SRCS} ${HDRS} ${PUB_HDRS})

ADD_LIBRARY(rk_yajl SHARED ${SRCS} ${HDRS} ${PUB_HDRS})

#### setup shared library version number
SET_TARGET_PROPERTIES(rk_yajl PROPERTIES
                      DEFINE_SYMBOL YAJL_SHARED
                      SOVERSION ${YAJL_MAJOR}
                      VERSION ${YAJL_MAJOR}.${YAJL_MINOR}.${YAJL_MICRO})

#### build up an sdk as a post build step

# create some directories
FILE(MAKE_DIRECTORY ${libDir})
FILE(MAKE_DIRECTORY ${incDir})

# copy public headers to output directory
FOREACH (header ${PUB_HDRS})
  SET (header ${CMAKE_CURRENT_SOURCE_DIR}/${header})

  EXEC_PROGRAM(${CMAKE_COMMAND} ARGS -E copy_if_different ${header} ${incDir})

  ADD_CUSTOM_COMMAND(TARGET rk_yajl_s POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different ${header} ${incDir})
ENDFOREACH (header ${PUB_HDRS})

INCLUDE_DIRECTORIES(${incDir}/..)

IF(NOT WIN32)
  INSTALL(TARGETS rk_yajl LIBRARY DESTINATION lib)
  INSTALL(TARGETS rk_yajl_s ARCHIVE DESTINATION lib)
  INSTALL(DIRECTORY api/ DESTINATION include/rk_yajl)
ENDIF()
