/* SPDX-License-Identifier: GPL-3.0-only */

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "configator.h"

#include <stdbool.h>
#include <stdint.h>

struct config {
  char *host;
  char *music_dir;
  char *password;
  char *port;
  char *stream_url;
  char *web_host;
  char *web_port;
  char *web_root;
};

struct arg {
  char *help;
  int *verbose;
  int *version;
};

extern struct config configs;
extern struct arg args;

void config_load(char *cfg_path);
void args_load(int argc, const char **argv);
void args_help();
void config_defaults();
void args_defaults();

void config_free();

#endif
