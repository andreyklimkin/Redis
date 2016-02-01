#include <iostream>
#include <unordered_map>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include <fstream>

#ifndef PARSE
#define PARSE

using namespace std;

//Fake for \r\n
enum state {
    START, ARR_SZ, ARR_BODY, STR_SIZE, STR_BODY, FINISH, ERROR, FAKE1, FAKE2, FAKE3
};


class User{
public:
    deque<char> buf;
    vector<string> str;
    state stat;
    int arr_size, str_size;
    bool wait;
    User() {
        stat = START;
        wait = true;
        arr_size = str_size = 0;
    }
};

void Parse(User& user);

#endif