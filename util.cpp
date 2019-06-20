#include "check_redis_config.h"
#include "util.hpp"

std::vector<std::string> split_lines(std::string str) {
    std::string linebreaks = "\r\n";
    std::string::size_type sub_str_position = 0;
    std::string str_tok;
    std::vector<std::string> result;
    
    while ((sub_str_position = str.find(linebreaks)) != std::string::npos) {
        str_tok = str.substr(0, sub_str_position);
        result.push_back(str_tok);
        str.erase(0, sub_str_position + linebreaks.length());
    }
    result.push_back(str);
    return result;
}

std::vector<std::string> split_string(std::string str, const std::string delimiter) {
    std::string::size_type sub_str_position = 0;
    std::string str_tok;
    std::vector<std::string> result;
    
    while ((sub_str_position = str.find(delimiter)) != std::string::npos) {
        str_tok = str.substr(0, sub_str_position);
        result.push_back(str_tok);
        str.erase(0, sub_str_position + delimiter.length());
    }
    result.push_back(str);
    return result;
}
