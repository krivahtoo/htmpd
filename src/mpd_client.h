/* SPDX-License-Identifier: GPL-3.0-only */

#ifndef __MPD_CLIENT_H__
#define __MPD_CLIENT_H__

#include <mpd/client.h>

#include "mongoose.h"

#define FILL_MPD_ERROR()                                                       \
  jim_set_string(&jim, "type", "error");                                       \
  jim_set_string(&jim, "message", mpd_connection_get_error_message(mpd.conn)); \
  mpd.conn_state = MPD_FAILURE;

enum mpd_cmds {
  MPD_GET_QUEUE,
  MPD_GET_BROWSE,
  MPD_GET_MPDHOST,
  MPD_ADD_TRACK,
  MPD_ADD_PLAY_TRACK,
  MPD_ADD_PLAYLIST,
  MPD_PLAY_TRACK,
  MPD_SAVE_QUEUE,
  MPD_RM_TRACK,
  MPD_RM_RANGE,
  MPD_RM_ALL,
  MPD_MOVE_TRACK,
  MPD_SEARCH,
  MPD_GET_CHANNELS,
  MPD_SEND_MESSAGE,
  MPD_SET_VOLUME,
  MPD_SET_PAUSE,
  MPD_SET_PLAY,
  MPD_SET_STOP,
  MPD_SET_SEEK,
  MPD_SET_NEXT,
  MPD_SET_PREV,
  MPD_SET_MPDHOST,
  MPD_SET_MPDPASS,
  MPD_UPDATE_DB,
  MPD_GET_OUTPUTS,
  MPD_TOGGLE_OUTPUT,
  MPD_TOGGLE_RANDOM,
  MPD_TOGGLE_CONSUME,
  MPD_TOGGLE_SINGLE,
  MPD_TOGGLE_CROSSFADE,
  MPD_TOGGLE_REPEAT,
  MPD_SEEK_CURRENT,
  MPD_GET_COMMANDS,
  MPD_AUTHORIZE
};

enum mpd_conn_states {
  MPD_DISCONNECTED,
  MPD_FAILURE,
  MPD_CONNECTED,
  MPD_RECONNECT,
  MPD_DISCONNECT
};

struct t_mpd {
  unsigned int port;
  char host[128];
  char *password;

  struct mpd_connection *conn;
  enum mpd_conn_states conn_state;

  const unsigned *version;
};

extern struct t_mpd mpd;

void mpd_poll(struct mg_mgr *mgr);
void mpd_search(struct mg_connection *c, const char *query);
void mpd_disconnect();

void mpd_callback(struct mg_connection *c, struct mg_ws_message *wm);

#endif
