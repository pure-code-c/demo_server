#include "../include/listener.h"
#include "../include/mysqlconnpool.h"
#include "../include/epoller.h"
#include <json/json.h>
#include <regex>

#include <string>
#include <map>
#include <mutex>
#include <fstream>
#include <set>

/*
结构
    node1   node3
     |  \   /    \
     |  node2 —— node4
     |
    node5       node6
*/

//账户正则
const std::regex reg_account("^[0-9]\\d*$");
//密码正则
const std::regex reg_passwd("^[a-zA-Z]\\w{5,15}$");
//用户节点
struct AccountNode : std::enable_shared_from_this<AccountNode>
{
    AccountNode() = default;
    AccountNode(const std::string &account_, const std::string &username_, demo::Connection *conn_)
        : account(account_), username(username_), conn(conn_) {}
    std::string account;                  //自身
    std::string username;                 //用户名
    demo::Connection *conn;               //连接
    std::set<AccountNode *> linked_nodes; //建立了连接的其他节点
};
std::map<std::string, std::shared_ptr<AccountNode>> Login_Account_Nodes;
std::mutex mutex;

/*******************************************************************************/
/*******************************************************************************/

//解析Buff中的json
std::string parseJsonFromBuffer(demo::Buffer &buffer);
//判断用户是否存在
bool accountValid(const std::string &account);
//判断连接是否登录
bool connLogined(demo::Connection *conn);
//获取account
std::string getAccount(demo::Connection *conn);
//检测用户是否有效
bool accountValid(const std::string &account);
//检测密码是否有效
bool passwdValid(const std::string &account, const std::string &passwd);
//获取用户名
std::string getUsername(const std::string &account);

/**********************************************************************************/
/**********************************************************************************/

//登录请求
void loginHandle(demo::Connection *conn, Json::Value &json_value);
//登录消息响应
void loginResponse(demo::Connection *conn, std::string status, std::string msg);
//连接和某个client建立连接
void linkRequestHandler(demo::Connection *conn, Json::Value &json_value);
//连接响应
void linkResponse(demo::Connection *conn, const std::string &status, const std::string &msg);
//连接请求
void linkRequest(demo::Connection *dest_conn, const std::string &account);
//客户端对连接的响应
void linkResponseHandler(demo::Connection *conn, Json::Value &json_value);
//断开与某个client的连接
void dislinkRequestHandler(demo::Connection *conn, Json::Value &json_value);
//请求建立了连接的所有client
void requestForLinkedHandler(demo::Connection *conn, Json::Value &json_value);
//断开与某个client的连接
void requestDislinkHandler(demo::Connection *conn, const std::string &account);
//获取所有在线用户
void requestAllAccountHandler(demo::Connection *conn);
//公共聊天室消息
void sendMsgToPublicHandler(demo::Connection *conn, Json::Value &json_value);
//个人消息
void senMsgToLinkedHandler(demo::Connection *conn, Json::Value &json_value);

