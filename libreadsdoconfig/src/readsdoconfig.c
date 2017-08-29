/*
 * readsdoconfig.c
 *
 * Read device configuration for the SDO transfers from CSV file.
 *
 * Frank Jeschke <fjeschke@synapticon.com>
 *
 * 2017 Synapticon GmbH
 */

#include <readsdoconfig.h>

#include <stdio.h>
#include <string.h>

#define MAX_INPUT_LINE    1024
#define MAX_TOKEN_SIZE    255

struct _token_t {
  char **token;
  size_t count;
  struct _token_t *next;
};

/* every token list has the size (token_count) = 2 + number_of_axis */
static size_t get_node_count(struct _token_t *token_list)
{
  struct _token_t *t = token_list;
  size_t token_count = t->count;
  t = t->next;

  while (t->next != NULL) {
    if (token_count != t->count) {
      return 0;  /* parse error -> token count == 0 */
    }

    t = t->next;
  }

  return (token_count - 2); /* substract index and subindex fields */
}

static size_t get_token_count(char *buf, size_t bufsize)
{
  size_t separator = 0;
  char *c = buf;

  for (size_t i = 0; i < bufsize && *c != '\0'; i++, c++) {
    if (*c == ',') {
      separator++;
    }
  }

  return (separator + 1);
}

static void free_token(struct _token_t *t)
{
  if (t->next == NULL) {
    free(t);
  } else {
    free_token(t->next);
  }
}

static void tokenize_inbuf(char *buf, size_t bufsize, struct _token_t *token)
{
  char *sep = ",";
  char *b = malloc(bufsize * sizeof(char));
  char *word = NULL;

  size_t tokenitem = 0;
  token->count = get_token_count(buf, bufsize);
  token->token = malloc(token->count * sizeof(char *));

  memmove(b, buf, bufsize * sizeof(char));

  for (word = strtok(b, sep);  word; word = strtok(NULL, sep)) {
    *(token->token + tokenitem) = malloc(strlen(word) + 1);
    strncpy(*(token->token + tokenitem), word, (strlen(word) + 1));
    tokenitem++;
  }

  free(b);
}

static long parse_token(char *token_str)
{
  long value = strtol(token_str, NULL, 0);

  return value;
}

static void parse_token_for_node(struct _token_t *tokens, SdoParam_t *param,
                                 size_t node)
{
  param->index    = (uint16_t) parse_token(*(tokens->token));
  param->subindex = (uint8_t)  parse_token(*(tokens->token + 1));
  param->value    = (uint32_t) parse_token(*(tokens->token + 2 + node));
}

int read_sdo_config(const char *path, SdoConfigParameter_t *parameter)
{
  if (parameter == NULL) {
    return -1;
  }

  FILE *f = fopen(path, "r");
  if (f == NULL) {
    return -1;
  }

  struct _token_t *token = malloc(sizeof(struct _token_t));
  token->token = NULL;
  token->count = 0;
  token->next  = NULL;

  struct _token_t *t = token;
  size_t param_count = 0;

  char inbuf[MAX_INPUT_LINE];
  size_t inbuf_length = 0;
  int c;

  /* read file and tokenize */
  while ((c = fgetc(f)) != EOF) {
    if (c == '#') {
      while (c != '\n') {
        c = fgetc(f);
      }
    }

    if (c == '\n') {
      if (inbuf_length > 1) {
        inbuf[inbuf_length++] = '\0';
        tokenize_inbuf(inbuf, inbuf_length, t);
        param_count++;
        t->next = calloc(1, sizeof(struct _token_t));
        if (t->next != NULL) {
          t = t->next;
        }
      }

      inbuf_length = 0;
      continue;
    }

    if (c == ' ' ||
        c == '\t') { /* filter whitespaces - FIXME attention if strings are supported! */
      continue;
    }

    inbuf[inbuf_length] = (char)c;
    inbuf_length++;
  }

  int retval = -1;
  if (feof(f)) {
    retval = 0;
  }

  fclose(f);

  parameter->param_count = param_count;
  parameter->node_count  = get_node_count(token);
  if (parameter->node_count == 0 || parameter->param_count == 0) {
    fprintf(stderr, "Parse error number of nodes are different!\n");
    free_token(token);
    return -1;
  }

  parameter->parameter = malloc(parameter->node_count * sizeof(SdoParam_t *));
  if (parameter->parameter == NULL) {
    fprintf(stderr, "Error allocating enough memory for parameter storage\n");
    return -1;
  }

  SdoParam_t **sdoparam = parameter->parameter;

  for (size_t node = 0; node < parameter->node_count; node++) {
    sdoparam[node] = malloc(parameter->param_count * sizeof(SdoParam_t));
    SdoParam_t *pn = sdoparam[node];

    t = token;
    for (size_t param = 0; param < parameter->param_count; param++) {
      if (t->next == NULL) {
        fprintf(stderr, "Warning, token ran out before parameter count was reached.\n");
        break;
      }

      SdoParam_t *p = &pn[param];
      parse_token_for_node(t, p, node);
      t = t->next;
    }
  }

  free_token(token);

  return retval;
}
