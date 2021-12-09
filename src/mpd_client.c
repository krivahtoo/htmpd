#include "mongoose.h"
#include "parson.h"
#include "mpd_client.h"
#include <string.h>

struct t_mpd mpd;

enum mpd_cmds get_cmd_id(JSON_Object *obj) {
  return (int)json_object_get_number(obj, "cmd_id");
}

char *mpd_get_title(struct mpd_song const *song) {
  char *str;

  str = (char *)mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
  if (str == NULL) {
    str = (char *)mpd_song_get_uri(song);
  }

  return str;
}

char *mpd_get_album(struct mpd_song const *song) {
  char *str;

  str = (char *)mpd_song_get_tag(song, MPD_TAG_ALBUM, 0);
  if (str == NULL) {
    str = "-";
  }

  return str;
}

char *mpd_get_artist(struct mpd_song const *song) {
  char *str;

  str = (char *)mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);
  if (str == NULL) {
    str = "-";
  }

  return str;
}

char *mpd_get_year(struct mpd_song const *song) {
  char *str;

  str = (char *)mpd_song_get_tag(song, MPD_TAG_DATE, 0);
  if (str == NULL) {
    str = "-";
  }

  return str;
}

void mpd_send_queue(struct mg_connection *c) {
  struct mpd_entity *entity;
  unsigned long totalTime = 0;
  JSON_Value *data = json_value_init_object();
  JSON_Value *songs_val = json_value_init_array();
  JSON_Object *obj = json_value_get_object(data);
  JSON_Array *songs = json_value_get_array(songs_val);

  if(!mpd_send_list_queue_meta(mpd.conn)) {
    json_object_set_string(obj, "type", "error");
    json_object_set_string(obj, "error", "Could not send queue");
    goto send_queue;
  }
  while((entity = mpd_recv_entity(mpd.conn)) != NULL) {
    const struct mpd_song *curr_song = mpd_entity_get_song(entity);
    if(mpd_entity_get_type(entity) == MPD_ENTITY_TYPE_SONG) {
      curr_song = mpd_entity_get_song(entity);
      JSON_Value *song = json_value_init_object();
      JSON_Object *songObj = json_value_get_object(song);
      json_object_set_string(songObj, "title", mpd_get_title(curr_song));
      json_object_set_string(songObj, "artist", mpd_get_artist(curr_song));
      json_object_set_string(songObj, "album", mpd_get_album(curr_song));
      json_object_set_string(songObj, "uri", mpd_song_get_uri(curr_song));
      json_object_set_number(songObj, "duration", mpd_song_get_duration(curr_song));
      json_object_set_number(songObj, "id", mpd_song_get_id(curr_song));
      json_object_set_number(songObj, "pos", mpd_song_get_pos(curr_song));
      json_object_set_string(songObj, "year", mpd_song_get_tag (curr_song, MPD_TAG_DATE, 0));
      json_array_append_value(songs, song);
      totalTime += mpd_song_get_duration(curr_song);
    }
    mpd_entity_free(entity);
  }
  json_object_set_string(obj, "type", "queue");

send_queue:
  json_object_set_number(obj, "totalTime", totalTime);
  json_object_set_value(obj, "queue", songs_val);
  char *json_str = json_serialize_to_string(data);

  mg_ws_send(c, json_str, strlen(json_str), WEBSOCKET_OP_TEXT);
  json_value_free(data);
}

void mpd_send_state(struct mg_connection *c) {
  JSON_Value *data = json_value_init_object();
  JSON_Object *obj = json_value_get_object(data);
  JSON_Value *song_val = json_value_init_object();
  JSON_Object *song = json_value_get_object(song_val);

  if(!mpd_send_status(mpd.conn)) {
  json_object_set_string(obj, "type", "error");
    json_object_set_string(obj, "error", "Could not send status");
    goto send_state;
  }
  struct mpd_status *status = mpd_recv_status(mpd.conn);
  if(status == NULL) {
    json_object_set_string(obj, "type", "error");
    json_object_set_string(obj, "error", "Could not receive status");
    goto send_state;
  }
  json_object_set_number(obj, "volume", mpd_status_get_volume(status));
  json_object_set_number(obj, "repeat", mpd_status_get_repeat(status));
  json_object_set_number(obj, "random", mpd_status_get_random(status));
  json_object_set_number(obj, "single", mpd_status_get_single(status));
  json_object_set_number(obj, "consume", mpd_status_get_consume(status));
  json_object_set_number(obj, "playlistLength", mpd_status_get_queue_length(status));
  json_object_set_number(obj, "playlistSong", mpd_status_get_song_pos(status));
  json_object_set_number(obj, "state", mpd_status_get_state(status));
  json_object_set_number(obj, "elapsedTime", mpd_status_get_elapsed_ms(status));
  json_object_set_number(obj, "totalTime", mpd_status_get_total_time(status));
  json_object_set_number(obj, "crossfade", mpd_status_get_crossfade(status));
  json_object_set_number(obj, "bitrate", mpd_status_get_kbit_rate(status));
  json_object_set_number(obj, "current", mpd_status_get_song_id(status));
  json_object_set_number(obj, "channels", mpd_status_get_audio_format(status)->channels);
  
  if(mpd_status_get_state(status) == MPD_STATE_PLAY || mpd_status_get_state(status) == MPD_STATE_PAUSE) {
    const struct mpd_song *curr_song = mpd_run_current_song(mpd.conn);
    if(curr_song != NULL) {
      json_object_set_string(song, "title", mpd_get_title(curr_song));
      json_object_set_string(song, "artist", mpd_get_artist(curr_song));
      json_object_set_string(song, "album", mpd_get_album(curr_song));
      json_object_set_number(song, "duration", mpd_song_get_duration(curr_song));
      json_object_set_number(song, "id", mpd_song_get_id(curr_song));
      json_object_set_number(song, "pos", mpd_song_get_pos(curr_song));
    }
  }
  json_object_set_string(obj, "type", "status");

send_state:
  json_object_set_value(obj, "song", song_val);
  char *json_str = json_serialize_to_string(data);

  mg_ws_send(c, json_str, strlen(json_str), WEBSOCKET_OP_TEXT);
  json_value_free(data);
}

