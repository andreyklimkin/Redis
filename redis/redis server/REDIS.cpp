#include <bits/stdc++.h>
#include <sys/time.h>
#include "REDIS.h"
#include <cstdlib>

// If out log more than TOO_MUCH CHANGES we must send changes to our database
const size_t TOO_MUCH_CHANGES = 5;

using namespace std;

Redis_Api::Redis_Api() {
    copy_file = "RedisDatabaseCopy.txt";
    changes_file = "LastChanges.txt";
    Snapshot();

    // We must copy all information from our disk_file to our database_map
    int len_key, len_value;
    while (database_copy_read >> len_key >> len_value) {
        string key;
        string value;
        database_copy_read.get(); // read a space between length and strings
        while(len_key--) {
            key.push_back(database_copy_read.get());
        }
        while(len_value--) {
            value.push_back(database_copy_read.get());
        }
        database[key] = value;
    }
}

// Merge our database with change_file in our copy of database and then rewrite the result to our database
void Redis_Api::Snapshot() {
    system("cp RedisDatabase.txt RedisDatabaseCopy.txt");
    database_copy_write.open(copy_file, ios::app);
    database_copy_read.open(copy_file);
    changes_read.open(changes_file);
    int len_key, len_value;
    while (changes_read >> len_key >> len_value) {
        string key;
        string value;
        database_copy_write << len_key << " " << len_value << " ";
        changes_read.get(); // read a space between length and strings
        while(len_key--) {
            key.push_back(changes_read.get());
        }
        while(len_value--) {
            value.push_back(changes_read.get());
        }
        database_copy_write << key << value;
    }
    database_copy_write.close();
    system("cp RedisDatabaseCopy.txt RedisDatabase.txt");
    changes_write.open(changes_file);
}

void Redis_Api::Write_changes_to_file() {
    change_file_size += changes.size();
    changes_write.close();
    changes_write.open(changes_file, ios::app);
    for (auto next : changes) {
        changes_write << next.first.length() << " " << next.second.length() << " ";
        changes_write << next.first << next.second;
    }
    changes_write.close();
    changes_read.close();
    changes.clear();
}

void Clean(User& user) {
    user.str.clear();
    user.wait = true;
}

string itoa(int a) {
    string res = "";
    while(a) {
        res += char(a % 10) + '0';
        a /= 10;
    }
    reverse(res.begin(), res.end());
    return res;
}

void Switched_Off(string& req) {
    string s = "The server was switched off";
    req = "$" + itoa(int(s.length())) + "\r\n" + s + "\r\n";
}

// The answer to our client is "req"
bool Redis_Api::MakeQuery(User& user, string &req) {
    string error = "-Error message\r\n";
    if(user.str.size() == 1 && user.str[0] == "SWITCH OFF") {
        Switched_Off(req);
        return true;
    }
    if(user.str.size() == 1 && user.str[0] == error) {
        req = error;
    } else {
        transform(user.str[0].begin(), user.str[0].end(), user.str[0].begin(), ::tolower);
        if (user.str.size() == 2 && user.str[0] == "get") {
            req = Get(user.str[1]);
        } else {
            if (user.str.size() == 3 && user.str[0] == "set") {
                Set(user.str[1], user.str[2]);
                req = "+OK\r\n";
            } else {
                req = error;
            }
        }
    }
    Clean(user);
    return false;
}

//Parse answer to bulk type
void Redis_Api::Get_Bulk(char *bulk, string& s) {
    sprintf(bulk, "$%d\r\n%s\r\n", s.length(), s.c_str());
    bulk[strlen(bulk)] = 0;
}

char* Redis_Api::Get(const string &key) {
    char response[MAX_LEN];
    if (database.find(key) != database.end()) {
        Get_Bulk(response, database[key]);
        return response;
    }
    //There is no such string in our database
    return "$-1\r\n";
}

void Redis_Api::Set(const string &key, const string &value) {
    changes[key] = value;
    database[key] = value;
    timeval t;
    gettimeofday(&t, NULL);
    if (changes.size() >= TOO_MUCH_CHANGES) {
        Write_changes_to_file();
    }
}

void Redis_Api::Game_Over() {
    if (changes.size()) {
        Write_changes_to_file();
    }
    Snapshot();
}

Redis_Api::~Redis_Api() {
    database_copy_read.close();
    database_copy_write.close();
    changes_read.close();
    changes_write.close();
}