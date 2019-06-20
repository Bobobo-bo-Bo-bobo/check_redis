#include "check_redis_config.h"
#include "util.hpp"
#include <stdio.h>
#include <stdlib.h>

#include <hiredis.h>
#include <iostream>
#include <string>
#include <vector>

// Options
// -H <host>
// -p <port>
// -t <timeout>

int main(int argc, char **argv) {
    const char *hostname = (argc > 1) ? argv[1] : "127.0.0.1";
    int port = (argc > 2) ? atoi(argv[2]) : 6379;
    redisContext *conn;
    redisReply *reply;
    struct timeval timeout = { 1, 500000 };
    std::string redis_info;
    
    conn = redisConnectWithTimeout(hostname, port, timeout);
    if (!conn) {
        std::cout << "Error: Can't connect to redis server" << std::endl;
        return STATUS_CRITICAL;
    }
    if (conn->err) { 
        std::cout << "Error: Can't connect to redis server: " << conn->errstr << std::endl;
        return STATUS_CRITICAL;
    }
    
    reply = (redisReply *) redisCommand(conn, "INFO");
    if (reply->type == REDIS_REPLY_ERROR) {
        std::cout << "Error: INFO command to redis server returned an error: " << reply->str << std::endl;
        freeReplyObject(reply);
        return STATUS_CRITICAL;
    }
    
    redis_info = reply->str;
    freeReplyObject(reply);
    redisFree(conn);
    
    std::vector<std::string> splitted = split_lines(redis_info);
    for (auto line: splitted) {
        std::string::size_type role_pos = line.find("role:");
        if (role_pos != std::string::npos) {
            std::string role_line = line;
            std::string role_str { "role:" };
            std::string role = role_line.erase(0, role_pos + role_str.length());
            std::cout << line << " / " << role_pos << " / " << role << std::endl;
        }
    }
    return 0;
}