int main()
{
    demo::Connection::setCloseCb(
        [](demo::Connection *conn) -> void
        {
            demo::Connection::defaultCloseCb(conn);
            if (!connLogined(conn))
                return;
            //删除建立连接的所有节点
            auto this_node = Login_Account_Nodes[getAccount(conn)];
            for (auto &linked_node : this_node->linked_nodes)
            {
                //向所有连接节点发送消息
                requestDislinkHandler(linked_node->conn, this_node->account);
                //删除节点
                mutex.lock();
                linked_node->linked_nodes.erase(this_node.get());
                mutex.unlock();
            }
            //删除自身节点
            mutex.lock();
            Login_Account_Nodes.erase(this_node->account);
            mutex.unlock();
        });

    demo::Connection::setWriteCb(
        [](demo::Connection *conn) -> void
        {
            //解析json
            std::string json_str = parseJsonFromBuffer(conn->buf);
            Json::Value json_value;
            static Json::Reader reader;
            reader.parse(json_str, json_value);
            std::string type = json_value["type"].asString();
            //登录请求
            if (type == "login_request")
                loginHandle(conn, json_value);
            //连接请求
            else if (type == "client_link_request")
                linkRequestHandler(conn, json_value);
            //断开请求
            else if (type == "client_disconn_request")
                dislinkRequestHandler(conn, json_value);
            //连接响应
            else if (type == "client_link_response")
                linkResponseHandler(conn, json_value);
            //获取所有好友
            else if (type == "request_conn_friends")
                requestForLinkedHandler(conn, json_value);
            //获取所有在线用户
            else if (type == "request_all_conn")
                requestAllAccountHandler(conn);
            //公共聊天室
            else if (type == "send_to_public")
                sendMsgToPublicHandler(conn, json_value);
            //个人消息
            else if (type == "message_request")
                senMsgToLinkedHandler(conn, json_value);
            //关闭读取回调
            conn->event.event.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
            conn->epoller.modEvent(conn->getFd(), &conn->event);
        });

    demo::Epoller epoller(20);
    demo::Listener listener(9355, epoller);
    epoller.loop();

    return 0;
}

std::string parseJsonFromBuffer(demo::Buffer &buffer)
{
    char *begin = (char *)buffer.peek();
    auto read_able = buffer.readableSize();
    //找到第一个'{'
    while (read_able > 0)
    {
        if (*begin == '{')
            break;
        ++begin;
        --read_able;
    }
    //解析最大的{}
    char *end = begin;
    char stack[128];
    int stack_top = 0;
    for (int i = 0; i < read_able; ++i)
    {
        if (*end == '{')
            stack[stack_top++] = '{';
        else if (*end == '}' && stack[stack_top - 1] == '{')
            --stack_top;
        ++end;
    }
    if (stack_top > 0)
    {
        buffer.retrieveAll();
        return "";
    }
    buffer.retrieveUntil(end);
    return std::string(begin, end);
}

bool connLogined(demo::Connection *conn)
{
    if (getAccount(conn).empty())
    {
        shutdown(conn->getFd(), SHUT_RDWR);
        return false;
    }
    return true;
}

std::string getAccount(demo::Connection *conn)
{
    for (auto &node : Login_Account_Nodes)
    {
        if (node.second->conn == conn)
            return node.first;
    }
    return "";
}

bool accountValid(const std::string &account)
{
    std::string sql = "select id from user where id = " + account;
    auto sql_conn = demo::MysqlConnPool::getConnPool()->getConn();
    sql_conn->query(sql);
    return sql_conn->next();
}

bool passwdValid(const std::string &account, const std::string &passwd)
{
    auto sql_conn = demo::MysqlConnPool::getConnPool()->getConn();
    std::string sql = "select passwd from user where id = " + account;
    sql_conn->query(sql);
    sql_conn->next();
    return sql_conn->value(0) == passwd;
}

std::string getUsername(const std::string &account)
{
    auto sql_conn = demo::MysqlConnPool::getConnPool()->getConn();
    std::string sql = "select username from user where id = " + account;
    sql_conn->query(sql);
    sql_conn->next();
    return sql_conn->value(0);
}

void loginResponse(demo::Connection *conn, std::string status, std::string msg)
{
    Json::FastWriter writer;
    Json::Value value;
    value["type"] = "login_response";
    value["data"]["status"] = status;
    value["data"]["msg"] = msg;
    std::string json_str = writer.write(value);
    write(conn->getFd(), json_str.data(), json_str.length());
}

