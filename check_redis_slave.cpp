#include "check_redis_config.h"
#include "redis_info.hpp"
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
    if (!reply) {
        std::cout << "Error: INFO command to redis server failed" << std::endl;
        redisFree(conn);
        return STATUS_CRITICAL;
    }

    if (reply->type == REDIS_REPLY_ERROR) {
        std::cout << "Error: INFO command to redis server returned an error: " << reply->str << std::endl;
        freeReplyObject(reply);
        redisFree(conn);
        return STATUS_CRITICAL;
    }
    
    redis_info = reply->str;
 
    freeReplyObject(reply);
    redisFree(conn);
 
    RedisRoleInfo role { redis_info };

    if (role.GetRole() != REDIS_ROLE_SLAVE) {
        std::cout << "Not a slave" << std::endl;
        return STATUS_CRITICAL;
    }

    if (role.GetMasterLinkStatus() != REDIS_MASTER_LINK_STATUS_UP) {
        std::cout << "Slave is connected but link to master at " << role.GetMasterHost() << ", port " << role.GetMasterPort() << " is down" << std::endl;
        return STATUS_CRITICAL;
    }

    std::cout << "Slave is connected to master at " << role.GetMasterHost() << ", port " << role.GetMasterPort() << std::endl;
    return STATUS_OK;
}

