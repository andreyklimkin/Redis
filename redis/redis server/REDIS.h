#include <bits/stdc++.h>
#include "PARSE.h"

#ifndef REDIS
#define REDIS

using namespace std;

class Redis_Api {
private:
    unordered_map<string, string> database; // Our Big Redis Key-Value database
    unordered_map<string, string> changes; // Last changes, no more than NEED_TO_CHANGE
    string copy_file;
    string changes_file;
    size_t database_file_size = 0;
    size_t change_file_size = 0;

    // copy file with database that will help us to make a snapshot
    ofstream database_copy_write;
    ifstream database_copy_read;

    ofstream changes_write;
    ifstream changes_read;
    size_t FILE_TOO_HUGE = 1000;  // if our change_file consists a lot of unnecessary information

    void Snapshot();
    void Recreate_change_file();
    void Get_Bulk(char *bulk, string& answer);

public:
    size_t MAX_LEN = 1e6;
    Redis_Api();
    char* Get(const string &key);
    void Set(const string& key, const string& value);
    bool MakeQuery(User& user, string& req);
    void Write_changes_to_file();
    void Game_Over();
    ~Redis_Api();
};

#endif