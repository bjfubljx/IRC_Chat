#ifndef IRC_COMMAND_H
#define IRC_COMMAND_H

#include <iostream>
#include <string>

#define SERVER  "server"    /* /server  [网络]              连接网络*/
#define NICK    "nick"      /* /nick    [昵称]              将自己的名字改为xxx */
#define WHOIS   "whois"     /* /whois   [昵称]              获取某人的一些信息，如对方的真实名字，对方的IP，对方是何时登录IRC服务器的，等等  */
#define LIST    "list"      /* /list                        列出当前服务器所有聊天室名称*/
#define NAMES   "names"     /* /names   [#聊天室名称]       列出当前聊天室所有昵称*/
#define MSG     "msg"       /* /msg     [别名|#聊天室名称]  [消息内容]   向某人发出密聊话语，或者对某个闲聊室的所有人说话 */

#define QUERY   "query"     /* /query   [昵称] [消息内容]   私信某人*/
#define JOIN    "join"      /* /join    [#聊天室名称]       加入某个聊天室*/
#define PART    "part"      /* /part    [#聊天室名称]       退出某个聊天室*/
#define KICK    "kick"      /* /kick    [#聊天室名称] [昵称] [原因] 把某人从指定的闲聊室赶出去，管理员权限*/
#define QUIT    "quit"      /* /quit    离开IRC网络 */

#endif