/* SPDX-License-Identifier: GPL-3.0-only */

#include "mpd_client.h"
#include "jim.h"
#include "mongoose.h"
#include "parser.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct t_mpd mpd;

typedef struct {
  size_t size;
  char *data;
} Buffer;

void buffer_free(Buffer *buffer) {
  free(buffer->data);
  buffer->data = NULL;
  buffer->size = 0;
}

size_t buffer_write(const void *ptr, size_t size, size_t nmemb, Buffer *sink) {
  size_t new_size = sink->size + size * nmemb;
  sink->data = realloc(sink->data, new_size + 1);
  if (sink->data == NULL) {
    return 0;
  }
  memcpy(sink->data + sink->size, ptr, size * nmemb);
  sink->size = new_size;
  sink->data[sink->size] = '\0';
  return size * nmemb;
}

int get_cmd_id(const char *json, size_t len) {
  size_t vlen = 0;
  const char *val = parse("cmd_id", 6, json, len, &vlen);
  if (vlen == 0) {
    return -1;
  }
  int i = (int)strtol(val, (char **)NULL, 10);
  return i;
}

const char *_get_key(struct mg_str json, const char *key) {
  size_t vlen = 0;
  const char *val = parse(key, strlen(key), json.ptr, json.len, &vlen);
  if (vlen == 0) {
    return NULL;
  }
  return val;
}

size_t _get_number(struct mg_str json, const char *key) {
  const char *val = _get_key(json, key);
  if (val != NULL) {
    if (strncmp(val, "true", 4) == 0) {
      return 1;
    } else {
      return (size_t)strtol(val, (char **)NULL, 10);
    }
  }
  return 0;
}

/*** Get title or fallback to uri ***/
char *get_title(struct mpd_song const *song) {
  char *str;

  str = (char *)mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
  if (str == NULL) {
    str = (char *)mpd_song_get_uri(song);
  }

  return str;
}

void send_queue(struct mg_connection *c, size_t offset, size_t limit) {
  struct mpd_entity *entity;
  size_t loc_limit;
  size_t i = 0;
  size_t j = 0;
  unsigned long totalTime = 0;

  Buffer buffer = {.data = NULL, .size = 0};
  Jim jim = {
      .sink = &buffer,
      .write = (Jim_Write)buffer_write,
  };

  jim_object_begin(&jim);

  if (limit == 0) {
    loc_limit = 20;
  } else {
    loc_limit = (size_t)limit;
  }
  if (!mpd_send_list_queue_meta(mpd.conn)) {
    FILL_MPD_ERROR();
    goto send_queue;
  }
  jim_member_key(&jim, "queue");
  jim_array_begin(&jim);
  while ((entity = mpd_recv_entity(mpd.conn)) != NULL) {
    if (i >= offset && j < loc_limit) {
      const struct mpd_song *curr_song = mpd_entity_get_song(entity);
      if (mpd_entity_get_type(entity) == MPD_ENTITY_TYPE_SONG) {
        curr_song = mpd_entity_get_song(entity);

        jim_object_begin(&jim);
        jim_set_string(&jim, "title", get_title(curr_song));
        jim_set_string(&jim, "artist",
                       mpd_song_get_tag(curr_song, MPD_TAG_ARTIST, 0));
        jim_set_string(&jim, "album",
                       mpd_song_get_tag(curr_song, MPD_TAG_ALBUM, 0));
        jim_set_string(&jim, "genre",
                       mpd_song_get_tag(curr_song, MPD_TAG_GENRE, 0));
        jim_set_string(&jim, "uri", mpd_song_get_uri(curr_song));
        jim_set_integer(&jim, "duration", mpd_song_get_duration(curr_song));
        jim_set_integer(&jim, "id", mpd_song_get_id(curr_song));
        jim_set_integer(&jim, "pos", mpd_song_get_pos(curr_song));
        jim_set_string(&jim, "year",
                       mpd_song_get_tag(curr_song, MPD_TAG_DATE, 0));
        jim_object_end(&jim);

        totalTime += mpd_song_get_duration(curr_song);
      }
      j++;
    }
    mpd_entity_free(entity);
    i++;
  }
  jim_array_end(&jim);
  jim_set_string(&jim, "type", "queue");
  jim_set_integer(&jim, "limit", loc_limit);
  jim_set_integer(&jim, "offset", offset);
  jim_set_integer(&jim, "total", i);
  jim_set_integer(&jim, "totalTime", totalTime);

send_queue : {
  jim_object_end(&jim);
  if (jim.error != JIM_OK) {
    fprintf(stderr, "ERROR: could not serialize json properly: %s\n",
            jim_error_string(jim.error));
    return;
  }
  mg_ws_send(c, buffer.data, buffer.size, WEBSOCKET_OP_TEXT);
}
  buffer_free(&buffer);
}

