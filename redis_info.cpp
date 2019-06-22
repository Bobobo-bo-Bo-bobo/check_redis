#include "check_redis_config.h"
#include "redis_info.hpp"
#include "util.hpp"

#include <string>
#include <vector>

RedisRoleInfo::RedisRoleInfo() {
    m_redis_role = REDIS_ROLE_UNKNOWN;
    m_redis_master_host = "";
    m_redis_master_port = 0;
    m_redis_master_link_status = REDIS_MASTER_LINK_STATUS_UNKNOWN;
    m_redis_master_link_down_since = -1;
    m_redis_master_last_io_seconds_ago = -1;
    m_redis_connected_slaves = 0;
    m_master_repl_offset = -1;
    m_slave_repl_offset = -1;
}

RedisRoleInfo::RedisRoleInfo(std::vector<std::string> splitted) {
    m_redis_role = REDIS_ROLE_UNKNOWN;
    m_redis_master_host = "";
    m_redis_master_port = 0;
    m_redis_master_link_status = REDIS_MASTER_LINK_STATUS_UNKNOWN;
    m_redis_master_link_down_since = -1;
    m_redis_master_last_io_seconds_ago = -1;
    m_redis_connected_slaves = 0;
    m_redis_info = splitted;
    m_master_repl_offset = -1;
    m_slave_repl_offset = -1;

    m_parse_info_lines(splitted);
}

RedisRoleInfo::RedisRoleInfo(std::string s) {
    m_redis_role = REDIS_ROLE_UNKNOWN;
    m_redis_master_host = "";
    m_redis_master_port = 0;
    m_redis_master_link_status = REDIS_MASTER_LINK_STATUS_UNKNOWN;
    m_redis_master_link_down_since = -1;
    m_redis_master_last_io_seconds_ago = -1;
    m_redis_connected_slaves = 0;
    m_master_repl_offset = -1;
    m_slave_repl_offset = -1;

    std::vector<std::string> splitted = split_lines(s);
    m_redis_info = splitted;

    m_parse_info_lines(splitted);
}

int RedisRoleInfo::GetRole(void) {
    return m_redis_role;
}

