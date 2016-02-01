#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <cstring>
#include <unistd.h>

using namespace std;

char first_message[] = "*3\r\n$3\r\nSET\r\n$4\r\nAABB\r\n$1\r\nA\r\n*2\r\n$3\r\nGET\r\n$4\r\nAAB"; 
char second_message[] = "B\r\n323";
char buf[1024];

int main()
{
    int sock;
    struct sockaddr_in addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
    {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(6379);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect");
        exit(2);
    }
    send(sock, first_message, strlen(first_message), 0);
    sleep(5);
    send(sock, second_message, strlen(second_message), 0);
    //send(sock, message_set2, sizeof(message_set2), 0);
    recv(sock, buf, sizeof(buf), 0);
    cout << buf;
    recv(sock, buf, sizeof(buf), 0);
    cout << buf;
    //recv(sock, buf, sizeof(buf), 0);
    //cout << buf;
    //close(sock);
    return 0;
}