#include "check_redis_config.h"
#include "redis_slave_info.hpp"
#include "util.hpp"

#include <string>

RedisSlaveInfo::RedisSlaveInfo() {
    m_port = -1;
    m_state = REDIS_SLAVE_STATE_UNKNOWN;
    m_offset = -1;
    m_lag = -1;
}

RedisSlaveInfo::RedisSlaveInfo(std::string s) {
    m_port = -1;
    m_state = REDIS_SLAVE_STATE_UNKNOWN;
    m_offset = -1;
    m_lag = -1;

    // split string in key=value pairs separated by ","
    std::vector<std::string> kv = split_string(s, ",");

    for (auto kvpair: kv) {
        std::vector<std::string> _kv = split_string(kvpair, "=");
        if (_kv[0] == "ip") {
            m_ip = _kv[1];
        } else if (_kv[0] == "port") {
            m_port = std::stoi(_kv[1], nullptr);
        } else if (_kv[0] == "state") {
            if (_kv[1] == "online") {
                m_state = REDIS_SLAVE_STATE_ONLINE;
            } else if (_kv[1] == "send_bulk"){
                m_state = REDIS_SLAVE_STATE_SEND_BULK;
            } else if (_kv[1] == "wait_bgsave") {
                m_state = REDIS_SLAVE_STATE_WAIT_BGSAVE;
            }
        } else if (_kv[0] == "offset") {
            m_offset = std::stoll(_kv[1], nullptr);
        } else if (_kv[0] == "lag") {
            m_lag = std::stoll(_kv[1], nullptr);
        }
    }
}

std::string RedisSlaveInfo::GetIP(void) {
    return m_ip;
}

int RedisSlaveInfo::GetPort(void) {
    return m_port;
}

int RedisSlaveInfo::GetState(void) {
    return m_state;
}

long long RedisSlaveInfo::GetOffset(void) {
    return m_offset;
}

long long RedisSlaveInfo::GetLag(void) {
    return m_lag;
}

