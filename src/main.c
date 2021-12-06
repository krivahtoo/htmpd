#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#define  _GNU_SOURCE
#include <stdio.h>

#define MG_ENABLE_LOG 1
#define SARG_NO_FILE
#include "args.h"
#include "mongoose.h"
#include "server.h"

static sarg_opt opts[] = {
  {"h", "help", "show help text", BOOL, NULL},
  {"v", "verbose", "increase verbosity", BOOL, NULL},
  {"H", "host", "server host", STRING, NULL},
  {"p", "port", "server port", STRING, NULL},
  {"d", "dir", "root directory", STRING, NULL},
  {NULL, NULL, NULL, INT, NULL}
};

static const char *s_listen_on = "ws://0.0.0.0:8000";
static const char *s_web_root = ".";

int main(int argc, const char **argv)
{
  sarg_root root;
  sarg_result *res;
  struct mg_mgr mgr;  // Event manager

  int ret = sarg_init(&root, opts, "htmpd");
  assert(ret == SARG_ERR_SUCCESS);

  ret = sarg_parse(&root, argv, argc);
  if(ret != SARG_ERR_SUCCESS) {
    printf("Parsing failed\n");
    sarg_help_print(&root);
    sarg_destroy(&root);
    return -1;
  }

  // check if help flag was set
  ret = sarg_get(&root, "help", &res);
  // assert actually never fails,
  // if the option was initialized in sarg_init
  assert(ret == SARG_ERR_SUCCESS);

  if(res->bool_val) {
    sarg_help_print(&root);
    sarg_destroy(&root);
    return 0;
  }

  // check for verbosity
  ret = sarg_get(&root, "v", &res);
  assert(ret == SARG_ERR_SUCCESS);

  printf("verbosity set to %d\n", res->count);

  ret = sarg_get(&root, "host", &res);
  assert(ret == SARG_ERR_SUCCESS);
  if(res->count > 0) {
    printf("host: %s\n", res->str_val);
  }

  sarg_destroy(&root);
  mg_log_set("3");
  mg_mgr_init(&mgr);  // Initialise event manager
  printf("Starting WS listener on %s/websocket\n", s_listen_on);
  mg_http_listen(&mgr, s_listen_on, http_callback, NULL);  // Create HTTP listener
  for (;;) mg_mgr_poll(&mgr, 500);             // Infinite event loop
  mg_mgr_free(&mgr);

  return 0;
}

