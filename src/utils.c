/* SPDX-License-Identifier: GPL-3.0-only */

#include "utils.h"

static char *get_file_extension(const char *filename) {
  char *dot = strrchr(filename, '.');
  if (!dot || dot == filename)
    return "";
  return dot + 1;
}

static char *guess_audio_type(const char *filename) {
  char *ext = get_file_extension(filename);
  if (!strcmp(ext, "mp3"))
    return "audio/mpeg";
  if (!strcmp(ext, "ogg"))
    return "audio/ogg";
  if (!strcmp(ext, "wav"))
    return "audio/wav";
  return "";
}

// Decode a uri string
void urldecode(char *dst, const char *src) {
  char a, b;
  while (*src) {
    if ((*src == '%') && ((a = src[1]) && (b = src[2])) &&
        (isxdigit(a) && isxdigit(b))) {
      if (a >= 'a')
        a -= 'a' - 'A';
      if (a >= 'A')
        a -= ('A' - 10);
      else
        a -= '0';
      if (b >= 'a')
        b -= 'a' - 'A';
      if (b >= 'A')
        b -= ('A' - 10);
      else
        b -= '0';
      *dst++ = 16 * a + b;
      src += 3;
    } else if (*src == '+') {
      *dst++ = ' ';
      src++;
    } else {
      *dst++ = *src++;
    }
  }
  *dst++ = '\0';
}

// Get audio album art from file
GdkPixbuf *retrieve_artwork(const char *music_dir, const char *uri) {
  GdkPixbuf *pixbuf = NULL;
  char *uri_path = NULL, *imagefile = NULL;
  DIR *dir;
  struct dirent *entry;
  regex_t regex;

  unsigned int i;
  const char *file_mime;
  AVFormatContext *fmt_ctx = NULL;
  GdkPixbufLoader *loader;

  if ((uri_path = malloc(strlen(music_dir) + strlen(uri) + 2)) == NULL) {
    LOG(LL_ERROR, ("malloc failed"));
    goto fail;
  }
  sprintf(uri_path, "%s/%s", music_dir, uri);
  file_mime = guess_audio_type(uri_path);
  if (strcmp(file_mime, "audio/mpeg") != 0) {
    LOG(LL_ERROR, ("%s is not an audio/mpeg file.\n", uri_path));
    goto image;
  }
  if ((fmt_ctx = avformat_alloc_context()) == NULL) {
    LOG(LL_ERROR, ("avformat_alloc_context() failed"));
    goto image;
  }
  if (avformat_open_input(&fmt_ctx, uri_path, NULL, NULL) != 0) {
    LOG(LL_ERROR, ("Could not open file %s", uri_path));
    goto image;
  }

  if (fmt_ctx->iformat->read_header(fmt_ctx) < 0) {
    LOG(LL_ERROR, ("Could not read header for file %s", uri_path));
    goto image;
  }

  // find the first attached picture
  for (i = 0; i < fmt_ctx->nb_streams; i++) {
    if (fmt_ctx->streams[i]->disposition & AV_DISPOSITION_ATTACHED_PIC) {
      AVPacket pkt;

      // Found a picture, read it
      LOG(LL_DEBUG, ("Found picture in stream %d", i));
      pkt = fmt_ctx->streams[i]->attached_pic;

      loader = gdk_pixbuf_loader_new();
      if (gdk_pixbuf_loader_write(loader, pkt.data, pkt.size, NULL) == FALSE) {
        LOG(LL_ERROR, ("Could not write to loader for file %s", uri_path));
        goto image;
      }

      if ((pixbuf = gdk_pixbuf_loader_get_pixbuf(loader)) == NULL) {
        LOG(LL_ERROR,
            ("Could not get pixbuf from loader for file %s", uri_path));
        goto image;
      }

      gdk_pixbuf_loader_close(loader, NULL);
      goto done;
    }
  }

image:
  // cut the file name from path for current directory
  *strrchr(uri_path, '/') = 0;
  if ((dir = opendir(uri_path)) == NULL) {
    LOG(LL_ERROR,
        ("Could not open directory %s : %s", uri_path, strerror(errno)));
    goto fail;
  }
  if (regcomp(&regex, REGEX_ARTWORK, REG_NOSUB + REG_ICASE) != 0) {
    LOG(LL_ERROR, ("Could not compile regex %s", REGEX_ARTWORK));
    goto fail;
  }

  while ((entry = readdir(dir))) {
    if (*entry->d_name == '.')
      continue;

    if (regexec(&regex, entry->d_name, 0, NULL, 0) == 0) {
      // found an image file
      if ((imagefile = malloc(strlen(uri_path) + strlen(entry->d_name) + 2)) ==
          NULL) {
        LOG(LL_ERROR, ("malloc failed"));
        goto fail;
      }

      sprintf(imagefile, "%s/%s", uri_path, entry->d_name);

      if ((pixbuf = gdk_pixbuf_new_from_file(imagefile, NULL)) == NULL) {
        LOG(LL_ERROR, ("Could not load image file %s", imagefile));
        goto fail;
      }

      free(imagefile);
      break;
    }
  }

  regfree(&regex);
  closedir(dir);

fail:
done:
  if (fmt_ctx != NULL) {
    avformat_close_input(&fmt_ctx);
    avformat_free_context(fmt_ctx);
  }
  free(uri_path);
  return pixbuf;
}
