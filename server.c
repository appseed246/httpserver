#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>

#include "./http_server.h"

#define HTTP_PORT 8080
#define READ_BUFFER_SIZE 4096
#define WRITE_BUFFER_SIZE 4096
#define RESPONSE_HEADER_BUFFER_SIZE 1048
#define RESPONSE_BODY_BUFFER_SIZE 256

#define CALCULATE_SUCCESS 0
#define CALCULATE_FAILURE -1

int calculate(char *formula, long *calculate_result);

/**
 * サーバープログラムのエントリポイント
 */
int main()
{
    // socketを作成
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        perror("socket creation failed.");
        return 1;
    }

    // SO_REUSEADDRを設定して、TIME_WAIT状態のソケットを即時再利用できるようにする
    const int yes = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
    {
        perror("setsockopt failed.");
        close(fd);
        return 1;
    }

    // アドレス構造体の設定
    struct sockaddr_in addr;
    memset((struct sockaddr *)&addr, 0, sizeof(addr));
    // Address Family Internet: IPv4プロトコルの使用
    addr.sin_family = AF_INET;
    // ポート番号の設定
    addr.sin_port = htons(HTTP_PORT);
    // すべてのインターフェースでLISTENする
    addr.sin_addr.s_addr = INADDR_ANY;

    // bind
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind failed.");
        return 1;
    }

    // listen
    if (listen(fd, 5) < 0)
    {
        perror("listen failed.");
        return 1;
    }

    // accept
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    int client_fd = accept(fd, (struct sockaddr *)&client_addr, &len);
    if (client_fd < 0)
    {
        perror("accept failed");
        return 1;
    }

    // HTTPリクエストを読み取る
    char buffer[READ_BUFFER_SIZE];
    ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read < 0)
    {
        perror("read failed.");
        close(client_fd);
        close(fd);
        return 1;
    }
    buffer[bytes_read] = '\0';

    printf("Received request:\n-------------\n%s\n-------------\n", buffer);

    // ヘッダー部分を解析
    // HttpRequest構造体にリクエスト情報を格納
    HttpRequest *request = new_http_request();
    parse_http_request(request, buffer);

    // 計算
    long calculate_result = 0;
    int result = CALCULATE_FAILURE;
    for (size_t i = 0; i < request->path_parameter->count; i++)
    {
        if (strcmp(request->path_parameter->entries[i]->key, "query") == 0)
        {
            result = calculate(request->path_parameter->entries[i]->value, &calculate_result);
            break;
        }
    }

    // HTTPレスポンス生成
    int http_status_code = -99;
    if (result == CALCULATE_SUCCESS)
    {
        http_status_code = 200;
    }
    else if (result == CALCULATE_FAILURE)
    {
        http_status_code = 400;
    }
    else
    {
        http_status_code = 500;
    }

    // write
    char response_header_buffer[RESPONSE_HEADER_BUFFER_SIZE];
    memset(response_header_buffer, 0, sizeof(response_header_buffer));
    char response_body_buffer[RESPONSE_BODY_BUFFER_SIZE];

    // ボディ文字列の作成
    int body_length = 0;
    if (http_status_code == 200)
    {
        // 計算結果を文字列に変換
        body_length = snprintf(response_body_buffer, RESPONSE_BODY_BUFFER_SIZE, "%ld", calculate_result);
    }
    else
    {
        // 計算不可能な場合、ボディは空
        response_body_buffer[0] = '\0';
    }

    // ヘッダー文字列の作成
    const char *http_status_message = get_http_status_message(http_status_code);
    snprintf(response_header_buffer, RESPONSE_HEADER_BUFFER_SIZE, "HTTP/1.1 %d %s\r\nContent-Length: %d\r\n\r\n",
             http_status_code,
             http_status_message,
             body_length);

    // レスポンスの書き込み
    printf("Sent response:\n-------------\n%s%s\n-------------\n", response_header_buffer, response_body_buffer);
    write(client_fd, response_header_buffer, strlen(response_header_buffer));
    write(client_fd, response_body_buffer, strlen(response_body_buffer));

    // close
    free_http_request(request);
    close(client_fd);
    close(fd);

    return 0;
}

/**
 * @return 計算可能な場合0、計算不可能な場合-1
 */
int calculate(char *formula_str, long *calculate_result)
{
    if (formula_str == NULL)
    {
        return CALCULATE_FAILURE;
    }

    char *endptr = NULL;
    char *startptr = formula_str;
    int error_value = CALCULATE_SUCCESS;

    long result = strtol(startptr, &endptr, 10);
    if (startptr == endptr)
    {
        perror("inital operand fetch error");
        // オペランドの取得失敗
        return CALCULATE_FAILURE;
    }
    startptr = endptr;

    while (1)
    {
        if (*endptr == '\0')
        {
            break;
        }

        char operator = *endptr;
        long operand = strtol(startptr, &endptr, 10);
        if (startptr == endptr)
        {
            // オペランドの取得失敗
            perror("operand fetch error");
            error_value = CALCULATE_FAILURE;
            break;
        }

        // 演算子ごとの処理
        if (operator == '+')
        {
            result += operand;
        }
        else if (operator == '-')
        {
            result -= operand;
        }
        else
        {
            perror("operand fetch error");
            error_value = CALCULATE_FAILURE;
        }
        startptr = endptr;
    }
    *calculate_result = result;
    return error_value;
}