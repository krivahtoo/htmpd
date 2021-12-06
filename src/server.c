#include <string.h>
#include <stdio.h>

#include "server.h"

void http_callback(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
  if (ev == MG_EV_OPEN) {
    // c->is_hexdumping = 1;
  } else if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    if (mg_http_match_uri(hm, "/websocket")) {
      // Upgrade to websocket. From now on, a connection is a full-duplex
  // Websocket connection, which will receive MG_EV_WS_MSG events.
      mg_ws_upgrade(c, hm, NULL);
    } else if (mg_http_match_uri(hm, "/rest")) {
      mg_http_reply(c, 200, "", "{\"result\": \"%s\"}\n", "Hello World");
    } else {
      // NOTE: Somehow mongoose doesn't parse uri correctly.
      char *delim = " ";
      char *uri = strtok((char *)hm->uri.ptr, delim);
      // delim = "?";
      // uri = strtok(uri, delim);
      const struct embedded_file *req_file;
      if (!strcmp(uri, "/")) {
        req_file = find_embedded_file("/index.html");
      } else {
        req_file = find_embedded_file(uri);
      }
      printf("%s\n", uri);
      if (req_file) {
        char *headers = (char*)malloc(50 * sizeof(char));
        sprintf(headers, "Content-Type: %s\r\n", req_file->mimetype);
        mg_http_reply(c, 200, headers, (const char *)req_file->data);
      } else {
        mg_http_reply(c, 404, "", "Not found");
      }
    }
  } else if (ev == MG_EV_WS_MSG) {
    // Got websocket frame. Received data is wm->data. Echo it back!
    struct mg_ws_message *wm = (struct mg_ws_message *) ev_data;
    mg_ws_send(c, wm->data.ptr, wm->data.len, WEBSOCKET_OP_TEXT);
  }
  (void) fn_data;
}