void send_status(struct mg_connection *c) {
  Buffer buffer = {.data = NULL, .size = 0};
  Jim jim = {
      .sink = &buffer,
      .write = (Jim_Write)buffer_write,
  };

  jim_object_begin(&jim);

  if (!mpd_send_status(mpd.conn)) {
    FILL_MPD_ERROR();
    goto send_status;
  }
  struct mpd_status *status = mpd_recv_status(mpd.conn);
  if (status == NULL) {
    FILL_MPD_ERROR();
    goto send_status;
  }
  jim_set_integer(&jim, "volume", mpd_status_get_volume(status));
  jim_set_integer(&jim, "repeat", mpd_status_get_repeat(status));
  jim_set_integer(&jim, "random", mpd_status_get_random(status));
  jim_set_integer(&jim, "single", mpd_status_get_single(status));
  jim_set_integer(&jim, "consume", mpd_status_get_consume(status));
  jim_set_integer(&jim, "playlistLength", mpd_status_get_queue_length(status));
  jim_set_integer(&jim, "playlistSong", mpd_status_get_song_pos(status));
  jim_set_integer(&jim, "state", mpd_status_get_state(status));
  jim_set_integer(&jim, "elapsedTime", mpd_status_get_elapsed_ms(status));
  jim_set_integer(&jim, "totalTime", mpd_status_get_total_time(status));
  jim_set_integer(&jim, "crossfade", mpd_status_get_crossfade(status));
  jim_set_integer(&jim, "bitrate", mpd_status_get_kbit_rate(status));
  jim_set_integer(&jim, "current", mpd_status_get_song_id(status));
  jim_set_integer(&jim, "mixrampdb", mpd_status_get_mixrampdb(status));
  jim_set_integer(&jim, "mixrampdelay", mpd_status_get_mixrampdelay(status));
  jim_set_integer(&jim, "nextId", mpd_status_get_next_song_id(status));
  jim_set_integer(&jim, "nextPos", mpd_status_get_next_song_pos(status));
  jim_set_integer(&jim, "version", mpd_status_get_queue_version(status));

  if (mpd_status_get_state(status) == MPD_STATE_PLAY ||
      mpd_status_get_state(status) == MPD_STATE_PAUSE) {
    const struct mpd_song *curr_song = mpd_run_current_song(mpd.conn);
    if (curr_song != NULL) {
      jim_member_key(&jim, "song");
      jim_object_begin(&jim);
      jim_set_string(&jim, "title", get_title(curr_song));
      jim_set_string(&jim, "artist",
                     mpd_song_get_tag(curr_song, MPD_TAG_ARTIST, 0));
      jim_set_string(&jim, "album",
                     mpd_song_get_tag(curr_song, MPD_TAG_ALBUM, 0));
      jim_set_string(&jim, "genre",
                     mpd_song_get_tag(curr_song, MPD_TAG_GENRE, 0));
      jim_set_integer(&jim, "duration", mpd_song_get_duration(curr_song));
      jim_set_integer(&jim, "id", mpd_song_get_id(curr_song));
      jim_set_integer(&jim, "pos", mpd_song_get_pos(curr_song));
      jim_set_string(&jim, "uri", mpd_song_get_uri(curr_song));
      jim_set_string(&jim, "year",
                     mpd_song_get_tag(curr_song, MPD_TAG_DATE, 0));
      jim_object_end(&jim);
    }
  }
  jim_set_string(&jim, "type", "status");

send_status : {
  jim_object_end(&jim);
  if (jim.error != JIM_OK) {
    fprintf(stderr, "ERROR: could not serialize json properly: %s\n",
            jim_error_string(jim.error));
    return;
  }
  mg_ws_send(c, buffer.data, buffer.size, WEBSOCKET_OP_TEXT);
}
  buffer_free(&buffer);
}

