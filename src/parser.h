#ifndef PARSER_H_
#define PARSER_H_
#include <stddef.h>

// @param key - string to match or null
// @param klen - key length (or 0), or if null key then len is the array offset value
// @param json - json object or array
// @param jlen - length of json
// @param vlen - where to store return value length
// @return pointer to value and sets len to value length, or 0 if not found
// any parse error will set vlen to the position of the error
const char *parse(const char *key, size_t klen, const char *json, size_t jlen,
                 size_t *vlen);

#endif
