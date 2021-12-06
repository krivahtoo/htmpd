#ifndef __SERVER_H__
#define __SERVER_H__

#include "mongoose.h"

struct embedded_file {
  const char *name;
  const unsigned char *data;
  const char *mimetype;
  size_t size;
};

const struct embedded_file *find_embedded_file(const char *name);
void http_callback(struct mg_connection *c, int ev, void *ev_data, void *fn_data);

#endif

