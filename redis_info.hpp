#ifndef __CHECK_REDIS_REDIS_INFO_HPP__
#define __CHECK_REDIS_REDIS_INFO_HPP__

#include <string>
#include <vector>

const std::string s_role_str { "role:" };
const std::string s_master_host_str { "master_host:" };
const std::string s_master_port_str { "master_port:" };
const std::string s_master_link_status_str { "master_link_status:" };
const std::string s_master_link_down_since_str { "master_link_down_since_seconds:" };

class RedisRoleInfo {
    public:
        RedisRoleInfo();
        RedisRoleInfo(std::string);
        RedisRoleInfo(std::vector<std::string>);
        int GetRole();
        std::string GetMasterHost();
        int GetMasterPort();
        int GetMasterLinkStatus();
        int GetMasterLinkDownSince();
    private:
        void m_parse_info_lines(std::vector<std::string>);
        int m_redis_role;
        std::string m_redis_master_host;
        int m_redis_master_port;
        int m_redis_master_link_status;
        int m_redis_master_link_down_since;
};

#endif /* __CHECK_REDIS_REDIS_INFO_HPP__ */

