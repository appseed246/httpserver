#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define HTTP_PORT 8080
#define READ_BUFFER_SIZE 4096

int main()
{
    // socketを作成
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        perror("socket creation failed.");
        return 1;
    }

    // アドレス構造体の設定
    struct sockaddr_in server_addr;
    memset((struct sockaddr *)&server_addr, 0, sizeof(server_addr));
    // Address Family Internet: IPv4プロトコルの使用
    server_addr.sin_family = AF_INET;
    // ポート番号の設定
    server_addr.sin_port = htons(HTTP_PORT);

    // サーバー側IPアドレスの指定
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0)
    {
        perror("Invalid address");
        return 1;
    }

    // connect()
    if (connect(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection Failed");
        return 1;
    }

    // write()
    char *message = "GET /calc?query=2+10 HTTP/1.1\r\n";
    char buffer[READ_BUFFER_SIZE];
    write(fd, message, strlen(message));

    // read()
    int valread = read(fd, buffer, READ_BUFFER_SIZE);
    if (valread > 0)
    {
        printf("Received response:\n-------------\n%s\n-------------\n", buffer);
    }

    // close()
    close(fd);

    return 0;
}