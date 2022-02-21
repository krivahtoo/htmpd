/* SPDX-License-Identifier: GPL-3.0-only */

#include <assert.h>
#include <math.h>
#include <signal.h>
#include <stddef.h>
#include <string.h>
#include <time.h>

#define _GNU_SOURCE
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#define MG_ENABLE_LOG 1
#include "config.h"
#include "mpd_client.h"
#include "server.h"

#include "version.h"

const char *cfg_path = NULL;

struct config configs;

struct arg args;

static bool run = true;
static char program[] = "htmpd";

static void sig_handler(int sig_num) {
  printf("Caught signal %d\n\nShutting down.\n", sig_num);
  run = false;
}

int main(int argc, const char **argv) {
  struct mg_mgr mgr; // Event manager
  time_t current_timer = 0, last_timer = 0;
  char *verbose = malloc(sizeof(char) * 2);
  char *s_listen_on = malloc(sizeof(char) * 25);

  config_defaults();
  args_defaults();
  config_load(NULL);
  args_load(argc, argv);

  if (args.help) {
    printf("%s %s\nA lightweight modern MPD web client\n\n", program, VERSION);
    args_help();
    return 0;
  }
  if (args.version) {
    printf("%s %s\n", program, VERSION);
    return 0;
  }
  sprintf(verbose, "%d", args.verbose);

  sprintf(s_listen_on, "ws://%s:%s", configs.web_host, configs.web_port);

  // Set up signal handler
  signal(SIGINT, sig_handler);

  // Initialize mpd client
  strcpy(mpd.host, configs.host);
  mpd.port = strtol(configs.port, 0, 10);
  mpd.conn = NULL;

  mg_log_set(verbose);
  mg_mgr_init(&mgr); // Initialise event manager
  LOG(LL_INFO, ("Starting web server on port %s", s_listen_on));
  if (args.serve) {
    LOG(LL_INFO, ("Serving files from %s", configs.web_root));
  } else {
    LOG(LL_INFO, ("Serving embedded files"));
  }
  mg_http_listen(&mgr, s_listen_on, server_handler,
                 NULL); // Create HTTP listener
  while (run) {
    mg_mgr_poll(&mgr, 200);
    current_timer = time(NULL);
    if (current_timer - last_timer) {
      last_timer = current_timer;
      mpd_poll(&mgr);
    }
  }
  mpd_disconnect();
  mg_mgr_free(&mgr);
  config_free();
  free(verbose);

  return 0;
}