void send_output(struct mg_connection *c) {
  Buffer buffer = {.data = NULL, .size = 0};
  Jim jim = {
      .sink = &buffer,
      .write = (Jim_Write)buffer_write,
  };

  jim_object_begin(&jim);
  if (!mpd_send_outputs(mpd.conn)) {
    FILL_MPD_ERROR();
    goto send_outputs;
  }
  struct mpd_output *output;
  jim_member_key(&jim, "outputs");
  jim_array_begin(&jim);
  while ((output = mpd_recv_output(mpd.conn)) != NULL) {
    jim_object_begin(&jim);
    jim_set_integer(&jim, "id", mpd_output_get_id(output));
    jim_set_string(&jim, "name", mpd_output_get_name(output));
    jim_set_integer(&jim, "enabled", mpd_output_get_enabled(output));
    jim_object_end(&jim);
    mpd_output_free(output);
  }
  jim_array_end(&jim);
  jim_set_string(&jim, "type", "outputs");

send_outputs : {
  jim_object_end(&jim);
  if (jim.error != JIM_OK) {
    fprintf(stderr, "ERROR: could not serialize json properly: %s\n",
            jim_error_string(jim.error));
    return;
  }
  mg_ws_send(c, buffer.data, buffer.size, WEBSOCKET_OP_TEXT);
}
  buffer_free(&buffer);
}

void send_browse(struct mg_connection *c, const char *uri, size_t offset,
                 size_t limit) {
  size_t loc_limit = (size_t)limit;
  size_t i = 0;
  size_t j = 0;
  struct {
    int dirs;
    int files;
    int playlists;
  } count = {0, 0, 0};
  Buffer buffer = {.data = NULL, .size = 0};
  Jim jim = {
      .sink = &buffer,
      .write = (Jim_Write)buffer_write,
  };

  jim_object_begin(&jim);

  if (loc_limit <= 0) {
    loc_limit = 20;
  }

  if (!mpd_send_list_meta(mpd.conn, uri)) {
    FILL_MPD_ERROR();
    goto send_browse;
  }
  struct mpd_entity *entity;
  jim_member_key(&jim, "browse");
  jim_array_begin(&jim);
  while ((entity = mpd_recv_entity(mpd.conn)) != NULL) {
    switch (mpd_entity_get_type(entity)) {
    case MPD_ENTITY_TYPE_DIRECTORY:
      count.dirs++;
      break;
    case MPD_ENTITY_TYPE_SONG:
      count.files++;
      break;
    case MPD_ENTITY_TYPE_PLAYLIST:
      count.playlists++;
      break;
    default:
      break;
    }
    if (i >= offset && j < loc_limit) {
      switch (mpd_entity_get_type(entity)) {
      case MPD_ENTITY_TYPE_DIRECTORY: {
        jim_object_begin(&jim);
        jim_set_string(&jim, "type", "directory");
        jim_set_string(
            &jim, "path",
            mpd_directory_get_path(mpd_entity_get_directory(entity)));
        jim_set_integer(
            &jim, "last_modified",
            mpd_directory_get_last_modified(mpd_entity_get_directory(entity)));
        jim_object_end(&jim);
      } break;
      case MPD_ENTITY_TYPE_SONG: {
        jim_object_begin(&jim);
        jim_set_string(&jim, "type", "file");
        jim_set_string(&jim, "title", get_title(mpd_entity_get_song(entity)));
        jim_set_string(
            &jim, "artist",
            mpd_song_get_tag(mpd_entity_get_song(entity), MPD_TAG_ARTIST, 0));
        jim_set_string(
            &jim, "album",
            mpd_song_get_tag(mpd_entity_get_song(entity), MPD_TAG_ALBUM, 0));
        jim_set_string(
            &jim, "genre",
            mpd_song_get_tag(mpd_entity_get_song(entity), MPD_TAG_GENRE, 0));
        jim_set_integer(&jim, "duration",
                        mpd_song_get_duration(mpd_entity_get_song(entity)));
        jim_set_string(&jim, "uri",
                       mpd_song_get_uri(mpd_entity_get_song(entity)));
        jim_set_string(
            &jim, "year",
            mpd_song_get_tag(mpd_entity_get_song(entity), MPD_TAG_DATE, 0));
        jim_object_end(&jim);
      } break;
      case MPD_ENTITY_TYPE_PLAYLIST: {
        jim_object_begin(&jim);
        jim_set_string(&jim, "type", "playlist");
        jim_set_string(&jim, "path",
                       mpd_playlist_get_path(mpd_entity_get_playlist(entity)));
        jim_set_integer(
            &jim, "last_modified",
            mpd_playlist_get_last_modified(mpd_entity_get_playlist(entity)));
        jim_object_end(&jim);
      } break;
      default:
        break;
      }
      j++;
    }
    mpd_entity_free(entity);
    i++;
  }
  jim_array_end(&jim);
  jim_set_string(&jim, "type", "browse");
  jim_set_integer(&jim, "limit", loc_limit);
  jim_set_integer(&jim, "offset", offset);
  jim_set_integer(&jim, "total", i);
  jim_set_integer(&jim, "dirsCount", count.dirs);
  jim_set_integer(&jim, "filesCount", count.files);
  jim_set_integer(&jim, "playlistsCount", count.playlists);

send_browse : {
  jim_object_end(&jim);
  if (jim.error != JIM_OK) {
    fprintf(stderr, "ERROR: could not serialize json properly: %s\n",
            jim_error_string(jim.error));
    return;
  }
  mg_ws_send(c, buffer.data, buffer.size, WEBSOCKET_OP_TEXT);
}
  buffer_free(&buffer);
}

