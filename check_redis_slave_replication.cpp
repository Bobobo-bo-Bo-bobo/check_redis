#include "check_redis_config.h"
#include "redis_info.hpp"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include <hiredis.h>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>

const char *short_options = "H:hp:t:c:w:";
static struct option long_options[] = {
    { "host", required_argument, 0, 'H' },
    { "help", no_argument, 0, 'h' },
    { "port", required_argument, 0, 'p' },
    { "timeout", required_argument, 0, 't' },
    { "critical", required_argument, 0, 'c' },
    { "warning", required_argument, 0, 'w' },
    { "data", no_argument, 0, 'D' },
    { NULL, 0, 0, 0 },
};

void usage(void) {
    std::cout << "check_redis_slave_replication version " << CHECK_REDIS_VERSION << std::endl;
    std::cout << std::endl;
    std::cout << "Copyright (c) by Andreas Maus <maus@ypbind.de>" << std::endl;
    std::cout << "This program comes with ABSOLUTELY NO WARRANTY." << std::endl;
    std::cout << std::endl;
    std::cout << "check_redis_slave is distributed under the terms of the GNU General" << std::endl;
    std::cout << "Public License Version 3. (http://www.gnu.org/copyleft/gpl.html)" << std::endl;
    std::cout << std::endl;
    std::cout << "Check the last time a slave was connected to a master or the amount of data not replicated yet." << std::endl;
    std::cout << std::endl;
    std::cout << "Usage: check_redis_slave_replication [-h|--help] -H <host>|--host=<host> [-p <port>|--port=<port>]" <<std::endl;
    std::cout << "         [-t <sec>|--timeout=<sec>] [-c <lim>|--critical=<lim>] [-w <lim>|--warn=<lim>]" << std::endl;
    std::cout << std::endl;
    std::cout << "  -h                  This text." << std::endl;
    std::cout << "  --help" << std::endl;
    std::cout << std::endl;
    std::cout << "  -D                  Check for missing data instead of last sync" << std::endl;
    std::cout << "  --data" << std::endl;
    std::cout << std::endl;
    std::cout << "  -H <host>           Redis server to connect to." << std::endl;
    std::cout << "  --host=<host>       This option is mandatory." << std::endl;
    std::cout << std::endl;
    std::cout << "  -c <lim>            Report critical condition if last master was <lim> or more ago." << std::endl;
    std::cout << "  --critical=<lim>    or if missing data is greater or equal than <lim> (if -D/--data is used)" << std::endl;
    std::cout << "                      Default: " << DEFAULT_REDIS_IO_AGO_CRITICAL << " sec or " << DEFAULT_REDIS_SLAVE_OFFET_CRITICAL <<" bytes" << std::endl;
    std::cout << std::endl;
    std::cout << "  -p <port>           Redis port to connect to," << std::endl;
    std::cout << "  --port=<port>       Default: " << DEFAULT_REDIS_PORT << std::endl;
    std::cout << std::endl;
    std::cout << "  -t <sec>            Connection timout in seconds." << std::endl;
    std::cout << "  --timeout=<sec>     Default: " << DEFAULT_REDIS_CONNECT_TIMEOUT << std::endl;
    std::cout << std::endl;
    std::cout << "  -w <sec>            Report critical condition if last master was <sec> or more ago." << std::endl;
    std::cout << "  --warn=<sec>        or if missing data is greater or equal than <lim> (if -D/--data is used)" << std::endl;
    std::cout << "                      Default: " << DEFAULT_REDIS_IO_AGO_WARNING << " sec or " << DEFAULT_REDIS_SLAVE_OFFSET_WARNING << " bytes" << std::endl;
    std::cout << std::endl;
}

