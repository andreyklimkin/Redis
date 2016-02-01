#include <bits/stdc++.h>
#include "REDIS.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

using namespace std;

const long long TIME = 2000;

void Make_Sockets(int &listener, sockaddr_in &addr) {
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        perror("socket");
        exit(1);
    }
    fcntl(listener, F_SETFL, O_NONBLOCK);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(6379);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listener, (sockaddr *) &addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(2);
    }
    listen(listener, 8);
}

void Listen_Clients(int listener, fd_set& readset, set<int> &clients) {
    FD_ZERO(&readset);
    FD_SET(listener, &readset);
    for (auto next : clients) {
        FD_SET(next, &readset);
    }
    timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    int maxfd = max(listener, *(max_element(clients.begin(), clients.end())));
    select(maxfd + 1, &readset, NULL, NULL, &timeout);
    if (FD_ISSET(listener, &readset)) {
        int new_sock = accept(listener, NULL, NULL);
        if (new_sock < 0) {
            perror("accept");
            exit(3);
        }
        fcntl(new_sock, F_SETFL, O_NONBLOCK);
        clients.insert(new_sock);
    }
}

// Delete \0 in the end, if it is necessary
void Check_Last_Symb(User &user) {
    while(user.buf.size() && user.buf.back() == 0) {
        user.buf.pop_back();
    }
}

void Make_Request(map<int, User> &users, Redis_Api &R, char* buf, int ready, vector<int> &del_clients) {
    ssize_t bytes_read = recv(ready, buf, R.MAX_LEN, 0);
    //bytes_read = strlen(buf);
    if (strlen(buf) <= 0 || bytes_read <= 0) {
        del_clients.push_back(ready);
        return;
    } else {
        if (users.find(ready) != users.end()) {
            users[ready].buf.insert(users[ready].buf.end(), buf, buf + bytes_read);
        } else {
            User Nw;
            Nw.buf.insert(Nw.buf.begin(), buf, buf + bytes_read);
            users[ready] = Nw;
        }
        Check_Last_Symb(users[ready]);
        while (users[ready].buf.size()) {
            Parse(users[ready]);
            if (!users[ready].wait) {
                string req;
                bool is_finish = false;
                is_finish = R.MakeQuery(users[ready], req);
                //char *ans[R.MAX_LEN];
                //ans[0] = 0;
                //strcat(ans, req.c_str());
                //send(ready, req.c_str(), req.length(), 0);
                if (is_finish) {
                    R.Game_Over();
                    exit(0);
                }
                send(ready, req.c_str(), req.length(), 0);
            }
        }
    }
}

void Check_Time(Redis_Api &R, long long &time_bef, long long &time) {
    struct timeval t;
    gettimeofday(&t, NULL);
    long mt = (long)t.tv_sec * 1000 + t.tv_usec / 1000;
    time = mt;
    if(time - time_bef > TIME) {
        time_bef = time;
        R.Write_changes_to_file();
    }
}

void Del_Viewed(Redis_Api &R, vector<int> &del_clients, set<int> &clients, long long time_bef, long long time) {
    Check_Time(R, time_bef, time);
    for (auto del : del_clients) {
        close(del);
        clients.erase(del);
    }
}

int main(int argc, char* argv[]) {
    Redis_Api R;
    if(argc > 1) {
        R.MAX_LEN = (size_t) stoll(argv[1]);
    }
    char* buf = new char[R.MAX_LEN];
    int listener;
    long long time_bef = 0, time = 0; // for timer
    sockaddr_in addr;
    Make_Sockets(listener, addr);

    map<int, User> users;
    set<int> clients;
    clients.clear();

    while (1) {
        fd_set readset;
        Listen_Clients(listener, readset, clients);
        vector<int> del_clients;
        for (auto ready : clients) {
            if (FD_ISSET(ready, &readset)) {
                Make_Request(users, R, buf, ready, del_clients);
            }
        }
        Del_Viewed(R, del_clients, clients, time_bef, time);
    }
}
