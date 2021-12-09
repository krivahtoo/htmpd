#include <string.h>
#include <stdio.h>

#include "mongoose.h"
#include "server.h"

void send_embedded_file(struct mg_connection *c, char *headers, const struct embedded_file *file) {
  mg_printf(c, "HTTP/1.1 %d %s\r\n%sContent-Length: %d\r\n\r\n", 200,
            "OK", headers == NULL ? "" : headers, file->size);
  mg_send(c, file->data, file->size);
}

void http_callback(struct mg_connection *c, struct mg_http_message *hm) {
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
    send_embedded_file(c, headers, req_file);
  } else {
    mg_http_reply(c, 404, "", "Not found");
  }
}

