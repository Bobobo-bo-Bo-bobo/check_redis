#ifndef __CHECK_REDIS_REDIS_SLAVE_INFO_HPP__
#define __CHECK_REDIS_REDIS_SLAVE_INFO_HPP__

#include <string>

class RedisSlaveInfo {
    public:
        RedisSlaveInfo(void);
        RedisSlaveInfo(std::string);
        std::string GetIP(void);
        int GetPort(void);
        int GetState(void);
        long long GetOffset(void);
        long long GetLag(void);

    private:
        std::string m_ip;
        int m_port;
        int m_state;
        long long m_offset;
        long long m_lag;
};

#endif /* __CHECK_REDIS_REDIS_SLAVE_INFO_HPP__ */

