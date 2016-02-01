#include <bits/stdc++.h>
#include "REDIS.h"

using namespace std;

void GOStart(User&);
void GOARR_SZ(User&);
void GOARR_BODY(User&);
void GOSTR_SZ(User&);
void GOSTR_BODY(User&);
void GO_FINISH(User&);
void GO_FAKE1(User&);
void GO_FAKE2(User&);
void GO_FAKE3(User&);

// request is incorrect
void Error(User& user) {
    user.wait = false;
    user.arr_size = 0;
    user.str_size = 0;
    user.stat = START;
    user.str.clear();
    user.str.push_back("-Error message\r\n");
    while(user.buf.size() && user.buf.front() != '*') {
        user.buf.pop_front();
    }
}

void Parse(User& user) {
    switch (user.stat) {
        case START: {
            GOStart(user);
            break;
        }
        case ARR_SZ: {
            GOARR_SZ(user);
            break;
        }
        case ARR_BODY: {
            GOARR_BODY(user);
            break;
        }
        case STR_BODY: {
            GOSTR_BODY(user);
            break;
        }
        case STR_SIZE: {
            GOSTR_SZ(user);
            break;
        }
        case FAKE1: {
            GO_FAKE1(user);
            break;
        }
        case FAKE2: {
            GO_FAKE2(user);
            break;
        }
        case FAKE3: {
            GO_FAKE3(user);
            break;
        }
        default: {
            Error(user);
        }
    }
}

void GOStart(User& user) {
    if(!user.buf.size()) {
        return;
    }
    char c = user.buf.front();
    user.buf.pop_front();
    if(c != '*') {
        Error(user);
        return;
    } else {
        GOARR_SZ(user);
    }
}

void GOARR_SZ(User& user) {
    char c;
    user.stat = ARR_SZ;
    while(user.buf.size() && isdigit(c = user.buf.front())) {
        user.buf.pop_front();
        user.arr_size = user.arr_size * 10 + c - '0';
    }
    if(!user.buf.size()) {
        return;
    } else {
        c = user.buf.front();
        user.buf.pop_front();
        if (c != '\r') {
            Error(user);
            return;
        } else {
            user.stat = FAKE1;
            GO_FAKE1(user);
            return;
        }
    }
}

void GO_FAKE1(User& user) {
    if(!user.buf.size()) {
        return;
    }
    char c = user.buf.front();
    user.buf.pop_front();
    if(c != '\n') {
        Error(user);
        return;
    }
    user.stat = ARR_BODY;
    GOARR_BODY(user);
}


void GOARR_BODY(User& user) {
    if(!user.buf.size()) {
        return;
    }
    char c = user.buf.front();
    user.buf.pop_front();
    if(c != '$') {
        Error(user);
        return ;
    } else {
        user.stat = STR_SIZE;
        GOSTR_SZ(user);
        return;
    }
}

void GOSTR_SZ(User& user) {
    char c;
    user.stat = STR_BODY;
    while(user.buf.size() && isdigit(c = user.buf.front())) {
        user.buf.pop_front();
        user.str_size = user.str_size * 10 + c - '0';
    }
    if(!user.buf.size()) {
        return;
    } else {
        c = user.buf.front();
        user.buf.pop_front();
        if (c != '\r') {
            Error(user);
            return;
        } else {
            user.stat = FAKE2;
            GO_FAKE2(user);
            return;
        }
    }
}

void GO_FAKE2(User& user) {
    if(!user.buf.size()) {
        return;
    }
    char c = user.buf.front();
    user.buf.pop_front();
    if(c != '\n') {
        Error(user);
        return;
    }
    user.stat = STR_BODY;
    --user.arr_size;
    user.str.push_back("");
    GOSTR_BODY(user);
}

void GOSTR_BODY(User& user) {
    if (!user.buf.size()) {
        return;
    }
    char c;
    while (user.buf.size() && user.str_size) {
        c = user.buf.front(), user.buf.pop_front();
        --user.str_size;
        user.str.back() += c;
    }
    if(!user.buf.size()) {
        return;
    }
    c = user.buf.front(), user.buf.pop_front();
    if(c != '\r') {
        Error(user);
    } else {
        user.stat = FAKE3;
        GO_FAKE3(user);
    }
}

void GO_FAKE3(User& user) {
    if(!user.buf.size()) {
        return;
    }
    char c = user.buf.front();
    user.buf.pop_front();
    if(c != '\n') {
        Error(user);
        return;
    }
    if(!user.arr_size) {
        user.stat = FINISH;
        GO_FINISH(user);
    } else {
        user.stat = ARR_BODY;
        GOARR_BODY(user);
    }
}

void GO_FINISH(User& user) {
    if(user.buf.size() && user.buf[0] == 0) {
        user.buf.pop_front();
    }
    user.arr_size = 0;
    user.str_size = 0;
    user.stat = START;
    user.wait = false;
    return;
}