void send_stats(struct mg_connection *c) {
  Buffer buffer = {.data = NULL, .size = 0};
  Jim jim = {
      .sink = &buffer,
      .write = (Jim_Write)buffer_write,
  };

  jim_object_begin(&jim);
  struct mpd_stats *stats = mpd_run_stats(mpd.conn);
  if (stats != NULL) {
    jim_set_string(&jim, "type", "stats");
    jim_set_integer(&jim, "artistsCount",
                    mpd_stats_get_number_of_artists(stats));
    jim_set_integer(&jim, "songsCount", mpd_stats_get_number_of_songs(stats));
    jim_set_integer(&jim, "albumsCount", mpd_stats_get_number_of_albums(stats));
    jim_set_integer(&jim, "uptime", mpd_stats_get_uptime(stats));
    jim_set_integer(&jim, "playTime", mpd_stats_get_play_time(stats));
    jim_set_integer(&jim, "dbPlayTime", mpd_stats_get_db_play_time(stats));
    jim_set_integer(&jim, "dbUpdateTime", mpd_stats_get_db_update_time(stats));

    mpd_stats_free(stats);
  } else {
    FILL_MPD_ERROR();
  }
  char *version = malloc(sizeof(char) * 10 + 1);
  sprintf(version, "%d.%d.%d", mpd.version[0], mpd.version[1], mpd.version[2]);
  jim_set_string(&jim, "version", version);
  free(version);

  jim_object_end(&jim);
  if (jim.error != JIM_OK) {
    fprintf(stderr, "ERROR: could not serialize json properly: %s\n",
            jim_error_string(jim.error));
    return;
  }
  mg_ws_send(c, buffer.data, buffer.size, WEBSOCKET_OP_TEXT);
  buffer_free(&buffer);
}

