/*
SPDX-License-Identifier: MIT

Copyright 2021 Alexey Kutepov <reximkut@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef JIM_H_
#define JIM_H_

#include <stddef.h>
#ifndef JIM_SCOPES_CAPACITY
#define JIM_SCOPES_CAPACITY 128
#endif // JIM_SCOPES_CAPACITY

typedef void *Jim_Sink;
typedef size_t (*Jim_Write)(const void *ptr, size_t size, size_t nmemb,
                            Jim_Sink sink);

typedef enum {
  JIM_OK = 0,
  JIM_WRITE_ERROR,
  JIM_SCOPES_OVERFLOW,
  JIM_SCOPES_UNDERFLOW,
  JIM_OUT_OF_SCOPE_KEY,
  JIM_SCOPE_MISMATCH,
  JIM_DOUBLE_KEY
} Jim_Error;

const char *jim_error_string(Jim_Error error);

typedef enum {
  JIM_ARRAY_SCOPE,
  JIM_OBJECT_SCOPE,
} Jim_Scope_Kind;

typedef struct {
  Jim_Scope_Kind kind;
  int tail;
  int key;
} Jim_Scope;

typedef struct {
  Jim_Sink sink;
  Jim_Write write;
  Jim_Error error;
  Jim_Scope scopes[JIM_SCOPES_CAPACITY];
  size_t scopes_size;
} Jim;

void jim_set_null(Jim *jim, const char *key);
void jim_set_bool(Jim *jim, const char *key, int boolean);
void jim_set_integer(Jim *jim, const char *key, long long int x);
void jim_set_float(Jim *jim, const char *key, double x, int precision);
void jim_set_string(Jim *jim, const char *key, const char *str);

void jim_null(Jim *jim);
void jim_bool(Jim *jim, int boolean);
void jim_integer(Jim *jim, long long int x);
void jim_float(Jim *jim, double x, int precision);
void jim_string(Jim *jim, const char *str);
void jim_string_sized(Jim *jim, const char *str, size_t size);

void jim_element_begin(Jim *jim);
void jim_element_end(Jim *jim);

void jim_array_begin(Jim *jim);
void jim_array_end(Jim *jim);

void jim_object_begin(Jim *jim);
void jim_member_key(Jim *jim, const char *str);
void jim_member_key_sized(Jim *jim, const char *str, size_t size);
void jim_object_end(Jim *jim);

#endif // JIM_H_
