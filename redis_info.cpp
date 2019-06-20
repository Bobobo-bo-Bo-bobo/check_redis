#include "check_redis_config.h"
#include "redis_info.hpp"
#include "util.hpp"

#include <string>
#include <vector>

RedisRoleInfo::RedisRoleInfo(): m_redis_role{ REDIS_ROLE_UNKNOWN }, m_redis_master_host{ "" }, m_redis_master_port{ 0 },
    m_redis_master_link_status{ REDIS_MASTER_LINK_STATUS_UNKNOWN };

RedisRoleInfo::RedisRoleInfo(std::vector<std::string> s) {
    std::vector<std::string> splitted = split_lines(s);
    for (auto line: splitted) {
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
    }
}

