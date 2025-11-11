#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct
{
    char *key;
    char *value;
} HttpRequestPathParameterEntry;

typedef struct
{
    int count;
    HttpRequestPathParameterEntry **entries;
} HttpRequestPathParameter;

typedef struct
{
    char *key;
    char *value;
} HttpRequestHeaderEntry;

typedef struct
{
    int count;
    HttpRequestHeaderEntry **entries;
} HttpRequestHeader;

typedef struct
{
    char *method;
    char *version;
    HttpRequestPathParameter *path_parameter;
    HttpRequestHeader *header;
    char *path;
    char *body;
} HttpRequest;

typedef struct
{
    int status_code;
    const char *status_message;
} HttpResponseStatus;

static const HttpResponseStatus HTTP_STATUS[] = {
    {200, "OK"},
    {400, "Bad Request"},
    {500, "Internal Server Error"},
    {-1, NULL}, // 番兵
};

HttpRequest *new_http_request();
void free_http_request(HttpRequest *request);
void parse_http_request(HttpRequest *request, char *request_str);
const char *get_http_status_message(int code);

#endif