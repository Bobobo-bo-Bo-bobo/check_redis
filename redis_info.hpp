#ifndef __CHECK_REDIS_REDIS_INFO_HPP__
#define __CHECK_REDIS_REDIS_INFO_HPP__

#include "redis_slave_info.hpp"

#include <string>
#include <vector>

const std::string s_role_str{ "role:" };
const std::string s_master_host_str{ "master_host:" };
const std::string s_master_port_str{ "master_port:" };
const std::string s_master_link_status_str{ "master_link_status:" };
const std::string s_master_link_down_since_str{ "master_link_down_since_seconds:" };
const std::string s_master_last_io_seconds_ago_str{ "master_last_io_seconds_ago:" };
const std::string s_connected_slaves_str{ "connected_slaves:" };
const std::string s_master_repl_offset_str{ "master_repl_offset:" };
const std::string s_slave_repl_offset_str{ "slave_repl_offset:" };

class RedisRoleInfo {
    public:
        RedisRoleInfo();
        RedisRoleInfo(std::string);
        RedisRoleInfo(std::vector<std::string>);
        int GetRole(void);
        std::string GetMasterHost(void);
        int GetMasterPort(void);
        int GetMasterLinkStatus(void);
        int GetMasterLinkDownSince(void);
        int GetMasterLastIOAgo(void);
        int GetNumberOfConnectedSlaves(void);
        std::vector<RedisSlaveInfo> GetSlaveInformation(void);
        long long GetMissingData(void); // only valid if running as slave
        long long GetMasterReplicationOffset(void);

    private:
        void m_parse_info_lines(std::vector<std::string>);
        RedisSlaveInfo m_parse_slave_info(std::string);

        std::vector<std::string> m_redis_info;
        int m_redis_role;
        std::string m_redis_master_host;
        int m_redis_master_port;
        int m_redis_master_link_status;
        int m_redis_master_link_down_since;
        int m_redis_master_last_io_seconds_ago;
        int m_redis_connected_slaves;
        long long m_master_repl_offset;
        std::vector<RedisSlaveInfo> m_master_slave_info;
        long long m_slave_repl_offset;
};

#endif /* __CHECK_REDIS_REDIS_INFO_HPP__ */

