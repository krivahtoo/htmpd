/* SPDX-License-Identifier: GPL-3.0-only */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "mongoose.h"
#include "server.h"
#include "utils.h"

// Helper function to forward stream requests to mpd
static void forward_request(struct mg_http_message *hm,
                            struct mg_connection *c) {
  size_t i, max = sizeof(hm->headers) / sizeof(hm->headers[0]);
  struct mg_str host = mg_url_host(configs.stream_url);
  mg_printf(c, "%.*s\r\n",
            (int)(hm->proto.ptr + hm->proto.len - hm->message.ptr),
            hm->message.ptr);
  for (i = 0; i < max && hm->headers[i].name.len > 0; i++) {
    struct mg_str *k = &hm->headers[i].name, *v = &hm->headers[i].value;
    if (mg_strcmp(*k, mg_str("Host")) == 0)
      v = &host;
    mg_printf(c, "%.*s: %.*s\r\n", (int)k->len, k->ptr, (int)v->len, v->ptr);
  }
  mg_send(c, "\r\n", 2);
  mg_send(c, hm->body.ptr, hm->body.len);
  LOG(LL_DEBUG, ("FORWARDING: %.*s %.*s", (int)hm->method.len, hm->method.ptr,
                 (int)hm->uri.len, hm->uri.ptr));
}

// Helper function to send embeded files
void send_embedded_file(struct mg_connection *c,
                        const struct embedded_file *file) {
  mg_printf(c,
            "HTTP/1.1 %d %s\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n",
            200, "OK", file->mimetype, file->size);
  mg_send(c, file->data, file->size);
}

// Handle http requests for embedded files.
static void http_callback(struct mg_connection *c, struct mg_http_message *hm) {
  const struct embedded_file *req_file;
  if (!strncmp((char *)hm->uri.ptr, "/", hm->uri.len)) {
    req_file = find_embedded_file(mg_str("/index.html"));
  } else {
    req_file = find_embedded_file(hm->uri);
  }
  if (req_file) {
    send_embedded_file(c, req_file);
  } else {
    mg_http_reply(c, 404, "", "Not found");
  }
}

// Helper function to handle mpd stream connection and forward to client,
// acting as a reverse proxy server
static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
  if (ev == MG_EV_READ && fn_data != NULL) {
    // All incoming data from the backend, forward to the client
    mg_send((struct mg_connection *)fn_data, c->recv.buf, c->recv.len);
    mg_iobuf_del(&c->recv, 0, c->recv.len);
  }
  (void)ev_data;
}

// Entry point for http requests
void server_handler(struct mg_connection *c, int ev, void *ev_data,
                    void *fn_data) {
  struct mg_connection *c2 = fn_data;
  if (ev == MG_EV_OPEN) {
    // c->is_hexdumping = 1;
  } else if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *)ev_data;
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
      char out[100];
      char *uri = strndup((char *)hm->uri.ptr, hm->uri.len);
      urldecode(out, strtok(uri, "/"));

      char *tmp_img = "/tmp/.htmpd-%s-artwork.png";
      char *image = malloc(strlen(tmp_img) + strlen(out));
      sprintf(image, tmp_img, out);
      struct mg_http_serve_opts opts = {
          .mime_types = "png=image/png",
          .extra_headers = "Cache-Control: max-age=2592000\r\n"};
      if (access(image, F_OK) == 0) {
        LOG(LL_INFO, ("Serving cached %s", image));
        mg_http_serve_file(c, hm, image, &opts);
      } else {
        GdkPixbuf *scaled, *pixbuf = retrieve_artwork(configs.music_dir, out);
        if (pixbuf) {
          scaled =
              gdk_pixbuf_scale_simple(pixbuf, 500, 500, GDK_INTERP_BILINEAR);
          gdk_pixbuf_save(scaled, image, "png", NULL, NULL);
          mg_http_serve_file(c, hm, image, &opts);
          free(scaled);
          free(pixbuf);
        } else {
          mg_http_reply(c, 404, "", "Not found\n");
        }
      }
      free(image);
      free(uri);
    } else {
      if (args.serve) {
        // Serve static files from web_root.
        struct mg_http_serve_opts opts = {.root_dir = configs.web_root};
        mg_http_serve_dir(c, ev_data, &opts);
      } else {
        http_callback(c, hm);
      }
    }
  } else if (ev == MG_EV_WS_MSG) {
    mpd_callback(c, (struct mg_ws_message *)ev_data);
  } else if (ev == MG_EV_CLOSE) {
    if (c2 != NULL) {
      c2->is_closing = 1;
      c2->fn_data = NULL;
    }
  }
  (void)fn_data;
}