void mpd_send_output(struct mg_connection *c) {
  JSON_Value *data = json_value_init_object();
  JSON_Object *obj = json_value_get_object(data);
  JSON_Value *outputs_val = json_value_init_array();
  JSON_Array *outputs = json_value_get_array(outputs_val);

  if(!mpd_send_outputs(mpd.conn)) {
    json_object_set_string(obj, "type", "error");
    json_object_set_string(obj, "error", "Could not send output list");
    goto send_outputs;
  }
  struct mpd_output *output;
  while((output = mpd_recv_output(mpd.conn)) != NULL) {
    JSON_Value *output_val = json_value_init_object();
    JSON_Object *output_obj = json_value_get_object(output_val);
    json_object_set_number(output_obj, "id", mpd_output_get_id(output));
    json_object_set_string(output_obj, "name", mpd_output_get_name(output));
    json_object_set_number(output_obj, "enabled", mpd_output_get_enabled(output));
    json_array_append_value(outputs, output_val);
    mpd_output_free(output);
  }
  json_object_set_string(obj, "type", "outputs");

send_outputs:
  json_object_set_value(obj, "outputs", outputs_val);
  char *json_str = json_serialize_to_string(data);

  mg_ws_send(c, json_str, strlen(json_str), WEBSOCKET_OP_TEXT);
  json_value_free(data);
}

