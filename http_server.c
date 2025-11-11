#include "./http_server.h"

#define MAX_COUNT_PARAM 10

void parse_http_request_path(HttpRequest *request, char *path_and_params_str);
void parse_http_request_query_param(HttpRequest *request, char *path_and_params_str);
void parse_http_request_header(HttpRequest *request, char *request_str);

HttpRequest *new_http_request()
{
    HttpRequest *request = malloc(sizeof(HttpRequest));
    request->method = malloc(sizeof(char *));
    request->version = malloc(sizeof(char *));
    request->path = malloc(sizeof(char *));
    request->body = malloc(sizeof(char *));

    request->path_parameter = malloc(sizeof(HttpRequestPathParameter *));
    request->path_parameter->count = 0;
    request->path_parameter->entries = malloc(sizeof(HttpRequestPathParameterEntry *) * MAX_COUNT_PARAM);

    request->header = malloc(sizeof(HttpRequestHeader *));
    request->header->count = 0;
    request->header->entries = malloc(sizeof(HttpRequestHeaderEntry *) * MAX_COUNT_PARAM);

    return request;
}

void free_http_request(HttpRequest *request)
{
    free(request->method);
    free(request->version);
    free(request->path);
    free(request->body);

    for (size_t i = 0; i < request->path_parameter->count; i++)
    {
        free(request->path_parameter->entries[i]);
    }
    free(request->path_parameter->entries);

    for (size_t i = 0; i < request->header->count; i++)
    {
        free(request->header->entries[i]);
    }
    free(request->header->entries);
}

void parse_http_request(HttpRequest *request, char *request_str)
{
    // ヘッダーをパース
    request->method = malloc(sizeof(char) * 16);
    request->version = malloc(sizeof(char) * 16);
    char path_and_params[1024];
    sscanf(request_str, "%s %s %s\r\n", request->method, path_and_params, request->version);
    printf("method = %s, path and params = %s, version = %s\n",
           request->method,
           path_and_params,
           request->version);

    // パスとクエリパラメータに分割
    char *position = NULL;
    char *token_ptr = strtok_r(path_and_params, "?", &position);

    char *path_str = NULL;
    if (token_ptr != NULL)
    {
        path_str = token_ptr;
    }
    else
    {
        path_str = path_and_params;
    }
    parse_http_request_path(request, path_str);

    token_ptr = strtok_r(NULL, "?", &position);
    if (token_ptr != NULL)
    {
        parse_http_request_query_param(request, token_ptr);
    }
}

void parse_http_request_path(HttpRequest *request, char *path_str)
{
    // パス文字列の抽出
    strncpy(request->path, path_str, strlen(path_str));
    request->path[strlen(path_str) + 1] = '\0';
}

void parse_http_request_query_param(HttpRequest *request, char *params_str)
{
    char query_params[1024];
    strncpy(query_params, params_str, strlen(params_str));
    query_params[strlen(params_str) + 1] = '\0';

    // クエリパラメータの分割
    char *position_amp = NULL;
    char *position_equal = NULL;
    char *param_token_ptr = strtok_r(params_str, "&", &position_amp);
    do
    {
        if (param_token_ptr == NULL)
        {
            break;
        }
        // parameterをkeyとvalueに分割
        HttpRequestPathParameterEntry *param_ptr = malloc(sizeof(HttpRequestPathParameterEntry));
        param_ptr->key = strdup(strtok_r(param_token_ptr, "=", &position_equal));
        param_ptr->value = strdup(strtok_r(NULL, "=", &position_equal));

        request->path_parameter->entries[request->path_parameter->count] = param_ptr;
        request->path_parameter->count++;
    } while ((param_token_ptr = strtok_r(NULL, "&", &position_amp)) != NULL && request->path_parameter->count < MAX_COUNT_PARAM);
}

const char *get_http_status_message(int code)
{
    int index = 0;
    while (HTTP_STATUS[index].status_code != -1)
    {
        if (HTTP_STATUS[index].status_code == code)
        {
            return HTTP_STATUS[index].status_message;
        }
        index++;
    }
    return NULL;
}
