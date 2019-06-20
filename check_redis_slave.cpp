#include "check_redis_config.h"
#include "redis_info.hpp"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include <hiredis.h>
#include <iostream>
#include <string>
#include <vector>

const char *short_options = "H:hp:t:";
static struct option long_options[] = {
    { "host", required_argument, 0, 'H' },
    { "help", no_argument, 0, 'h' },
    { "port", required_argument, 0, 'p' },
    { "timeout", required_argument, 0, 't' },
    { NULL, 0, 0, 0 },
};

void usage(void) {
    std::cout << "check_redis_slave version " << CHECK_REDIS_VERSION << std::endl;
    std::cout << std::endl;
    std::cout << "Copyright (c) by Andreas Maus <maus@ypbind.de>" << std::endl;
    std::cout << "This program comes with ABSOLUTELY NO WARRANTY." << std::endl;
    std::cout << std::endl;
    std::cout << "check_redis_slave is distributed under the terms of the GNU General" << std::endl;
    std::cout << "Public License Version 3. (http://www.gnu.org/copyleft/gpl.html)" << std::endl;
    std::cout << std::endl;
    std::cout << "Check if Redis slave is connected to it's master" << std::endl;
    std::cout << std::endl;
    std::cout << "Usage: check_redis_slave [-h|--help] -H <host>|--host=<host> [-p <port>|--port=<port>]" <<std::endl;
    std::cout << "         [-t <sec>|--timeout=<sec>]" << std::endl;
    std::cout << std::endl;
    std::cout << "  -h              This text." << std::endl;
    std::cout << "  --help" << std::endl;
    std::cout << std::endl;
    std::cout << "  -H <host>       Redis server to connect to." << std::endl;
    std::cout << "  --host=<host>   This option is mandatory." << std::endl;
    std::cout << std::endl;
    std::cout << "  -p <port>       Redis port to connect to," << std::endl;
    std::cout << "  --port=<port>   Default: " << DEFAULT_REDIS_PORT << std::endl;
    std::cout << std::endl;
    std::cout << "  -t <sec>        Connection timout in seconds." << std::endl;
    std::cout << "  --timeout=<sec> Default: " << DEFAULT_REDIS_CONNECT_TIMEOUT << std::endl;
    std::cout << std::endl;
}

int main(int argc, char **argv) {
    std::string hostname { "" };
    int port = DEFAULT_REDIS_PORT;
    redisContext *conn = NULL;
    redisReply *reply = NULL;
    int t_sec = DEFAULT_REDIS_CONNECT_TIMEOUT;
    int option_index = 0;
    int _rc;

    while ((_rc = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
        switch (_rc) {
            case 'h': {
                          usage();
                          return STATUS_OK;
                          break;
                      }
            case 'H': {
                          hostname = optarg;
                          break;
                      }
            case 'p': {
                          std::string port_str{ optarg };
                          std::string::size_type remain;
                          port = std::stoi(port_str, &remain);
                          if (remain != port_str.length()) {
                              std::cerr << "Error: Can't convert port " << optarg << " to a number" << std::endl;
                              return STATUS_UNKNOWN;
                          }
                          break;
                      }
            case 't': {
                          std::string t_str{ optarg };
                          std::string::size_type remain;
                          t_sec = std::stoi(t_str, &remain);
                          if (remain != t_str.length()) {
                              std::cerr << "Error: Can't convert timeout " << optarg << " to a number" << std::endl;
                              return STATUS_UNKNOWN;
                          }
                          if (t_sec <= 0) {
                              std::cerr << "Error: Timout must be greater than 0" << std::endl;
                              return STATUS_UNKNOWN;
                          }
                          break;
                      }
            default: {
                          std::cerr << "Error: Unknwon argument " << _rc << std::endl;
                          usage();
                          return STATUS_UNKNOWN;
                          break;
                     }
        }
    }

    if (hostname == "") {
        std::cerr << "Error: No host specified" << std::endl;
        std::cerr << std::endl;
        usage();
        return STATUS_UNKNOWN;
    }

    struct timeval timeout = { t_sec, 0 };
    conn = redisConnectWithTimeout(hostname.c_str(), port, timeout);
    if (!conn) {
        std::cout << "Error: Can't connect to redis server" << std::endl;
        return STATUS_CRITICAL;
    }
    if (conn->err) { 
        std::cout << "Error: Can't connect to redis server: " << conn->errstr << std::endl;
        return STATUS_CRITICAL;
    }
    
    reply = (redisReply *) redisCommand(conn, "INFO REPLICATION");
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
    
    std::string redis_info{ reply->str };
 
    freeReplyObject(reply);
    redisFree(conn);
 
    RedisRoleInfo role { redis_info };

    if ((role.GetRole() != REDIS_ROLE_SLAVE) && (role.GetRole() != REDIS_ROLE_CHAINED_REPLICATION_SLAVE)) {
        std::cout << "Not running as a slave | connected_slaves=" << role.GetNumberOfConnectedSlaves() << ";;;0" << std::endl;
        return STATUS_CRITICAL;
    }

    if (role.GetMasterLinkStatus() != REDIS_MASTER_LINK_STATUS_UP) {
        std::cout << "Slave is connected but link to master at " << role.GetMasterHost() << ", port " << role.GetMasterPort() << " is down since " << role.GetMasterLinkDownSince() << " seconds | connected_slaves=" << role.GetNumberOfConnectedSlaves() << ";;;0" << std::endl;
        return STATUS_CRITICAL;
    }

    std::cout << "Slave is connected to master at " << role.GetMasterHost() << ", port " << role.GetMasterPort() << " | connected_slaves=" << role.GetNumberOfConnectedSlaves() << ";;;0" << std::endl;
    return STATUS_OK;
}