void send_channels(struct mg_connection *c) {
  Buffer buffer = {.data = NULL, .size = 0};
  Jim jim = {
      .sink = &buffer,
      .write = (Jim_Write)buffer_write,
  };

  jim_object_begin(&jim);
  if (mpd_send_channels(mpd.conn)) {
    struct mpd_pair *channel;
    jim_member_key(&jim, "channels");
    jim_array_begin(&jim);
    while ((channel = mpd_recv_channel_pair(mpd.conn)) != NULL) {
      jim_string(&jim, channel->value);
      mpd_return_pair(mpd.conn, channel);
    }
    jim_array_end(&jim);
    jim_set_string(&jim, "type", "channels");
  } else {
    FILL_MPD_ERROR();
  }
  jim_object_end(&jim);
  if (jim.error != JIM_OK) {
    fprintf(stderr, "ERROR: could not serialize json properly: %s\n",
            jim_error_string(jim.error));
    return;
  }
  mg_ws_send(c, buffer.data, buffer.size, WEBSOCKET_OP_TEXT);
  buffer_free(&buffer);
}

void mpd_poll(struct mg_mgr *mgr) {
  struct mg_connection *c, *tmp;
  Buffer buffer = {.data = NULL, .size = 0};
  Jim jim = {
      .sink = &buffer,
      .write = (Jim_Write)buffer_write,
  };

  for (c = mgr->conns; c != NULL; c = tmp) {
    tmp = c->next;
    if (mpd.conn == NULL) {
      mpd.conn_state = MPD_DISCONNECTED;
    } else {
      // Report errors
      if (mpd_connection_get_error(mpd.conn) != MPD_ERROR_SUCCESS) {
        LOG(LL_ERROR, ("MPD connection failed: %s",
                       mpd_connection_get_error_message(mpd.conn)));
        if (c->is_websocket) {
          jim_object_begin(&jim);
          jim_set_string(&jim, "type", "error");
          jim_set_string(&jim, "message",
                         mpd_connection_get_error_message(mpd.conn));
          jim_object_end(&jim);
          if (jim.error != JIM_OK) {
            fprintf(stderr, "ERROR: could not serialize json properly: %s\n",
                    jim_error_string(jim.error));
            return;
          }
          mg_ws_send(c, buffer.data, buffer.size, WEBSOCKET_OP_TEXT);
          buffer_free(&buffer);
        }
        // try recovering
        if (!mpd_connection_clear_error(mpd.conn)) {
          mpd.conn_state = MPD_FAILURE;
        }
      }
    }
    switch (mpd.conn_state) {
    case MPD_DISCONNECTED:
      LOG(LL_DEBUG,
          ("MPD disconnected, reconnecting to %s:%d...", mpd.host, mpd.port));
      mpd.conn = mpd_connection_new(mpd.host, mpd.port, 5000);
      if (mpd.conn == NULL) {
        LOG(LL_ERROR, ("MPD connection failed"));
        mpd.conn_state = MPD_DISCONNECTED;
      } else {
        if (mpd_connection_get_error(mpd.conn) != MPD_ERROR_SUCCESS) {
          LOG(LL_ERROR, ("MPD connection failed: %s",
                         mpd_connection_get_error_message(mpd.conn)));
          mpd.conn_state = MPD_DISCONNECTED;
        } else {
          if (mpd.password != NULL) {
            if (!mpd_run_password(mpd.conn, mpd.password)) {
              LOG(LL_ERROR, ("MPD password failed: %s",
                             mpd_connection_get_error_message(mpd.conn)));
              mpd.conn_state = MPD_DISCONNECTED;
              return;
            }
          }
          LOG(LL_DEBUG, ("MPD connected"));
          mpd.version = mpd_connection_get_server_version(mpd.conn);
          mpd.conn_state = MPD_CONNECTED;
          if (!c->is_websocket || c == NULL) {
            continue;
          }
          char *version = malloc(sizeof(char) * 10 + 1);

          sprintf(version, "%d.%d.%d", mpd.version[0], mpd.version[1],
                  mpd.version[2]);
          jim_object_begin(&jim);
          jim_set_string(&jim, "type", "connected");
          jim_set_string(&jim, "version", version);
          jim_object_end(&jim);
          if (jim.error != JIM_OK) {
            fprintf(stderr, "ERROR: could not serialize json properly: %s\n",
                    jim_error_string(jim.error));
            return;
          }
          mg_ws_send(c, buffer.data, buffer.size, WEBSOCKET_OP_TEXT);
          buffer_free(&buffer);
          free(version);
        }
      }
      break;
    case MPD_CONNECTED:
      if (!c->is_websocket || c == NULL) {
        continue;
      }
      // send_queue(c);
      send_status(c);
      send_output(c);
      send_stats(c);
      send_channels(c);
      break;
    case MPD_FAILURE:
      LOG(LL_ERROR, ("MPD connection failed"));

    case MPD_RECONNECT:
    case MPD_DISCONNECT:
      if (mpd.conn != NULL) {
        LOG(LL_DEBUG, ("MPD disconnecting..."));
        mpd_connection_free(mpd.conn);
        mpd.conn = NULL;
      }
      mpd.conn_state = MPD_DISCONNECTED;
      break;
    }
  }
}

