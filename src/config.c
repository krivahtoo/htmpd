/* SPDX-License-Identifier: GPL-3.0-only */

#include "configator.h"
#include "config.h"
#include "argoat.h"

#include <linux/limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define CONFIG_PATH ".config/htmpd/config.ini"

void handle_str(void* data, char** pars, const int pars_count) {
  (void)pars_count;
  if (*((char**)data) != NULL) {
    free(*((char**)data));
  }

  *((char**)data) = strdup(*pars);
}

void handle_path(void* data, char** pars, const int pars_count) {
  (void)pars_count;
  if (*((char**)data) != NULL) {
    free(*((char**)data));
  }

  if (*pars[0] == '~') {
    char res[PATH_MAX];
    strcpy(res, getenv("HOME"));
    strcat(res, pars[0] + 1);
    *((char**)data) = strdup(res);
  } else {
    *((char**)data) = strdup(*pars);
  }
}

void config_handle_char(void* data, char** pars, const int pars_count) {
  (void)pars_count;
  *((char*)data) = **pars;
}

void config_handle_bool(void* data, char** pars, const int pars_count) {
  (void)pars_count;
  *((bool*)data) = (strcmp("true", *pars) == 0 || strcmp("yes", *pars) == 0);
}

void args_handle_bool(void* data, char** pars, const int pars_count) {
  (void)pars;
  (void)pars_count;
  *((bool*) data) = true;
}

void args_handle_add(void* data, char** pars, const int pars_count) {
  (void)pars;
  (void)pars_count;
  if ((int *)data == NULL) {
    *((int*)data) = 0;
  }
  *((int*) data) = *((int*) data) + 1;
}

void handle_main(void* data, char** pars, const int pars_count) {
  (void)data;
  (void)pars;
  (void)pars_count;
  return;
}

void config_load(char *cfg_path) {
  if (cfg_path == NULL) {
    cfg_path = malloc(strlen(getenv("HOME")) + strlen(CONFIG_PATH) + strlen("/"));
    sprintf(cfg_path, "%s/%s", getenv("HOME"), CONFIG_PATH);
  }
  struct configator_param map_no_section[] = {
    {"host", &configs.host, handle_str},
    {"music_dir", &configs.music_dir, handle_path},
    {"password", &configs.password, handle_str},
    {"port", &configs.port, handle_str},
    {"web_host", &configs.web_host, handle_str},
    {"web_port", &configs.web_port, handle_str},
    {"web_root", &configs.web_root, handle_path},
  };
  uint16_t map_len[] = {7};
  struct configator_param* map[] = {
    map_no_section,
    // map_mpd_section
  };

  uint16_t sections_len = 0;
  struct configator_param* sections = NULL;

  struct configator config;
  config.map = map;
  config.map_len = map_len;
  config.sections = sections;
  config.sections_len = sections_len;

  configator(&config, cfg_path);
  free(cfg_path);
}

void args_load(int argc, const char **argv) {
  char* unflagged[2];

  const struct argoat_sprig sprigs[14] = {
    {NULL, 0, NULL, handle_main},
    {"H", 1, &configs.web_host, handle_str},
    {"P", 1, &configs.web_port, handle_str},
    {"V", 0, &args.version, args_handle_add },
    {"h", 0, &args.help, args_handle_bool },
    {"help", 0, &args.help, args_handle_bool },
    {"mpd-host", 1, &configs.host, handle_str},
    {"mpd-port", 1, &configs.port, handle_str},
    {"p", 1, &configs.port, handle_str},
    {"v", 0, &args.verbose, args_handle_add },
    {"verbose", 0, &args.verbose, args_handle_add },
    {"version", 0, &args.version, args_handle_add },
    {"web-host", 1, &configs.web_host, handle_str},
    {"web-port", 1, &configs.web_port, handle_str},
  };

  struct argoat args = {sprigs, 14, unflagged, 0, 2};

  argoat_graze(&args, argc, (char **)argv);
}

void args_help() {
  printf("Usage: htmpd [OPTION]...\n");
  printf("\n");
  printf("  -H, --web-host=HOSTNAME  hostname to bind to (default: 0.0.0.0)\n");
  printf("  -P, --web-port=PORT      port to bind to (default: 8000)\n");
  printf("  -h, --help               display this help and exit\n");
  printf("  -v, --verbose            increase verbosity\n");
  printf("  -V, --version            output version information and exit\n");
  printf("\n");
  printf("  -p, --mpd-port=PORT      port to connect to (default: 6600)\n");
  printf("  --mpd-host=HOSTNAME      hostname to connect to (default: 127.0.0.1)\n");
  printf("\n");
  printf("Report bugs to https://github.com/krivahtoo/htmpd/issues\n");
}

void config_defaults() {
  char* music_path = malloc(strlen(getenv("HOME")) + strlen("/Music"));
  sprintf(music_path, "%s/%s", getenv("HOME"), "Music");
  configs.host = strdup("127.0.0.1");
  configs.port = strdup("6600");
  configs.music_dir = strdup(music_path);
  configs.password = strdup("");
  configs.web_root = strdup("./dist");
  configs.stream_url = strdup("http://localhost:8008");
  configs.web_host = strdup("0.0.0.0");
  configs.web_port = strdup("8000");
  free(music_path);
}

void args_defaults() {
  args.help = false;
  args.verbose = 0;
  args.version = false;
}

void config_free() {
  free(configs.host);
  free(configs.port);
  free(configs.music_dir);
  free(configs.password);
  free(configs.web_root);
  free(configs.stream_url);
  free(configs.web_host);
  free(configs.web_port);
}
