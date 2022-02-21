/* SPDX-License-Identifier: GPL-3.0-only */

#ifndef __UTILS_H__
#define __UTILS_H__

#define _GNU_SOURCE

#include <regex.h>
#include <stdlib.h>

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libavformat/avformat.h>

#include "mongoose.h"
#include "mpd_client.h"

#define REGEX_ARTWORK "\\(folder\\|cover\\)\\.\\(jpg\\|png\\)"

void urldecode(char *dst, const char *src);
GdkPixbuf *retrieve_artwork(const char *music_dir, const char *uri);

#endif