void loginHandle(demo::Connection *conn, Json::Value &json_value)
{
    std::string account = json_value["data"]["account"].asString();
    std::string passwd = json_value["data"]["passwd"].asString();
    //验证账户
    if (!std::regex_match(account, reg_account) || !accountValid(account))
    {
        loginResponse(conn, "ERROR", "account_not_exist");
        return;
    }
    //验证密码
    if (!std::regex_match(passwd, reg_passwd) || !passwdValid(account, passwd))
    {
        loginResponse(conn, "ERROR", "passwd_error");
        return;
    }
    //检验是否已登录
    if (Login_Account_Nodes.find(account) != Login_Account_Nodes.end())
    {
        loginResponse(conn, "ERROR", "account_logined");
        return;
    }
    //登录成功
    std::string username = getUsername(account);
    loginResponse(conn, "OK", username);
    mutex.lock();
    Login_Account_Nodes[account] = std::make_shared<AccountNode>(account, username, conn);
    mutex.unlock();
}

void linkResponse(demo::Connection *conn, const std::string &status, const std::string &msg)
{
    Json::FastWriter writer;
    Json::Value value;
    value["type"] = "server_link_response";
    value["data"]["status"] = status;
    value["data"]["msg"] = msg;
    std::string json_str = writer.write(value);
    write(conn->getFd(), json_str.data(), json_str.length());
}

void linkRequest(demo::Connection *dest_conn, const std::string &account)
{
    Json::FastWriter writer;
    Json::Value value;
    value["type"] = "server_link_request";
    value["data"]["account"] = account;
    value["data"]["username"] = Login_Account_Nodes[account]->username;
    std::string json_str = writer.write(value);
    write(dest_conn->getFd(), json_str.data(), json_str.length());
}

void linkRequestHandler(demo::Connection *conn, Json::Value &json_value)
{
    if (!connLogined(conn))
        return;

    std::string account = json_value["data"]["account"].asString();
    auto dest_node = Login_Account_Nodes.find(account);
    //对方未在线
    if (dest_node == Login_Account_Nodes.end())
    {
        linkResponse(conn, "ERROR", "not_logined");
        return;
    }
    //不能和自己连接
    if (dest_node->second->conn == conn)
    {
        linkResponse(conn, "ERROR", "no_link_self");
        return;
    }
    //账号不存在
    if (!std::regex_match(account, reg_account) || !accountValid(account))
    {
        linkResponse(conn, "ERROR", "account_not_exist");
        return;
    }
    //发送连接请求
    linkRequest(dest_node->second->conn, getAccount(conn));
    //发送连接响应
    linkResponse(conn, "WAIT", "wair_for_link");
}

//断开连接响应
void disconnResponse(demo::Connection *conn, const std::string &status, const std::string &account)
{
    static Json::FastWriter writer;
    Json::Value json;
    json["type"] = "server_disconn_response";
    json["data"]["account"] = account;
    json["data"]["status"] = status;
    std::string json_str = writer.write(json);
    write(conn->getFd(), json_str.data(), json_str.length());
}

void dislinkRequestHandler(demo::Connection *conn, Json::Value &json_value)
{
    if (!connLogined(conn))
        return;
    std::string account = json_value["data"]["account"].asString();
    auto this_node = Login_Account_Nodes[getAccount(conn)];
    for (auto &friend_node : this_node->linked_nodes)
    {
        if (friend_node->account == account)
        {
            disconnResponse(conn, "OK", account);
            requestDislinkHandler(friend_node->conn, this_node->account);
            mutex.lock();
            friend_node->linked_nodes.erase(this_node.get());
            this_node->linked_nodes.erase(friend_node);
            mutex.unlock();
            return;
        }
    }
    disconnResponse(conn, "ERROR", account);
}

void linkResponseHandler(demo::Connection *conn, Json::Value &json_value)
{
    std::string status = json_value["data"]["status"].asString();
    std::string account = json_value["data"]["account"].asString();
    demo::Connection *dest_conn = Login_Account_Nodes[account]->conn;
    //拒绝连接
    if (status != "OK")
    {
        linkResponse(dest_conn, "NO", account);
        return;
    }
    if (account == getAccount(conn))
        return;
    //同意连接
    linkResponse(dest_conn, "ok", account);
    std::string this_account = getAccount(conn);
    mutex.lock();
    Login_Account_Nodes[account]->linked_nodes.insert(Login_Account_Nodes[this_account].get());
    Login_Account_Nodes[this_account]->linked_nodes.insert(Login_Account_Nodes[account].get());
    mutex.unlock();
}

