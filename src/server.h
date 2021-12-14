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

const struct embedded_file *find_embedded_file(const char *name);
void http_callback(struct mg_connection *c, struct mg_http_message *hm);

#endif

