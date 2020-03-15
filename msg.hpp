#ifndef IRC_MSG_HPP
#define IRC_MSG_HPP
#include "head.hpp"

struct User{
    std::string nick;
    std::string note;
    std::string hostname;
    std::string  mode; //模式
    char state;   //状态标识
    char logonup; //注册标识
    std::string away;
    int connfd;
};
struct Channel{
    std::string channel_name;
    std::string root_name;
    std::string theme;
    std::vector<User>channel_user_names;
    std::string mode;
    std::vector<std::string>o_users;
    std::vector<std::string>v_users;
};
std::list<int>clientfd;
std::vector<User>users;
std::vector<Channel>channels;


#endif // !IRC_MSG_H

