/* SPDX-License-Identifier: GPL-3.0-only */

#ifndef __SERVER_H__
#define __SERVER_H__

#include "mongoose.h"

struct embedded_file {
  const char *name;
  const unsigned char *data;
  const char *mimetype;
  size_t size;
};

void server_handler(struct mg_connection *c, int ev, void *ev_data,
                           void *fn_data);
const struct embedded_file *find_embedded_file(const struct mg_str name);

#endif
