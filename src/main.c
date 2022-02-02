/* SPDX-License-Identifier: GPL-3.0-only */

#include <math.h>
#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <string.h>
#include <time.h>

#define  _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define MG_ENABLE_LOG 1
#include "mongoose.h"
#include "server.h"
#include "mpd_client.h"
#include "utils.h"
#include "config.h"

#include "version.h"


const char *cfg_path = NULL;

struct config configs;

struct arg args;

static bool run = true;
static char program[] = "htmpd";

static void sig_handler(int sig_num) {
  // (void) sig_num;
  printf("Caught signal %d\n\nShutting down.\n", sig_num);
  run = false;
}

static void forward_request(struct mg_http_message *hm, struct mg_connection *c) {
  size_t i, max = sizeof(hm->headers) / sizeof(hm->headers[0]);
  struct mg_str host = mg_url_host(configs.stream_url);
  mg_printf(c, "%.*s\r\n",
            (int) (hm->proto.ptr + hm->proto.len - hm->message.ptr),
            hm->message.ptr);
  for (i = 0; i < max && hm->headers[i].name.len > 0; i++) {
    struct mg_str *k = &hm->headers[i].name, *v = &hm->headers[i].value;
    if (mg_strcmp(*k, mg_str("Host")) == 0) v = &host;
    mg_printf(c, "%.*s: %.*s\r\n", (int) k->len, k->ptr, (int) v->len, v->ptr);
  }
  mg_send(c, "\r\n", 2);
  mg_send(c, hm->body.ptr, hm->body.len);
  LOG(LL_DEBUG, ("FORWARDING: %.*s %.*s", (int) hm->method.len, hm->method.ptr,
                 (int) hm->uri.len, hm->uri.ptr));
}

static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
  if (ev == MG_EV_READ && fn_data != NULL) {
    // All incoming data from the backend, forward to the client
    mg_send((struct mg_connection *) fn_data, c->recv.buf, c->recv.len);
    mg_iobuf_del(&c->recv, 0, c->recv.len);
  }
  (void) ev_data;
}

static void server_handler(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
  struct mg_connection *c2 = fn_data;
  if (ev == MG_EV_OPEN) {
    // c->is_hexdumping = 1;
  } else if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    if (mg_http_match_uri(hm, "/ws")) {
      // Upgrade to websocket. From now on, a connection is a full-duplex
      // Websocket connection, which will receive MG_EV_WS_MSG events.
      mg_ws_upgrade(c, hm, NULL);
    } else if (mg_http_match_uri(hm, "/rest")) {
      mg_http_reply(c, 200, "", "{\"result\": \"%s\"}\n", "Hello World");
    } else if (mg_http_match_uri(hm, "/stream.mp3")) {
      c2 = mg_connect(c->mgr, configs.stream_url, fn, c);
      if (c2 == NULL) {
        mg_error(c, "Cannot create backend connection");
      } else {
        if (mg_url_is_ssl(configs.stream_url)) {
          struct mg_tls_opts opts = {.ca = "ca.pem"};
          mg_tls_init(c2, &opts);
        }
        c->fn_data = c2;
        forward_request(hm, c2);
        // c2->is_hexdumping = 1;
      }
    } else if (mg_http_match_uri(hm, "/*.mp3")) {
      char *delim = " ";
      char out[100];
      char *uri = strtok((char *)hm->uri.ptr, delim);
      uri = strtok(uri, "/");
      urldecode(out, uri);

      char *tmp_img = "/tmp/.htmpd-%s-artwork.png";
      char *image = malloc(strlen(tmp_img) + strlen(out));
      sprintf(image, tmp_img, out);
      struct mg_http_serve_opts opts = {.mime_types = "png=image/png",
                                    .extra_headers = "Cache-Control: max-age=2592000\r\n"};
      if (access(image, F_OK) == 0) {
        LOG(LL_INFO, ("Serving cached %s", image));
        mg_http_serve_file(c, hm, image, &opts);
      } else {
        char *music_dir = malloc(strlen(getenv("HOME")) + strlen("/Music"));
        sprintf(music_dir, "%s/Music", getenv("HOME"));
        GdkPixbuf *pixbuf = retrieve_artwork(music_dir, out);
        if (pixbuf) {
          pixbuf = gdk_pixbuf_scale_simple(pixbuf, 500, 500, GDK_INTERP_BILINEAR);
          gdk_pixbuf_save(pixbuf, image, "png", NULL, NULL);
          mg_http_serve_file(c, hm, image, &opts);
        } else {
          mg_http_reply(c, 404, "", "Not found\n");
        }
        free(music_dir);
      }
      free(image);
    } else {
#ifdef USE_DYNAMIC_WEB_PAGE
      // Serve static files from web_root.
      struct mg_http_serve_opts opts = {.root_dir = configs.web_root};
      mg_http_serve_dir(c, ev_data, &opts);
#else
      (void) s_web_root;
      http_callback(c, hm);
#endif
    }
  } else if (ev == MG_EV_WS_MSG) {
    struct mg_ws_message *wm = (struct mg_ws_message *) ev_data;
    mpd_callback(c, wm);
  } else if (ev == MG_EV_CLOSE) {
    if (c2 != NULL) c2->is_closing = 1;
    c->fn_data = NULL;
  }
  (void) fn_data;
}

int main(int argc, const char **argv) {
  struct mg_mgr mgr;  // Event manager
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
  mg_mgr_init(&mgr);  // Initialise event manager
  LOG(LL_INFO, ("Starting web server on port %s", s_listen_on));
#ifdef USE_DYNAMIC_WEB_PAGE
  LOG(LL_INFO, ("Serving files from %s", configs.web_root));
#endif
  mg_http_listen(&mgr, s_listen_on, server_handler, NULL);  // Create HTTP listener
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