void requestForLinkedHandler(demo::Connection *conn, Json::Value &json_value)
{
    if (!connLogined(conn))
        return;

    std::string account = getAccount(conn);
    std::string friend_account;
    std::string friend_username;
    //获取所有建立连接的节点
    auto this_node = Login_Account_Nodes[account];
    auto end = this_node->linked_nodes.end();
    for (auto begin = this_node->linked_nodes.begin(); begin != end; ++begin)
    {
        friend_account.append((*begin)->account + ";");
        friend_username.append((*begin)->username + ";");
    }
    //
    static Json::FastWriter writer;
    Json::Value json;
    json["type"] = "response_conn_friends";
    json["data"]["account"] = friend_account;
    json["data"]["username"] = friend_username;
    std::string json_str = writer.write(json);
    //
    write(conn->getFd(), json_str.data(), json_str.length());
}

void requestDislinkHandler(demo::Connection *conn, const std::string &account)
{
    static Json::FastWriter writer;
    Json::Value json;
    json["type"] = "friend_disconn";
    json["data"]["account"] = account;
    json["data"]["username"] = Login_Account_Nodes[account]->username;
    std::string json_str = writer.write(json);
    write(conn->getFd(), json_str.data(), json_str.length());
}

void requestAllAccountHandler(demo::Connection *conn)
{
    if (!connLogined(conn))
        return;

    std::string conn_accounts;
    std::string conn_usernames;
    for (auto account : Login_Account_Nodes)
    {
        conn_accounts.append(account.second->account + ";");
        conn_usernames.append(account.second->username + ";");
    }
    static Json::FastWriter writer;
    Json::Value json;
    json["type"] = "response_all_conn";
    json["data"]["account"] = conn_accounts;
    json["data"]["username"] = conn_usernames;
    std::string json_str = writer.write(json);
    write(conn->getFd(), json_str.data(), json_str.length());
}

void sendMsgToPublicHandler(demo::Connection *conn, Json::Value &json_value)
{
    if (!connLogined(conn))
        return;

    auto node = Login_Account_Nodes[getAccount(conn)];
    json_value["data"]["account"] = node->account;
    json_value["data"]["username"] = node->username;
    static Json::FastWriter writer;
    std::string json_str = writer.write(json_value);
    for (auto &account : Login_Account_Nodes)
    {
        if (account.second->conn != conn)
            write(account.second->conn->getFd(), json_str.data(), json_str.length());
    }
}

//消息响应
void messageResponse(demo::Connection *conn, const std::string &status, const std::string &account)
{
    static Json::FastWriter writer;
    Json::Value value;
    value["type"] = "message_response";
    value["data"]["status"] = status;
    value["data"]["account"] = account;
    std::string json_str = writer.write(value);
    write(conn->getFd(), json_str.data(), json_str.length());
}

//消息发送
void messageSend(demo::Connection *conn, const std::string &account, const std::string &msg)
{
    static Json::FastWriter writer;
    Json::Value value;
    value["type"] = "message_send";
    value["data"]["msg"] = msg;
    value["data"]["account"] = account;
    value["data"]["username"] = Login_Account_Nodes[account]->username;
    std::string json_str = writer.write(value);
    write(conn->getFd(), json_str.data(), json_str.length());
}

void senMsgToLinkedHandler(demo::Connection *conn, Json::Value &json_value)
{
    if (!connLogined(conn))
        return;
    std::string account = json_value["data"]["account"].asString();
    auto this_node = Login_Account_Nodes[getAccount(conn)];
    for (auto &friend_node : this_node->linked_nodes)
    {
        if (friend_node->account == account)
        {
            messageResponse(conn, "OK", account);
            messageSend(Login_Account_Nodes[account]->conn, account, json_value["data"]["msg"].asString());
            return;
        }
    }
    messageResponse(conn, "ERROR", account);
    return;
}