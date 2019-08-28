#include <string.h>
#include <stdio.h>
#include "config.h"
#include "log.h"

// Size of the buffer used when reading a line from the config file
#define CONFIG_BUFFERSIZE  1024

#define DEFAULT_CALLBACK_URL "localhost"
#define DEFAULT_HTTP_LISTEN_IP ""
#define DEFAULT_HTTP_LISTEN_PORT ""
#define DEFAULT_RTSP_LISTEN_IP ""
#define DEFAULT_RTSP_LISTEN_PORT ""
#define IS_WHITESPACE(ch) ((ch) == ' ' || (ch) =='\t' || (ch) =='\n' || (ch) =='\r')

static void
config_set_default_values(RtspConfiguration *config)
{
  if (config != NULL) {
    //config->loglevel = LOGLEVEL_DEBUG;
    config->callbackUrl = g_malloc(strlen(DEFAULT_CALLBACK_URL) + 1);
    (void) strcpy(config->callbackUrl, DEFAULT_CALLBACK_URL);
    config->httpListenIp = g_malloc(strlen(DEFAULT_HTTP_LISTEN_IP) + 1);
    (void) strcpy(config->httpListenIp, DEFAULT_HTTP_LISTEN_IP);
    config->httpListenPort = g_malloc(strlen(DEFAULT_HTTP_LISTEN_PORT) + 1);
    (void) strcpy(config->httpListenPort, DEFAULT_HTTP_LISTEN_PORT);
    config->rtspListenIp = g_malloc(strlen(DEFAULT_RTSP_LISTEN_IP) + 1);
    (void) strcpy(config->rtspListenIp, DEFAULT_RTSP_LISTEN_IP);
    config->rtspListenPort = g_malloc(strlen(DEFAULT_RTSP_LISTEN_PORT) + 1);
    (void) strcpy(config->rtspListenPort, DEFAULT_RTSP_LISTEN_PORT);

  }
}

static gchar *
strip_leading(gchar *str)
{
  if (str != NULL) {
    while (IS_WHITESPACE(str[0])) {
        str++;
    }
  }
  return str;
}

static void
strip_trailing(gchar *str)
{
  if (str != NULL) {
    int pos = strlen(str) - 1;
    while (pos > 0 && IS_WHITESPACE(str[pos])) {
      str[pos] = '\0';
      pos--;
    }
  }
}

static gchar *
config_find_first_character(gchar *str, gchar* tokens)
{
  gchar* res = NULL; // found instance
  gchar* tmp = NULL;

  if (tokens != NULL && str != NULL) {
    // Step through all tokens
    for (; tokens != NULL && *tokens != '\0'; tokens++) {
      // Find first instance of this character
      tmp = strchr(str, *tokens);
      if (tmp != NULL) {
        if (res == NULL) {
          res = tmp;
        } else {
          res = (tmp < res) ? tmp : res; // if this character is earlier, use this instead
        }
      }
    }
  }
  return res;
}

static Loglevel
loglevel_from_string(gchar *loglevel)
{
  Loglevel ret;
  if (0 == strcmp(loglevel, "Debug")) {
   ret = LOGLEVEL_DEBUG;
  }
  if (0 == strcmp(loglevel, "Info")) {
   ret = LOGLEVEL_INFO;
  }
  if (0 == strcmp(loglevel, "Warn")) {
   ret = LOGLEVEL_WARN;
  }
   if (0 == strcmp(loglevel, "Error")) {
   ret = LOGLEVEL_ERROR;
  }
 return ret;
}