void mpd_callback(struct mg_connection *c, struct mg_ws_message *wm) {
  enum mpd_cmds cmd_id = get_cmd_id(wm->data.ptr, wm->data.len);
  Buffer buffer = {.data = NULL, .size = 0};
  Jim jim = {
      .sink = &buffer,
      .write = (Jim_Write)buffer_write,
  };

  if (mpd.conn_state != MPD_CONNECTED) {
    LOG(LL_ERROR, ("MPD is not connected"));
    jim_object_begin(&jim);
    jim_set_string(&jim, "type", "disconnected");
    jim_object_end(&jim);
    mg_ws_send(c, buffer.data, buffer.size, WEBSOCKET_OP_TEXT);
    buffer_free(&buffer);
    return;
  }
  switch (cmd_id) {
  case MPD_UPDATE_DB:
    if (mpd.conn_state == MPD_CONNECTED) {
      if (mpd_run_update(mpd.conn, NULL)) {
        LOG(LL_DEBUG, ("MPD update database"));
        jim_object_begin(&jim);
        jim_set_string(&jim, "type", "update");
        jim_set_bool(&jim, "success", true);
        jim_object_end(&jim);
        mg_ws_send(c, buffer.data, buffer.size, WEBSOCKET_OP_TEXT);
        buffer_free(&buffer);
        mpd.conn_state = MPD_RECONNECT;
      } else {
        LOG(LL_ERROR, ("MPD update database failed: %s",
                       mpd_connection_get_error_message(mpd.conn)));
        jim_object_begin(&jim);
        jim_set_string(&jim, "type", "update");
        jim_set_bool(&jim, "success", false);
        jim_set_string(&jim, "message",
                       mpd_connection_get_error_message(mpd.conn));
        jim_object_end(&jim);
        mg_ws_send(c, buffer.data, buffer.size, WEBSOCKET_OP_TEXT);
        buffer_free(&buffer);
        mpd.conn_state = MPD_FAILURE;
      }
    }
    break;
  case MPD_GET_CHANNELS:
    send_channels(c);
    break;
  case MPD_SEND_MESSAGE:
    mpd_run_send_message(mpd.conn, _get_key(wm->data, "channel"),
                         _get_key(wm->data, "text"));
    break;
  case MPD_SET_PLAY:
    mpd_run_play(mpd.conn);
    break;
  case MPD_SET_PAUSE:
    mpd_run_toggle_pause(mpd.conn);
    break;
  case MPD_SET_STOP:
    mpd_run_stop(mpd.conn);
    break;
  case MPD_SET_NEXT:
    mpd_run_next(mpd.conn);
    break;
  case MPD_SET_PREV:
    mpd_run_previous(mpd.conn);
    break;
  case MPD_SET_VOLUME:
    mpd_run_set_volume(mpd.conn, (int)_get_number(wm->data, "volume"));
    break;
  case MPD_SET_SEEK:
    mpd_run_seek_id(mpd.conn, (int)_get_number(wm->data, "id"),
                    (int)_get_number(wm->data, "pos"));
    break;
  case MPD_RM_TRACK:
    mpd_run_delete_id(mpd.conn, (int)_get_number(wm->data, "id"));
    break;
  case MPD_RM_ALL:
    mpd_run_clear(mpd.conn);
    break;
  case MPD_ADD_TRACK: {
    size_t vlen = 0;
    char *raw = parse("path", 4, wm->data.ptr, wm->data.len, &vlen);
    char *path = strndup(raw, vlen);
    mpd_run_add(mpd.conn, path);
    free(path);
  } break;
  case MPD_ADD_PLAY_TRACK: {
    size_t vlen = 0;
    char *raw = parse("path", 4, wm->data.ptr, wm->data.len, &vlen);
    char *path = strndup(raw, vlen);
    int i = mpd_run_add_id(mpd.conn, path);
    if (i <= 0) {
      jim_object_begin(&jim);
      jim_set_string(&jim, "type", "error");
      jim_set_string(&jim, "message", "Could not add song to queue");
      jim_object_end(&jim);

      mg_ws_send(c, buffer.data, buffer.size, WEBSOCKET_OP_TEXT);
      buffer_free(&buffer);
    } else {
      mpd_run_play_id(mpd.conn, i);
    }
    free(path);
  } break;
  case MPD_ADD_PLAYLIST: {
    size_t vlen = 0;
    char *raw = parse("name", 4, wm->data.ptr, wm->data.len, &vlen);
    char *name = strndup(raw, vlen);
    mpd_run_load(mpd.conn, name);
    free(name);
  } break;
  case MPD_TOGGLE_OUTPUT:
    mpd_run_toggle_output(mpd.conn, (int)_get_number(wm->data, "id"));
    break;
  case MPD_TOGGLE_RANDOM:
    mpd_run_random(mpd.conn, (int)_get_number(wm->data, "mode"));
    break;
  case MPD_TOGGLE_REPEAT:
    mpd_run_repeat(mpd.conn, (int)_get_number(wm->data, "mode"));
    break;
  case MPD_TOGGLE_SINGLE:
    mpd_run_single(mpd.conn, (int)_get_number(wm->data, "mode"));
    break;
  case MPD_TOGGLE_CONSUME:
    mpd_run_consume(mpd.conn, (int)_get_number(wm->data, "mode"));
    break;
  case MPD_TOGGLE_CROSSFADE:
    mpd_run_crossfade(mpd.conn, (int)_get_number(wm->data, "seconds"));
    break;
  case MPD_GET_QUEUE:
    send_queue(c, _get_number(wm->data, "offset"),
               _get_number(wm->data, "limit"));
    break;
  case MPD_GET_BROWSE:
    send_browse(c, _get_key(wm->data, "path"), _get_number(wm->data, "offset"),
                _get_number(wm->data, "limit"));
    break;
  case MPD_SAVE_QUEUE: {
    size_t vlen = 0;
    char *raw = parse("name", 4, wm->data.ptr, wm->data.len, &vlen);
    char *name = strndup(raw, vlen);
    mpd_run_save(mpd.conn, name);
    free(name);
  } break;
  case MPD_SEARCH:
    mpd_search(c, _get_key(wm->data, "query"));
    break;
  case MPD_GET_OUTPUTS:
    send_output(c);
    break;
  case MPD_PLAY_TRACK:
    mpd_run_play_id(mpd.conn, (int)_get_number(wm->data, "id"));
    break;
  case MPD_SEEK_CURRENT:
    mpd_run_seek_current(mpd.conn, (int)_get_number(wm->data, "pos"),
                         (int)_get_number(wm->data, "relative"));
    break;
  case MPD_GET_COMMANDS:
    if (mpd_send_allowed_commands(mpd.conn)) {
      struct mpd_pair *command;
      jim_object_begin(&jim);
      jim_set_string(&jim, "type", "commands");
      jim_member_key(&jim, "commands");
      jim_array_begin(&jim);
      while ((command = mpd_recv_command_pair(mpd.conn)) != NULL) {
        jim_string(&jim, command->value);
        mpd_return_pair(mpd.conn, command);
      }
      jim_array_end(&jim);
      jim_object_end(&jim);

      mg_ws_send(c, buffer.data, buffer.size, WEBSOCKET_OP_TEXT);
      buffer_free(&buffer);
    }
    break;
  default:
    LOG(LL_ERROR, ("Unknown command: %d", cmd_id));
    jim_object_begin(&jim);
    jim_set_string(&jim, "type", "error");
    jim_set_string(&jim, "message", "Unknown command");
    jim_object_end(&jim);

    mg_ws_send(c, buffer.data, buffer.size, WEBSOCKET_OP_TEXT);
    buffer_free(&buffer);
    break;
  }
}

