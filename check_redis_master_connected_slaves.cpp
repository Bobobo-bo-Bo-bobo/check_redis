#include "check_redis_config.h"
#include "redis_info.hpp"
#include "redis_slave_info.hpp"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include <hiredis.h>
#include <iostream>
#include <string>
#include <vector>
#include <climits>

const char *short_options = "SH:hp:t:c:w:";
static struct option long_options[] = {
    { "host", required_argument, 0, 'H' },
    { "help", no_argument, 0, 'h' },
    { "port", required_argument, 0, 'p' },
    { "timeout", required_argument, 0, 't' },
    { "slaves", no_argument, 0, 'S' },
    { "critical", required_argument, 0, 'c' },
    { "warn", required_argument, 0, 'w' },
    { NULL, 0, 0, 0 },
};

void usage(void) {
    std::cout << "check_redis_master_connected_slaves version " << CHECK_REDIS_VERSION << std::endl;
    std::cout << std::endl;
    std::cout << "Copyright (c) by Andreas Maus <maus@ypbind.de>" << std::endl;
    std::cout << "This program comes with ABSOLUTELY NO WARRANTY." << std::endl;
    std::cout << std::endl;
    std::cout << "check_redis_master_connected_slaves is distributed under the terms of the GNU General" << std::endl;
    std::cout << "Public License Version 3. (http://www.gnu.org/copyleft/gpl.html)" << std::endl;
    std::cout << std::endl;
    std::cout << "Check if number of connected slaves and the amount of missing data on the slaves." << std::endl;
    std::cout << std::endl;
    std::cout << "Usage: check_redis_master_connected_slaves [-h|--help] -H <host>|--host=<host> [-p <port>|--port=<port>]" <<std::endl;
    std::cout << "         [-t <sec>|--timeout=<sec>] [-c <lim>|--critical=<lim>] [-w <lim>|--warn=<lim>] [-S|--slaves] " << std::endl;
    std::cout << std::endl;
    std::cout << "  -h                  This text." << std::endl;
    std::cout << "  --help" << std::endl;
    std::cout << std::endl;
    std::cout << "  -S                  Check number of connected slaves." << std::endl;
    std::cout << "  --slaves" << std::endl;
    std::cout << std::endl;
    std::cout << "  -H <host>           Redis server to connect to." << std::endl;
    std::cout << "  --host=<host>       This option is mandatory." << std::endl;
    std::cout << std::endl;
    std::cout << "  -p <port>           Redis port to connect to," << std::endl;
    std::cout << "  --port=<port>       Default: " << DEFAULT_REDIS_PORT << std::endl;
    std::cout << std::endl;
    std::cout << "  -c <lim>            Report critical condition if master has <lim> slaves or less." << std::endl;
    std::cout << "  --critical=<lim>    Or if missing data is greater or equal than <lim> (if -S/--slave is used)" << std::endl;
    std::cout << "                      Default: " << DEFAULT_REDIS_MASTER_CONNECTED_SLAVES_CRITICAL << " or " << DEFAULT_REDIS_SLAVE_OFFSET_CRITICAL << " bytes" << std::endl;
    std::cout << std::endl;
    std::cout << "  -t <sec>            Connection timout in seconds." << std::endl;
    std::cout << "  --timeout=<sec>     Default: " << DEFAULT_REDIS_CONNECT_TIMEOUT << std::endl;
    std::cout << std::endl;
    std::cout << "  -w <lim>            Report warning condition if master has <lim> slaves or less." << std::endl;
    std::cout << "  --warn=<lim>        or if missing data is greater or equal than <lim> (if -D/--data is used)" << std::endl;
    std::cout << "                      Default: " << DEFAULT_REDIS_MASTER_CONNECTED_SLAVES_WARNING << " or " << DEFAULT_REDIS_SLAVE_OFFSET_WARNING << " bytes" << std::endl;
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
    bool slave_check = false;
    long long warn_limit = -1;
    long long crit_limit = -1;
    int connected_slaves = -1;
    int slv_warn = DEFAULT_REDIS_MASTER_CONNECTED_SLAVES_WARNING;
    int slv_crit = DEFAULT_REDIS_MASTER_CONNECTED_SLAVES_CRITICAL;
    long long warn_delta = DEFAULT_REDIS_SLAVE_OFFSET_WARNING;
    long long crit_delta = DEFAULT_REDIS_SLAVE_OFFSET_CRITICAL;
    long long max_delta = LLONG_MIN;
    long slave_online = 0;
    long slave_wait_bgsave = 0;
    long slave_send_bulk = 0;
    std::vector<RedisSlaveInfo> slaves;
    int slave_state = REDIS_SLAVE_STATE_UNKNOWN;
    std::string mip;
    int mport;
    long long mofs;
    long long delta;
    long long master_offset = -1;

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
            case 'S': {
                          slave_check = true;
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
            case 'c': {   
                          std::string c_str{ optarg };
                          std::string::size_type remain;
                          crit_limit = std::stoll(c_str, &remain);
                          if (remain != c_str.length()) {
                              std::cerr << "Error: Can't convert critical threshold " << optarg << " to a number" << std::endl;
                              return STATUS_UNKNOWN;
                          }
                          break;
                      }
            case 'w': {   
                          std::string w_str{ optarg };
                          std::string::size_type remain;
                          warn_limit = std::stoll(w_str, &remain);
                          if (remain != w_str.length()) {
                              std::cerr << "Error: Can't convert warning threshold " << optarg << " to a number" << std::endl;
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

    if (slave_check) {
        if (warn_limit != -1) {
            slv_warn = warn_limit;
        }
        if (crit_limit != -1) {
            slv_crit = crit_limit;
        }
    } else {
        if (warn_limit != -1) {
            warn_delta = warn_limit;
        }
        if (crit_limit != -1) {
            crit_delta = crit_limit;
        }
    }

    if (slave_check) {
        if (slv_warn < 0) {
            std::cerr << "Error: Warning threshold must be greater than 0" << std::endl;
            return STATUS_UNKNOWN;
        }
        if (slv_crit < 0) {
            std::cerr << "Error: Critical threshold must be greater than 0" << std::endl;
            return STATUS_UNKNOWN;
        }
        if (slv_crit > slv_warn) {
            std::cerr << "Error: Critical threshold must be less or equal than warning threshold for number of connected slaves" << std::endl;
            return STATUS_UNKNOWN;
        }
    } else {
        if (warn_delta <= 0) {
            std::cerr << "Error: Warning threshold must be greater or equal than 0" << std::endl;
            return STATUS_UNKNOWN;
        }
        if (crit_delta <= 0) {
            std::cerr << "Error: Critical threshold must be greater or equal than 0" << std::endl;
            return STATUS_UNKNOWN;
        }
        if (crit_delta < warn_delta) {
            std::cerr << "Error: Critical threshold must be greater or equal than warning threshold" << std::endl;
            return STATUS_UNKNOWN;
        }
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

    connected_slaves = role.GetNumberOfConnectedSlaves();

    if (role.GetRole() != REDIS_ROLE_MASTER) {
        std::cout << "Not running as a master" << std::endl;
        return STATUS_CRITICAL;
    }

    slaves = role.GetSlaveInformation();
    _rc = STATUS_OK;

    if (slave_check) {
        // Note: INFO REPLICATION reports all _connected_ slaves, not slaves that _should_ be
        //       connected (something the server can't know).
        //       We consider slaves in states "wait_bgsave" or "send_bulk" as "online" too.
        for (auto slv: slaves) {
            slave_state = slv.GetState();
            if (slave_state == REDIS_SLAVE_STATE_WAIT_BGSAVE) {
                slave_wait_bgsave ++;
            } else if (slave_state == REDIS_SLAVE_STATE_SEND_BULK) {
                slave_send_bulk ++;
            } else if (slave_state == REDIS_SLAVE_STATE_ONLINE) {
                slave_online ++;
            }
        }
        if (connected_slaves <= slv_crit) {
            _rc = STATUS_CRITICAL;
        } else if (connected_slaves <= slv_warn) {
            _rc = STATUS_WARNING;
        }
        std::cout << connected_slaves << " slaves connected to master | connected_slaves=" << role.GetNumberOfConnectedSlaves() << ";;;0";
        std::cout << " slaves_online=" << slave_online << ";;;0" << " slaves_send_bulk=" << slave_send_bulk << ";;;0";
        std::cout << " slaves_wait_bgsave=" << slave_wait_bgsave << ";;;0" << std::endl;
    } else {
        if (slaves.empty()) {
            std::cout << "No slaves connected to this master" << std::endl;
            _rc = STATUS_CRITICAL;
        } else {
            master_offset = role.GetMasterReplicationOffset();
            std::string perfdata{ "" };
            for (auto slv: slaves) {
                mip = slv.GetIP();
                mport = slv.GetPort();
                mofs = slv.GetOffset();
                delta = master_offset - mofs;
                if (std::abs(delta) > max_delta) {
                    max_delta = std::abs(delta - mofs);
                }
                perfdata += "slave_" + mip + "_" + std::to_string(mport) + "_replication_delta=" + std::to_string(delta) + ";";
                perfdata += std::to_string(warn_delta) + ";" + std::to_string(crit_delta) + ";0";
            }
            if (max_delta >= crit_delta) {
                _rc = STATUS_CRITICAL;
            } else if (max_delta >= warn_delta) {
                _rc = STATUS_WARNING;
            }
            std::cout << "Maximum of outstanding data to slaves is " << delta << " bytes | connected_slaves=" << role.GetNumberOfConnectedSlaves() << ";;;0 ";
            std::cout << perfdata << std::endl;
        }
    }
    return _rc;
}