void mpd_poll(struct mg_mgr *mgr) {
  struct mg_connection *c, *tmp;
  for(c = mgr->conns; c != NULL; c = tmp) {
    tmp = c->next;
    if (mpd.conn == NULL) {
      mpd.conn_state = MPD_DISCONNECTED;
    }
    switch (mpd.conn_state) {
      case MPD_DISCONNECTED:
        LOG(LL_DEBUG, ("MPD disconnected, reconnecting to %s:%d...", mpd.host, mpd.port));
        mpd.conn = mpd_connection_new(mpd.host, mpd.port, 5000);
        if (mpd.conn == NULL) {
          LOG(LL_ERROR, ("MPD connection failed"));
          mpd.conn_state = MPD_DISCONNECTED;
        } else {
          if (mpd_connection_get_error(mpd.conn) != MPD_ERROR_SUCCESS) {
            LOG(LL_ERROR, ("MPD connection failed: %s", mpd_connection_get_error_message(mpd.conn)));
            mpd.conn_state = MPD_DISCONNECTED;
          } else {
            if (mpd.password != NULL) {
              if (!mpd_run_password(mpd.conn, mpd.password)) {
                LOG(LL_ERROR, ("MPD password failed: %s", mpd_connection_get_error_message(mpd.conn)));
                mpd.conn_state = MPD_DISCONNECTED;
                return;
              }
            }
            LOG(LL_DEBUG, ("MPD connected"));
            mpd.conn_state = MPD_CONNECTED;
            if (!c->is_websocket || c == NULL) {
              continue;
            }
            char * msg = "{\"type\":\"connected\"}";
            mg_ws_send(c, msg, strlen(msg), WEBSOCKET_OP_TEXT);
          }
        }
        break;
      case MPD_CONNECTED:
        if (!c->is_websocket || c == NULL) {
          continue;
        }
        mpd_send_queue(c);
        mpd_send_state(c);
        mpd_send_output(c);
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
  JSON_Value *data = json_parse_string(wm->data.ptr);
  JSON_Object *obj = json_value_get_object(data);
  enum mpd_cmds cmd_id = get_cmd_id(obj);
  switch (cmd_id) {
    case MPD_UPDATE_DB:
      if (mpd.conn_state == MPD_CONNECTED) {
        if (mpd_run_update(mpd.conn, NULL)) {
          LOG(LL_DEBUG, ("MPD update database"));
          mpd.conn_state = MPD_RECONNECT;
        } else {
          LOG(LL_ERROR, ("MPD update database failed: %s", mpd_connection_get_error_message(mpd.conn)));
          mpd.conn_state = MPD_FAILURE;
        }
      }
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
      mpd_run_set_volume(mpd.conn, json_object_get_number(obj, "volume"));
      break;
    case MPD_SET_SEEK:
      mpd_run_seek_id(mpd.conn, json_object_get_number(obj, "id"), json_object_get_number(obj, "pos"));
      break;
    case MPD_RM_TRACK:
      mpd_run_delete_id(mpd.conn, json_object_get_number(obj, "id"));
      break;
    case MPD_ADD_TRACK:
      mpd_run_add(mpd.conn, json_object_get_string(obj, "path"));
      break;
    case MPD_ADD_PLAYLIST:
      mpd_run_load(mpd.conn, json_object_get_string(obj, "name"));
      break;
    case MPD_TOGGLE_RANDOM:
      mpd_run_random(mpd.conn, json_object_get_number(obj, "mode"));
      break;
    case MPD_TOGGLE_REPEAT:
      mpd_run_repeat(mpd.conn, json_object_get_number(obj, "mode"));
      break;
    case MPD_TOGGLE_SINGLE:
      mpd_run_single(mpd.conn, json_object_get_number(obj, "mode"));
      break;
    case MPD_TOGGLE_CONSUME:
      mpd_run_consume(mpd.conn, json_object_get_number(obj, "mode"));
      break;
    case MPD_GET_QUEUE:
      mpd_send_queue(c);
      break;
    case MPD_SAVE_QUEUE:
      mpd_run_save(mpd.conn, json_object_get_string(obj, "name"));
      break;
    case MPD_SEARCH:
      mpd_search(c, json_object_get_string(obj, "query"));
      break;
  }
  json_value_free(data);
}

void mpd_search(struct mg_connection *c, const char *query) {
  struct mpd_song *curr_song;
  JSON_Value *data = json_value_init_array();
  JSON_Value *songs_val = json_value_init_array();
  JSON_Array *songs = json_value_get_array(songs_val);
  JSON_Object *obj = json_value_get_object(data);
  if (mpd.conn_state == MPD_CONNECTED) {
    if (mpd_search_db_songs(mpd.conn, false) == false) {
      LOG(LL_ERROR, ("MPD search failed: %s", mpd_connection_get_error_message(mpd.conn)));
      mpd.conn_state = MPD_FAILURE;
    } else {
      if (mpd_search_add_tag_constraint(mpd.conn, MPD_OPERATOR_DEFAULT, MPD_TAG_ARTIST, query) == false) {
        LOG(LL_ERROR, ("MPD search failed: %s", mpd_connection_get_error_message(mpd.conn)));
        mpd.conn_state = MPD_FAILURE;
      } else {
        if (mpd_search_add_tag_constraint(mpd.conn, MPD_OPERATOR_DEFAULT, MPD_TAG_ALBUM, query) == false) {
          LOG(LL_ERROR, ("MPD search failed: %s", mpd_connection_get_error_message(mpd.conn)));
          mpd.conn_state = MPD_FAILURE;
        } else {
          if (mpd_search_add_tag_constraint(mpd.conn, MPD_OPERATOR_DEFAULT, MPD_TAG_TITLE, query) == false) {
            LOG(LL_ERROR, ("MPD search failed: %s", mpd_connection_get_error_message(mpd.conn)));
            mpd.conn_state = MPD_FAILURE;
          } else {
            if (mpd_search_commit(mpd.conn) == false) {
              LOG(LL_ERROR, ("MPD search failed: %s", mpd_connection_get_error_message(mpd.conn)));
              mpd.conn_state = MPD_FAILURE;
            } else {
              while ((curr_song = mpd_recv_song(mpd.conn)) != NULL) {
                JSON_Value *song_val = json_value_init_object();
                JSON_Object *song = json_value_get_object(song_val);
                json_object_set_string(song, "artist", mpd_song_get_tag(curr_song, MPD_TAG_ARTIST, 0));
                json_object_set_string(song, "album", mpd_song_get_tag(curr_song, MPD_TAG_ALBUM, 0));
                json_object_set_string(song, "title", mpd_song_get_tag(curr_song, MPD_TAG_TITLE, 0));
                json_object_set_number(song, "id", mpd_song_get_id(curr_song));
                json_array_append_value(songs, song_val);
                mpd_song_free(curr_song);
              }
            }
          }
        }
      }
    }
  }
  json_object_set_string(obj, "type", "search");
  json_object_set_value(obj, "songs", songs_val);
  char *json = json_serialize_to_string_pretty(data);
  mg_ws_send(c, json, strlen(json), WEBSOCKET_OP_TEXT);
}

void mpd_disconnect() {
  if (mpd.conn_state == MPD_CONNECTED) {
    LOG(LL_INFO, ("Disconnecting from MPD"));
    mpd_connection_free(mpd.conn);
    mpd.conn_state = MPD_DISCONNECTED;
  }
}