void RedisRoleInfo::m_parse_info_lines(std::vector<std::string> splitted) {
    for (auto line: splitted) {
        // extract role
        std::string::size_type role_pos = line.find(s_role_str);
        if (role_pos != std::string::npos) {
            std::string role_line = line;
            std::string role = role_line.erase(0, role_pos + s_role_str.length());

            if (role == "master") {
                m_redis_role = REDIS_ROLE_MASTER;
            } else if (role == "slave") {
                m_redis_role = REDIS_ROLE_SLAVE;
            } else {
                m_redis_role = REDIS_ROLE_UNKNOWN;
            }
        }
        std::string::size_type mh_pos = line.find(s_master_host_str);
        if (mh_pos != std::string::npos) {
            std::string mh_line = line;
            m_redis_master_host = mh_line.erase(0, mh_pos + s_master_host_str.length());
        }

        std::string::size_type mp_pos = line.find(s_master_port_str);
        if (mp_pos != std::string::npos) {
            std::string mp_line = line;
            std::string mp = mp_line.erase(0, mp_pos + s_master_port_str.length());
            m_redis_master_port = std::stoi(mp, nullptr);
        }

        std::string::size_type ml_pos = line.find(s_master_link_status_str);
        if (ml_pos != std::string::npos) {
            std::string ml_line = line;
            std::string mls = ml_line.erase(0, ml_pos + s_master_link_status_str.length());
            if (mls == "up") {
                m_redis_master_link_status = REDIS_MASTER_LINK_STATUS_UP;
            } else if (mls == "down") {
                m_redis_master_link_status = REDIS_MASTER_LINK_STATUS_DOWN;
            } else {
                m_redis_master_link_status = REDIS_MASTER_LINK_STATUS_UNKNOWN;
            }
        }

        std::string::size_type ld_pos = line.find(s_master_link_down_since_str);
        if (ld_pos != std::string::npos) {
            std::string ld_line = line;
            std::string ld = ld_line.erase(0, ld_pos + s_master_link_down_since_str.length());
            m_redis_master_link_down_since = std::stoi(ld, nullptr);
        }

        std::string::size_type ls_pos = line.find(s_master_last_io_seconds_ago_str);
        if (ls_pos != std::string::npos) {
            std::string ls_line = line;
            std::string ls = ls_line.erase(0, ls_pos + s_master_last_io_seconds_ago_str.length());
            m_redis_master_last_io_seconds_ago = std::stoi(ls, nullptr);
        }

        std::string::size_type cs_pos = line.find(s_connected_slaves_str);
        if (cs_pos != std::string::npos) {
            std::string cs_line = line;
            std::string cs = cs_line.erase(0, cs_pos + s_connected_slaves_str.length());
            m_redis_connected_slaves = std::stoi(cs, nullptr);
        }

        std::string::size_type mrpl_pos = line.find(s_master_repl_offset_str);
        if (mrpl_pos != std::string::npos) {
            std::string mrpl_line = line;
            std::string mrpl = mrpl_line.erase(0, mrpl_pos + s_master_repl_offset_str.length());
            m_master_repl_offset = std::stoll(mrpl, nullptr);
        }

        std::string::size_type srpl_pos = line.find(s_slave_repl_offset_str);
        if (srpl_pos != std::string::npos) {
            std::string srpl_line = line;
            std::string srpl = srpl_line.erase(0, srpl_pos + s_slave_repl_offset_str.length());
            m_slave_repl_offset = std::stoll(srpl, nullptr);
        }
    }
    // Note: No master/slave role can't be selected by looking at the role entry alone.
    //       In a chained replication a slave which also servers as a master for another slave
    //       will be reported as "master" but has the master_host/master_port field set to its
    //       master (see src/server.c in the Redis source code).
    if ((m_redis_role == REDIS_ROLE_MASTER) && (m_redis_master_host != "")) {
        m_redis_role = REDIS_ROLE_CHAINED_REPLICATION_SLAVE;
    }
};

std::string RedisRoleInfo::GetMasterHost(void) {
    return m_redis_master_host;
}

int RedisRoleInfo::GetMasterPort(void) {
    return m_redis_master_port;
}

int RedisRoleInfo::GetMasterLinkStatus(void) {
    return m_redis_master_link_status;
}

int RedisRoleInfo::GetMasterLinkDownSince(void) {
    return m_redis_master_link_down_since;
}

int RedisRoleInfo::GetMasterLastIOAgo(void) {
    return m_redis_master_last_io_seconds_ago;
}

int RedisRoleInfo::GetNumberOfConnectedSlaves(void) {
    return m_redis_connected_slaves;
}

std::vector<RedisSlaveInfo> RedisRoleInfo::GetSlaveInformation(void) {
    std::string::size_type pos;
    std::string::size_type colon_pos;
    std::string slave_info_str;

    if (m_master_slave_info.empty()) {
        for (auto line: m_redis_info) {
            pos = line.find("slave");
            if ((pos != std::string::npos) && (pos == 0)) {
                if ((line.find("state=") != std::string::npos) && (line.find("offset=") != std::string::npos)) {
                    slave_info_str = line;
                    colon_pos = line.find(":");
                    if (colon_pos != std::string::npos) {
                        slave_info_str.erase(0, colon_pos + 1); // add one for colon itself
                        RedisSlaveInfo rsi = m_parse_slave_info(slave_info_str);
                        m_master_slave_info.push_back(rsi);
                    }
                }
            }
        }
    }
    return m_master_slave_info;
};

RedisSlaveInfo RedisRoleInfo::m_parse_slave_info(std::string s) {
    RedisSlaveInfo rsi{ s };
    return rsi;
}

long long RedisRoleInfo::GetMissingData(void) {
    return m_master_repl_offset - m_slave_repl_offset;
}

long long RedisRoleInfo::GetMasterReplicationOffset(void) {
    return m_master_repl_offset;
}

