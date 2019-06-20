#ifndef __CHECK_REDIS_REDIS_INFO_HPP__
#define __CHECK_REDIS_REDIS_INFO_HPP__

#include <string>
#include <vector>

static std::string s_role_str { "role:" };

class RedisRoleInfo {
    public:
        RedisRoleInfo();
        RedisRoleInfo(std::vector<std::string>);
    private:
        int m_redis_role;
        std::string m_redis_master_host;
        int m_redis_master_port;
        int m_redis_master_link_status;
};

#endif /* __CHECK_REDIS_REDIS_INFO_HPP__ */