int main(int argc, char **argv) {
    std::string hostname { "" };
    int port = DEFAULT_REDIS_PORT;
    redisContext *conn = NULL;
    redisReply *reply = NULL;
    int t_sec = DEFAULT_REDIS_CONNECT_TIMEOUT;
    long long warn_delta = DEFAULT_REDIS_SLAVE_OFFSET_WARNING;
    long long crit_delta = DEFAULT_REDIS_SLAVE_OFFET_CRITICAL;
    int last_io_warn = DEFAULT_REDIS_IO_AGO_WARNING;
    int last_io_crit = DEFAULT_REDIS_IO_AGO_CRITICAL;
    long long warn_limit = -1;
    long long crit_limit = -1;
    int option_index = 0;
    int _rc;
    int last_io = -1;
    bool data_check = false;
    long long data_delta = -1;

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
            case 'D': {
                          data_check = true;
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
            case 'c': {
                          std::string c_str{ optarg };
                          std::string::size_type remain;
                          crit_limit = std::stoll(c_str, &remain);
                          if (remain != c_str.length()) {
                              std::cerr << "Error: Can't convert critical threshold " << optarg << " to a number" << std::endl;
                              return STATUS_UNKNOWN;
                          }
                          if (crit_limit <= 0) {
                              std::cerr << "Error: Critical threshold must be greater than 0" << std::endl;
                              return STATUS_UNKNOWN;
                          }
                          break;
                      }
            case 'w': {
                          std::string w_str{ optarg };
                          std::string::size_type remain;
                          warn_limit = std::stoi(w_str, &remain);
                          if (remain != w_str.length()) {
                              std::cerr << "Error: Can't convert warning threshold " << optarg << " to a number" << std::endl;
                              return STATUS_UNKNOWN;
                          }
                          if (warn_limit <= 0) {
                              std::cerr << "Error: Warning threshold must be greater than 0" << std::endl;
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

    if (data_check) {
        if (warn_limit != -1) {
            warn_delta = warn_limit;
        }
        if (crit_limit != -1) {
            crit_delta = crit_limit;
        }
    } else {
        if (warn_limit != -1) {
            last_io_warn = warn_limit;
        }
        if (crit_limit != -1) {
            last_io_crit = crit_limit;
        }
    }

    if (hostname == "") {
        std::cerr << "Error: No host specified" << std::endl;
        std::cerr << std::endl;
        usage();
        return STATUS_UNKNOWN;
    }

    if ((last_io_crit < last_io_warn) || (crit_delta < warn_delta)) {
        std::cerr << "Error: Critical threshold must be greater or equal than warning threshold" << std::endl;
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
        std::cout << "Not running as a slave | connected_slaves=" << role.GetNumberOfConnectedSlaves() << ";;;0 master_last_io_ago=";
        std::cout << role.GetMasterLastIOAgo() << "s;" << last_io_warn << ";" << last_io_crit << ";0" << std::endl;
        return STATUS_CRITICAL;
    }

    if (role.GetMasterLinkStatus() != REDIS_MASTER_LINK_STATUS_UP) {
        std::cout << "Slave is connected but link to master at " << role.GetMasterHost() << ", port " << role.GetMasterPort();
        std::cout << " is down since " << role.GetMasterLinkDownSince() << " seconds | connected_slaves=" << role.GetNumberOfConnectedSlaves() << ";;;0";
        std::cout << " master_last_io_ago=" << role.GetMasterLastIOAgo() << "s;" << last_io_warn << ";" << last_io_crit << ";0";
        std::cout << std::endl;
        return STATUS_CRITICAL;
    }

    _rc = STATUS_OK;
    if (data_check) {
        data_delta = role.GetMissingData();
        std::cout << "Slave data is " << std::abs(data_delta) << " bytes behind master at " << role.GetMasterHost() << ", port " << role.GetMasterPort();
        std::cout << " | slave_replication_delta=" << data_delta << "B;" << warn_delta << ";" << crit_delta;
        if (std::abs(data_delta) >= crit_delta) {
            _rc = STATUS_CRITICAL;
        }
        if (std::abs(data_delta) >= warn_delta) {
            _rc = STATUS_WARNING;
        }
    } else {
        last_io = role.GetMasterLastIOAgo();
        std::cout << "Last sync with master at " << role.GetMasterHost() << ", port " << role.GetMasterPort() << " was " << last_io;
        std::cout << " seconds ago";
        std::cout << " | master_last_io_ago=" << role.GetMasterLastIOAgo() << "s;" << last_io_warn << ";" << last_io_crit << ";0";

        if (last_io >= last_io_crit) {
            _rc = STATUS_CRITICAL;
        }
        if (last_io >= last_io_warn) {
            _rc = STATUS_WARNING;
        }
    }
    std::cout << " connected_slaves=" << role.GetNumberOfConnectedSlaves() << ";;;0";
    std::cout << std::endl;

    return _rc;
}