void mpd_search(struct mg_connection *c, const char *query) {
  struct mpd_song *curr_song;
  Buffer buffer = {.data = NULL, .size = 0};
  Jim jim = {
      .sink = &buffer,
      .write = (Jim_Write)buffer_write,
  };

  jim_object_begin(&jim);
  if (mpd.conn_state == MPD_CONNECTED) {
    if (mpd_search_db_songs(mpd.conn, false) == false) {
      LOG(LL_ERROR, ("MPD search failed: %s",
                     mpd_connection_get_error_message(mpd.conn)));
      mpd.conn_state = MPD_FAILURE;
    } else {
      if (mpd_search_add_tag_constraint(mpd.conn, MPD_OPERATOR_DEFAULT,
                                        MPD_TAG_ARTIST, query) == false) {
        LOG(LL_ERROR, ("MPD search failed: %s",
                       mpd_connection_get_error_message(mpd.conn)));
        mpd.conn_state = MPD_FAILURE;
      } else {
        if (mpd_search_add_tag_constraint(mpd.conn, MPD_OPERATOR_DEFAULT,
                                          MPD_TAG_ALBUM, query) == false) {
          LOG(LL_ERROR, ("MPD search failed: %s",
                         mpd_connection_get_error_message(mpd.conn)));
          mpd.conn_state = MPD_FAILURE;
        } else {
          if (mpd_search_add_tag_constraint(mpd.conn, MPD_OPERATOR_DEFAULT,
                                            MPD_TAG_TITLE, query) == false) {
            LOG(LL_ERROR, ("MPD search failed: %s",
                           mpd_connection_get_error_message(mpd.conn)));
            mpd.conn_state = MPD_FAILURE;
          } else {
            if (mpd_search_commit(mpd.conn) == false) {
              LOG(LL_ERROR, ("MPD search failed: %s",
                             mpd_connection_get_error_message(mpd.conn)));
              mpd.conn_state = MPD_FAILURE;
            } else {
              jim_member_key(&jim, "songs");
              jim_array_begin(&jim);
              while ((curr_song = mpd_recv_song(mpd.conn)) != NULL) {
                jim_set_string(&jim, "title", get_title(curr_song));
                jim_set_string(&jim, "artist",
                               mpd_song_get_tag(curr_song, MPD_TAG_ARTIST, 0));
                jim_set_string(&jim, "album",
                               mpd_song_get_tag(curr_song, MPD_TAG_ALBUM, 0));
                jim_set_string(&jim, "genre",
                               mpd_song_get_tag(curr_song, MPD_TAG_GENRE, 0));
                jim_set_integer(&jim, "duration",
                                mpd_song_get_duration(curr_song));
                jim_set_integer(&jim, "id", mpd_song_get_id(curr_song));
                jim_set_string(&jim, "uri", mpd_song_get_uri(curr_song));
                jim_set_string(&jim, "year",
                               mpd_song_get_tag(curr_song, MPD_TAG_DATE, 0));

                mpd_song_free(curr_song);
                buffer_free(&buffer);
              }
              jim_array_end(&jim);
            }
          }
        }
      }
    }
  }
  jim_object_end(&jim);
  jim_set_string(&jim, "type", "search");
  mg_ws_send(c, buffer.data, buffer.size, WEBSOCKET_OP_TEXT);

  buffer_free(&buffer);
}

void mpd_disconnect(void) {
  if (mpd.conn_state == MPD_CONNECTED) {
    LOG(LL_INFO, ("Disconnecting from MPD"));
    mpd_connection_free(mpd.conn);
    mpd.conn_state = MPD_DISCONNECTED;
  }
}
