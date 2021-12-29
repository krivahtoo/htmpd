/* SPDX-License-Identifier: GPL-3.0-only */

#include "parson.h"
#include "mpd_client.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

struct t_mpd mpd;

enum mpd_cmds get_cmd_id(JSON_Object *obj) {
  return (int)json_object_get_number(obj, "cmd_id");
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

void send_queue(struct mg_connection *c, double offset, double limit) {
  struct mpd_entity *entity;
  size_t loc_limit;
  size_t i = 0;
  size_t j = 0;
  unsigned long totalTime = 0;
  JSON_Value *data = json_value_init_object();
  JSON_Value *songs_val = json_value_init_array();
  JSON_Object *obj = json_value_get_object(data);
  JSON_Array *songs = json_value_get_array(songs_val);

  if (limit == 0) {
    loc_limit = 20;
  } else {
    loc_limit = (size_t)limit;
  }
  if(!mpd_send_list_queue_meta(mpd.conn)) {
    json_object_set_string(obj, "type", "error");
    json_object_set_string(obj, "message", mpd_connection_get_error_message(mpd.conn));
    mpd.conn_state = MPD_FAILURE;
    goto send_queue;
  }
  while((entity = mpd_recv_entity(mpd.conn)) != NULL) {
    if (i >= offset && j <= loc_limit) {
      const struct mpd_song *curr_song = mpd_entity_get_song(entity);
      if(mpd_entity_get_type(entity) == MPD_ENTITY_TYPE_SONG) {
        curr_song = mpd_entity_get_song(entity);
        JSON_Value *song = json_value_init_object();
        JSON_Object *songObj = json_value_get_object(song);
        json_object_set_string(songObj, "title", get_title(curr_song));
        json_object_set_string(songObj, "artist", mpd_song_get_tag(curr_song, MPD_TAG_ARTIST, 0));
        json_object_set_string(songObj, "album", mpd_song_get_tag(curr_song, MPD_TAG_ALBUM, 0));
        json_object_set_string(songObj, "genre", mpd_song_get_tag(curr_song, MPD_TAG_GENRE, 0));
        json_object_set_string(songObj, "uri", mpd_song_get_uri(curr_song));
        json_object_set_number(songObj, "duration", mpd_song_get_duration(curr_song));
        json_object_set_number(songObj, "id", mpd_song_get_id(curr_song));
        json_object_set_number(songObj, "pos", mpd_song_get_pos(curr_song));
        json_object_set_string(songObj, "year", mpd_song_get_tag (curr_song, MPD_TAG_DATE, 0));
        json_array_append_value(songs, song);
        totalTime += mpd_song_get_duration(curr_song);
      }
      j++;
    }
    mpd_entity_free(entity);
    i++;
  }
  json_object_set_string(obj, "type", "queue");
  json_object_set_number(obj, "limit", loc_limit);
  json_object_set_number(obj, "offset", offset);
  json_object_set_number(obj, "total", i);
  json_object_set_number(obj, "totalTime", totalTime);
  json_object_set_value(obj, "queue", songs_val);

send_queue:
  {
    char *json_str = json_serialize_to_string(data);
    mg_ws_send(c, json_str, strlen(json_str), WEBSOCKET_OP_TEXT);
  }
  json_value_free(data);
}

void send_status(struct mg_connection *c) {
  JSON_Value *data = json_value_init_object();
  JSON_Object *obj = json_value_get_object(data);
  JSON_Value *song_val = json_value_init_object();
  JSON_Object *song = json_value_get_object(song_val);

  if(!mpd_send_status(mpd.conn)) {
  json_object_set_string(obj, "type", "error");
    json_object_set_string(obj, "message", mpd_connection_get_error_message(mpd.conn));
    mpd.conn_state = MPD_FAILURE;
    goto send_status;
  }
  struct mpd_status *status = mpd_recv_status(mpd.conn);
  if(status == NULL) {
    json_object_set_string(obj, "type", "error");
    json_object_set_string(obj, "message", mpd_connection_get_error_message(mpd.conn));
    mpd.conn_state = MPD_FAILURE;
    goto send_status;
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
  
  if(mpd_status_get_state(status) == MPD_STATE_PLAY || mpd_status_get_state(status) == MPD_STATE_PAUSE) {
    const struct mpd_song *curr_song = mpd_run_current_song(mpd.conn);
    if(curr_song != NULL) {
      json_object_set_string(song, "title", get_title(curr_song));
      json_object_set_string(song, "artist", mpd_song_get_tag(curr_song, MPD_TAG_ARTIST, 0));
      json_object_set_string(song, "album", mpd_song_get_tag(curr_song, MPD_TAG_ALBUM, 0));
      json_object_set_string(song, "genre", mpd_song_get_tag(curr_song, MPD_TAG_GENRE, 0));
      json_object_set_number(song, "duration", mpd_song_get_duration(curr_song));
      json_object_set_number(song, "id", mpd_song_get_id(curr_song));
      json_object_set_number(song, "pos", mpd_song_get_pos(curr_song));
      json_object_set_string(song, "uri", mpd_song_get_uri(curr_song));
      json_object_set_string(song, "year", mpd_song_get_tag(curr_song, MPD_TAG_DATE, 0));
    }
  }
  json_object_set_string(obj, "type", "status");
  json_object_set_value(obj, "song", song_val);

send_status:
  {
    char *json_str = json_serialize_to_string(data);
    mg_ws_send(c, json_str, strlen(json_str), WEBSOCKET_OP_TEXT);
  }
  json_value_free(data);
}

void send_output(struct mg_connection *c) {
  JSON_Value *data = json_value_init_object();
  JSON_Object *obj = json_value_get_object(data);
  JSON_Value *outputs_val = json_value_init_array();
  JSON_Array *outputs = json_value_get_array(outputs_val);

  if(!mpd_send_outputs(mpd.conn)) {
    json_object_set_string(obj, "type", "error");
    json_object_set_string(obj, "message", mpd_connection_get_error_message(mpd.conn));
    mpd.conn_state = MPD_FAILURE;
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
  json_object_set_value(obj, "outputs", outputs_val);

send_outputs:
  {
    char *json_str = json_serialize_to_string(data);
    mg_ws_send(c, json_str, strlen(json_str), WEBSOCKET_OP_TEXT);
  }
  json_value_free(data);
}

void send_browse(struct mg_connection *c, const char *uri, double offset, double limit) {
  size_t loc_limit = (size_t)limit;
  size_t i = 0;
  size_t j = 0;
  struct {
    int dirs;
    int files;
    int playlists;
  } count = {0, 0, 0};
  JSON_Value *data = json_value_init_object();
  JSON_Object *obj = json_value_get_object(data);
  JSON_Value *dirs_val = json_value_init_array();
  JSON_Array *dirs = json_value_get_array(dirs_val);
  JSON_Value *files_val = json_value_init_array();
  JSON_Array *files = json_value_get_array(files_val);
  JSON_Value *playlists_val = json_value_init_array();
  JSON_Array *playlists = json_value_get_array(playlists_val);

  if (loc_limit <= 0) {
    loc_limit = 20;
  }

  if(!mpd_send_list_meta(mpd.conn, uri)) {
    json_object_set_string(obj, "type", "error");
    json_object_set_string(obj, "message", "Could not send directory list");
    mpd.conn_state = MPD_FAILURE;
    goto send_browse;
  }
  struct mpd_entity *entity;
  while((entity = mpd_recv_entity(mpd.conn)) != NULL) {
    switch(mpd_entity_get_type(entity)) {
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
    if (i >= offset && j <= loc_limit) {
      switch(mpd_entity_get_type(entity)) {
        case MPD_ENTITY_TYPE_DIRECTORY: {
          JSON_Value *dir_val = json_value_init_object();
          JSON_Object *dir = json_value_get_object(dir_val);
          json_object_set_string(dir, "name", mpd_directory_get_path(mpd_entity_get_directory(entity)));
          json_array_append_value(dirs, dir_val);
        } break;
        case MPD_ENTITY_TYPE_SONG: {
          JSON_Value *file_val = json_value_init_object();
          JSON_Object *file = json_value_get_object(file_val);
          json_object_set_string(file, "title", get_title(mpd_entity_get_song(entity)));
          json_object_set_string(file, "artist", mpd_song_get_tag(mpd_entity_get_song(entity), MPD_TAG_ARTIST, 0));
          json_object_set_string(file, "album", mpd_song_get_tag(mpd_entity_get_song(entity), MPD_TAG_ALBUM, 0));
          json_object_set_string(file, "genre", mpd_song_get_tag(mpd_entity_get_song(entity), MPD_TAG_GENRE, 0));
          json_object_set_number(file, "duration", mpd_song_get_duration(mpd_entity_get_song(entity)));
          json_object_set_string(file, "uri", mpd_song_get_uri(mpd_entity_get_song(entity)));
          json_object_set_string(file, "year", mpd_song_get_tag (mpd_entity_get_song(entity), MPD_TAG_DATE, 0));
          json_array_append_value(files, file_val);
        } break;
        case MPD_ENTITY_TYPE_PLAYLIST: {
          JSON_Value *playlist_val = json_value_init_object();
          JSON_Object *playlist = json_value_get_object(playlist_val);
          json_object_set_string(playlist, "name", mpd_playlist_get_path(mpd_entity_get_playlist(entity)));
          json_array_append_value(playlists, playlist_val);
        } break;
        default:
          break;
      }
      j++;
    }
    mpd_entity_free(entity);
    i++;
  }
  json_object_set_string(obj, "type", "browse");
  json_object_set_number(obj, "limit", loc_limit);
  json_object_set_number(obj, "offset", offset);
  json_object_set_number(obj, "total", i);
  json_object_set_number(obj, "dirsCount", count.dirs);
  json_object_set_number(obj, "filesCount", count.files);
  json_object_set_number(obj, "playlistsCount", count.playlists);
  json_object_set_value(obj, "dirs", dirs_val);
  json_object_set_value(obj, "files", files_val);
  json_object_set_value(obj, "playlists", playlists_val);

send_browse:
  {
    char *json_str = json_serialize_to_string(data);
    mg_ws_send(c, json_str, strlen(json_str), WEBSOCKET_OP_TEXT);
  }
  json_value_free(data);
}

void send_stats(struct mg_connection *c) {
  JSON_Value *data = json_value_init_object();
  JSON_Object *obj = json_value_get_object(data);
  struct mpd_stats *stats = mpd_run_stats(mpd.conn);
  if (stats != NULL) {
    json_object_set_string(obj, "type", "stats");
    json_object_set_number(obj, "artistsCount", mpd_stats_get_number_of_artists(stats));
    json_object_set_number(obj, "songsCount", mpd_stats_get_number_of_songs(stats));
    json_object_set_number(obj, "albumsCount", mpd_stats_get_number_of_albums(stats));
    json_object_set_number(obj, "uptime", mpd_stats_get_uptime(stats));
    json_object_set_number(obj, "playTime", mpd_stats_get_play_time(stats));
    json_object_set_number(obj, "dbPlayTime", mpd_stats_get_db_play_time(stats));
    json_object_set_number(obj, "dbUpdateTime", mpd_stats_get_db_update_time(stats));
    mpd_stats_free(stats);
  } else {
    json_object_set_string(obj, "type", "error");
    json_object_set_string(obj, "message", "Could not send stats");
    mpd.conn_state = MPD_FAILURE;
  }
  char *json_str = json_serialize_to_string(data);

  mg_ws_send(c, json_str, strlen(json_str), WEBSOCKET_OP_TEXT);
  json_value_free(data);
}

void send_channels(struct mg_connection *c) {
  JSON_Value *data = json_value_init_object();
  JSON_Object *obj = json_value_get_object(data);
  if (mpd_send_channels(mpd.conn)) {
    JSON_Value *channels_val = json_value_init_array();
    JSON_Array *channels = json_value_get_array(channels_val);
    struct mpd_pair *channel;
    while ((channel = mpd_recv_channel_pair(mpd.conn)) != NULL) {
      json_array_append_string(channels, channel->value);
      mpd_return_pair(mpd.conn, channel);
    }
    json_object_set_string(obj, "type", "channels");
    json_object_set_value(obj, "channels", channels_val);
  } else {
    json_object_set_string(obj, "type", "error");
    json_object_set_string(obj, "message", "Could not send channels");
  }
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
    } else {
      // Report errors
      if (mpd_connection_get_error(mpd.conn) != MPD_ERROR_SUCCESS) {
        LOG(LL_ERROR, ("MPD connection failed: %s", mpd_connection_get_error_message(mpd.conn)));
        if (c->is_websocket) {
          JSON_Value *data = json_value_init_object();
          JSON_Object *obj = json_value_get_object(data);
          json_object_set_string(obj, "type", "error");
          json_object_set_string(obj, "message", mpd_connection_get_error_message(mpd.conn));
          char *json_str = json_serialize_to_string(data);
          mg_ws_send(c, json_str, strlen(json_str), WEBSOCKET_OP_TEXT);
          json_value_free(data);
        }
        // try recovering
        if (!mpd_connection_clear_error(mpd.conn)) {
          mpd.conn_state = MPD_FAILURE;
        }
      }
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
            mpd.version = mpd_connection_get_server_version(mpd.conn);
            mpd.conn_state = MPD_CONNECTED;
            if (!c->is_websocket || c == NULL) {
              continue;
            }
            JSON_Value *data = json_value_init_object();
            JSON_Object *obj = json_value_get_object(data);
            json_object_set_string(obj, "type", "connected");
            // json_object_set_string(obj, "version", mpd.version);
            char *msg = json_serialize_to_string(data);
            mg_ws_send(c, msg, strlen(msg), WEBSOCKET_OP_TEXT);
            json_value_free(data);
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
  if (mpd.conn_state != MPD_CONNECTED) {
    LOG(LL_ERROR, ("MPD is not connected"));
    JSON_Value *dt = json_value_init_object();
    JSON_Object *dt_obj = json_value_get_object(dt);
    json_object_set_string(dt_obj, "type", "disconnected");
    char *json_str = json_serialize_to_string(dt);
    mg_ws_send(c, json_str, strlen(json_str), WEBSOCKET_OP_TEXT);
    json_value_free(dt);
    return;
  }
  switch (cmd_id) {
    case MPD_UPDATE_DB:
      if (mpd.conn_state == MPD_CONNECTED) {
        if (mpd_run_update(mpd.conn, NULL)) {
          LOG(LL_DEBUG, ("MPD update database"));
          JSON_Value *dt = json_value_init_object();
          JSON_Object *dt_obj = json_value_get_object(dt);
          json_object_set_string(dt_obj, "type", "update");
          json_object_set_boolean(dt_obj, "success", true);
          char *msg = json_serialize_to_string(dt);
          mg_ws_send(c, msg, strlen(msg), WEBSOCKET_OP_TEXT);
          json_value_free(dt);
          mpd.conn_state = MPD_RECONNECT;
        } else {
          LOG(LL_ERROR, ("MPD update database failed: %s", mpd_connection_get_error_message(mpd.conn)));
          JSON_Value *dt = json_value_init_object();
          JSON_Object *dt_obj = json_value_get_object(dt);
          json_object_set_string(dt_obj, "type", "update");
          json_object_set_boolean(dt_obj, "success", false);
          json_object_set_string(dt_obj, "message", mpd_connection_get_error_message(mpd.conn));
          char *msg = json_serialize_to_string(dt);
          mg_ws_send(c, msg, strlen(msg), WEBSOCKET_OP_TEXT);
          json_value_free(dt);
          mpd.conn_state = MPD_FAILURE;
        }
      }
      break;
    case MPD_GET_CHANNELS:
      send_channels(c);
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
    case MPD_RM_ALL:
      mpd_run_clear(mpd.conn);
      break;
    case MPD_ADD_TRACK:
      mpd_run_add(mpd.conn, json_object_get_string(obj, "path"));
      break;
    case MPD_ADD_PLAY_TRACK: {
        int i = mpd_run_add_id(mpd.conn, json_object_get_string(obj, "path"));
        if (i <= 0) {
          JSON_Value *val = json_value_init_object();
          JSON_Object *val_obj = json_value_get_object(val);
          json_object_set_string(val_obj, "type", "error");
          json_object_set_string(val_obj, "message", "Could not add song to queue");
          char *str = json_serialize_to_string(val);
          mg_ws_send(c, str, strlen(str), WEBSOCKET_OP_TEXT);
          json_value_free(val);
        } else {
          mpd_run_play_id(mpd.conn, i);
        }
      } break;
    case MPD_ADD_PLAYLIST:
      mpd_run_load(mpd.conn, json_object_get_string(obj, "name"));
      break;
    case MPD_TOGGLE_OUTPUT:
      mpd_run_toggle_output(mpd.conn, json_object_get_number(obj, "id"));
      break;
    case MPD_TOGGLE_RANDOM:
      mpd_run_random(mpd.conn, json_object_get_boolean(obj, "mode"));
      break;
    case MPD_TOGGLE_REPEAT:
      mpd_run_repeat(mpd.conn, json_object_get_boolean(obj, "mode"));
      break;
    case MPD_TOGGLE_SINGLE:
      mpd_run_single(mpd.conn, json_object_get_boolean(obj, "mode"));
      break;
    case MPD_TOGGLE_CONSUME:
      mpd_run_consume(mpd.conn, json_object_get_boolean(obj, "mode"));
      break;
    case MPD_TOGGLE_CROSSFADE:
      mpd_run_crossfade(mpd.conn, json_object_get_number(obj, "seconds"));
      break;
    case MPD_GET_QUEUE:
      send_queue(c, json_object_get_number(obj, "offset"), json_object_get_number(obj, "limit"));
      break;
    case MPD_GET_BROWSE:
      send_browse(c, json_object_get_string(obj, "path"), json_object_get_number(obj, "offset"), json_object_get_number(obj, "limit"));
      break;
    case MPD_SAVE_QUEUE:
      mpd_run_save(mpd.conn, json_object_get_string(obj, "name"));
      break;
    case MPD_SEARCH:
      mpd_search(c, json_object_get_string(obj, "query"));
      break;
    case MPD_GET_OUTPUTS:
      send_output(c);
      break;
    case MPD_PLAY_TRACK:
      mpd_run_play_id(mpd.conn, json_object_get_number(obj, "id"));
      break;
    case MPD_SEEK_CURRENT:
      mpd_run_seek_current(mpd.conn, json_object_get_number(obj, "pos"), json_object_get_boolean(obj, "relative"));
      break;
    case MPD_GET_COMMANDS:
      if(mpd_send_allowed_commands(mpd.conn)) {
        struct mpd_pair *command;
        JSON_Value *dt = json_value_init_object();
        JSON_Object *dt_obj = json_value_get_object(dt);
        JSON_Value *cmds = json_value_init_array();
        JSON_Array *cmds_arr = json_value_get_array(cmds);
        while ((command = mpd_recv_command_pair(mpd.conn)) != NULL) {
          json_array_append_string(cmds_arr, command->value);
          mpd_return_pair(mpd.conn, command);
        }
        json_object_set_string(dt_obj, "type", "commands");
        json_object_set_value(dt_obj, "commands", cmds);
        char * msg = json_serialize_to_string(dt);
        mg_ws_send(c, msg, strlen(msg), WEBSOCKET_OP_TEXT);
        json_value_free(dt);
      }
      break;
    default:
      LOG(LL_ERROR, ("Unknown command: %d", cmd_id));
      JSON_Value *dt = json_value_init_object();
      JSON_Object *dt_obj = json_value_get_object(dt);
      json_object_set_string(dt_obj, "type", "error");
      json_object_set_string(dt_obj, "message", "Unknown command");
      char *msg = json_serialize_to_string(dt);
      mg_ws_send(c, msg, strlen(msg), WEBSOCKET_OP_TEXT);
      json_value_free(dt);
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
                json_object_set_string(song, "title", get_title(curr_song));
                json_object_set_string(song, "artist", mpd_song_get_tag(curr_song, MPD_TAG_ARTIST, 0));
                json_object_set_string(song, "album", mpd_song_get_tag(curr_song, MPD_TAG_ALBUM, 0));
                json_object_set_string(song, "genre", mpd_song_get_tag(curr_song, MPD_TAG_GENRE, 0));
                json_object_set_number(song, "duration", mpd_song_get_duration(curr_song));
                json_object_set_number(song, "id", mpd_song_get_id(curr_song));
                json_object_set_string(song, "uri", mpd_song_get_uri(curr_song));
                json_object_set_string(song, "year", mpd_song_get_tag(curr_song, MPD_TAG_DATE, 0));
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