static void
config_set_value(RtspConfiguration *config, gchar* key,
        gchar* value)
{
  if (config != NULL && key != NULL && value != NULL && value[0] != '\0') {
    if (0 == strcmp(key, CALLBACK_URL)) {
      g_free(config->callbackUrl);
      config->callbackUrl = g_malloc(strlen(value) + 1);
      (void) strcpy(config->callbackUrl, value);
    } else if (0 == strcmp(key, LOGLEVEL)) {
      config->loglevel = loglevel_from_string(value);
    } else if (0 == strcmp(key, HTTP_LISTEN_IP)) {
      g_free(config->httpListenIp);
      config->httpListenIp = g_malloc(strlen(value) + 1);
      (void) strcpy(config->httpListenIp, value);
    } else if (0 == strcmp(key, HTTP_LISTEN_PORT)) {
      g_free(config->httpListenPort);
      config->httpListenPort = g_malloc(strlen(value) + 1);
     (void) strcpy(config->httpListenPort, value);
   } else if (0 == strcmp(key, RTSP_LISTEN_IP)) {
     g_free(config->rtspListenIp);
      config->rtspListenIp = g_malloc(strlen(value) + 1);
     (void) strcpy(config->rtspListenIp, value);
   } else if (0 == strcmp(key, RTSP_LISTEN_PORT)) {
     g_free(config->rtspListenPort);
      config->rtspListenPort = g_malloc(strlen(value) + 1);
     (void) strcpy(config->rtspListenPort, value);
 } else {
      PWARN("Unknown configuration key %s", key);
    }
  }
}

static gint
config_file_close(FILE *file)
{
  gint ret = 0;
  if (file != NULL && fclose(file) != 0) {
    char buff[256];
    ret = -1;
    // Find default error message
    if( strerror_r(errno, buff, 256 ) == 0 ) {
      PERROR("Failed to close file: %s\n", buff);
    }
  }
  return ret;
}

static ssize_t
config_readline_clean(FILE *file, gchar *buffer, size_t buffer_size)
{
  gchar *content = buffer;
  if (file == NULL || buffer == NULL || buffer_size <= 0) {
    content = NULL;
  } else {
    do {
      content = fgets(buffer, buffer_size, file);
      if (content != NULL) {
        gchar *comment;
        content = strip_leading(content);
         // First comment marks end of content (and therefore end of line)
        comment = config_find_first_character(content, ";#");
         if (comment != NULL) {
          comment[0] = '\0';
        }
        strip_trailing(content);
      }
      /* Check if the line turned out to be empty (or consisting only of
       * whitespace and/or comments).
       * If that's the case, ditch it and try the next line instead.
       */
    } while (content != NULL && content[0] == '\0'); // Skip empty lines
  }
  if (content == NULL) {
    return -1;
  } else {
    return (content - buffer);
  }
}

gint
getConfig(RtspConfiguration *config, gchar *filename)
{
  if (config == NULL)
  {
    return -1;
  }
  gint ret = -1;
  FILE *file;
  file = fopen(filename, "r");
  if (file == NULL) {
    return -1;
  }
  gchar buffer[CONFIG_BUFFERSIZE];
  gchar *line;  // This will point to the inside of the buffer.
  gchar *value; // So will this.
  ssize_t offset = -1;

  // Clear the current config
  config_set_default_values(config);

  do {
    // Read a line of text and remove comments and surrounding whitespace.
    offset = config_readline_clean(file, buffer, CONFIG_BUFFERSIZE);
    if (offset >= 0) {
      line = buffer + offset;
      /* line will now point to the the start of the content inside the buffer.
       * "        <<Key>>       =        <<Value>>    #comment"
       * +-------------buffer---------------------|-----------|
       *         +----line------------------------|
       */

      // Find the assignment operator
      value = config_find_first_character(line, ":=");
      if (value != NULL) {
        /* We now know the position of the assignment character.
         *     "<<Key>>       =        <<Value>>"
         *     +----line------------------------|
         *                    +------value------|
         */

         // mark the end of the key part
         *value = '\0';
         value++;
         /*     "<<Key>>       =        <<Value>>"
          *     +----line------|
          *                     +-----value------|
          */

         // Get rid of whitespace around the assignment char
         strip_trailing(line);
         value = strip_leading(value);
         /*     "<<Key>>       =        <<Value>>"
          *     +line-|              +-value-|
          */

         /* Key and value are now isolated in line and value. */
         config_set_value(config, line, value);
      } else {
        /* The line has no assignment - treat as a comment and ignore */
      }
    }
  } while (offset >= 0);
  ret = 0;
  if (file != NULL) {
    ret = config_file_close(file);
  }

  return ret;
}
