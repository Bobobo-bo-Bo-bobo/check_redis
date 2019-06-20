#include "check_redis_config.h"
#include "redis_info.hpp"
#include "util.hpp"

#include <string>
#include <vector>

RedisRoleInfo::RedisRoleInfo(): m_redis_role{ REDIS_ROLE_UNKNOWN }, m_redis_master_host{ "" }, m_redis_master_port{ 0 },
    m_redis_master_link_status{ REDIS_MASTER_LINK_STATUS_UNKNOWN } {};

RedisRoleInfo::RedisRoleInfo(std::vector<std::string> splitted): m_redis_master_host{ "" }, m_redis_master_port{ 0 } {
    m_parse_info_lines(splitted);
}

RedisRoleInfo::RedisRoleInfo(std::string s): m_redis_master_host{ "" }, m_redis_master_port{ 0 } {
    std::vector<std::string> splitted = split_lines(s);
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

