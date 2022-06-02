#include "../include/epoller.h"
#include "../include/listener.h"
#include "../include/http/http.h"
#include "../include/mysqlconnpool.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <functional>
#include <sys/stat.h>
#include <fcntl.h>
#include <json/json.h>
#include <atomic>
#include <hash_fun.h>
#include <regex>

/*
    文件
        index.html      主页面
        register.html   注册页面
        notfound.html   404页面

    请求
        validate                                                验证码
        register?{"username":"xxx",
                "email":"xxx","passwd":"xxx",
                "validate":"xxx","xxx":"xxx"}                   申请注册

*/

//随机产生字符串
std::string randStr(int count);
//根据传入的验证码生成base64编码的jpg
std::string genJpgBase64(const std::string &validate);
//验证码
void validateHandler(demo::Connection *conn, demo::http::HttpRequest &request);
//注册
void registerHandler(demo::Connection *conn, demo::http::HttpRequest &request);
//正常文件请求
void normalFileHandler(demo::Connection *conn, const std::string &path);

/*******************************************/
/*******************************************/

//文件后缀对应的content-type
std::map<std::string, std::string> Types = {
    {"html", "text/html"},
    {"css", "text/css"},
    {"js", "text/javascript"}};
//每个会话保存一个验证码（通过cookie判断是否属于同一个会话）
std::map<std::string, std::string> HashCodes;

int main()
{
    demo::Connection::setWriteCb(
        [](demo::Connection *conn)
        {
            demo::http::HttpContext context;
            assert(context.parseRequest(&conn->buf));
            demo::http::HttpRequest request = context.getRequest();
            //验证码
            if (request.getPath() == "/validate")
                validateHandler(conn, request);
            //注册
            else if (request.getPath() == "/register")
                registerHandler(conn, request);
            //其他页面
            else
                normalFileHandler(conn, request.getPath());
        });
    demo::Epoller epoller;
    demo::Listener listen(9354, epoller);
    epoller.loop();

    return 0;
}

std::string randStr(int count)
{
    srand(time(0));
    char validate[count + 1] = {0};
    for (int i = 0; i < count; ++i)
    {
        validate[i] = 'a' + rand() % 26;
    }
    return validate;
}

std::string genJpgBase64(const std::string &validate)
{

    //生成文件
    std::string command = "python3 gen_validate.py ";
    command.append(validate);
    system(command.data());
    //读取内容
    std::ifstream ifs(validate);
    assert(ifs.is_open());
    std::string ret_base64;
    ifs >> ret_base64;
    //删除文件
    remove(validate.data());
    return ret_base64;
}

void validateHandler(demo::Connection *conn, demo::http::HttpRequest &request)
{
    static std::hash<std::string> hash_fun;
    std::string code = randStr(4);
    std::string jpg_base64 = std::move(genJpgBase64(code));
    //构造响应
    demo::http::HttpResponse response(true);
    response.setStatusCode(demo::http::HttpResponse::OK_200);
    response.setStatusMessage("OK");
    response.setContentType("text/plain");
    response.setContentLength(jpg_base64.length());
    //产生有效hashcode
    std::string hash_code;
    do
    {
        hash_code = std::to_string(hash_fun(randStr(rand() % 100 + 100)));

    } while (HashCodes.find(hash_code) != HashCodes.end());
    std::string ret_cookie("cookie_id=");
    ret_cookie.append(hash_code);
    response.addHeader("Set-Cookie", ret_cookie);
    HashCodes[hash_code] = code;
    //发送响应
    std::string res_str = response.toString();
    write(conn->getFd(), res_str.data(), res_str.length());
    write(conn->getFd(), jpg_base64.data(), jpg_base64.length());
}

void normalFileHandler(demo::Connection *conn, const std::string &path)
{
    //处理文件名
    std::string file_name("./root");
    file_name.append(path);
    if (file_name.back() == '/' || file_name.back() == '\\')
    {
        file_name.append("index.html");
    }
    std::ifstream ifs(file_name);
    demo::http::HttpResponse response(true);
    //返回404页面
    if (!ifs.is_open())
    {
        file_name.clear();
        file_name.append("./root/code_404.html");
        ifs.open(file_name, std::ifstream::in);
        response.setStatusCode(demo::http::HttpResponse::NotFound_404);
        response.setStatusMessage("NotFount");
        response.setContentType(Types.at("html"));
        response.setContentLength(std::filesystem::file_size(file_name));
    }
    //正常页面
    else
    {
        //获取文件后缀
        size_t index = file_name.find_last_of('.') + 1;
        std::string suffix = std::move(file_name.substr(index));
        response.setStatusCode(demo::http::HttpResponse::OK_200);
        response.setStatusMessage("OK");
        response.setContentType(Types.at(suffix));
        response.setContentLength(std::filesystem::file_size(file_name));
    }

    //读取文件内容
    uintmax_t file_size = std::filesystem::file_size(file_name);
    char file_data[file_size];
    int fd = open(file_name.data(), O_RDONLY);
    read(fd, file_data, file_size);

    //发送响应
    std::string res_str = response.toString();
    write(conn->getFd(), res_str.data(), res_str.length());
    write(conn->getFd(), file_data, file_size);
}

//替换所有字串
std::string replaceAll(std::string resource_str, std::string sub_str, std::string new_str)
{
    std::string dst_str = resource_str;
    std::string::size_type pos = 0;
    while ((pos = dst_str.find(sub_str)) != std::string::npos) //替换所有指定子串
    {
        dst_str.replace(pos, sub_str.length(), new_str);
    }
    return dst_str;
}

//返回信息
void msgBack(demo::Connection *conn, const std::string &msg)
{
    //构造响应
    demo::http::HttpResponse response(true);
    response.setStatusCode(demo::http::HttpResponse::OK_200);
    response.setStatusMessage("OK");
    response.setContentType("text/plain");
    response.setContentLength(msg.length());
    //发送响应
    std::string res_str = response.toString();
    write(conn->getFd(), res_str.data(), res_str.length());
    write(conn->getFd(), msg.data(), msg.length());
}

void registerHandler(demo::Connection *conn, demo::http::HttpRequest &request)
{
    std::string query = request.getQuery();
    query = std::move(replaceAll(query, "%22", "\""));
    std::string username;
    std::string email;
    std::string passwd;
    std::string validate;
    //解析query
    Json::Reader reader;
    Json::Value value;
    reader.parse(query, value);
    //检测
    const char *keys[4] = {"username", "email", "passwd", "code"};
    for (int i = 0; i < 4; ++i)
    {
        if (!value[keys[i]].isString())
        {
            std::string error_msg("error:");
            msgBack(conn, error_msg + keys[i]);
            return;
        }
    }
    //检测cookie
    std::string cookie = request.getHeader("Cookie");
    std::string hash_code = cookie.substr(cookie.find_first_of("=") + 1);
    if (HashCodes.find(hash_code) == HashCodes.end())
    {
        msgBack(conn, "error:cookie error");
        return;
    }
    username = value["username"].asString();
    email = value["email"].asString();
    passwd = value["passwd"].asString();
    validate = value["code"].asString();
    //检测验证码
    if (validate != HashCodes[hash_code])
    {
        msgBack(conn, "验证码错误");
        return;
    }
    //检测输入是否有效
    static std::regex reg_name("^(\\w){6,20}$");
    static std::regex reg_email("^\\w+([-+.]\\w+)*@\\w+([-.]\\w+)*\\.\\w+([-.]\\w+)*$");
    static std::regex reg_passwd("^[a-zA-Z]\\w{5,15}$");
    if (!std::regex_match(username, reg_name))
    {
        msgBack(conn, "用户名无效");
        return;
    }
    else if (!std::regex_match(email, reg_email))
    {
        msgBack(conn, "邮箱无效");
        return;
    }
    else if (!std::regex_match(passwd, reg_passwd))
    {
        msgBack(conn, "密码无效");
        return;
    }
    auto sql_conn = demo::MysqlConnPool::getConnPool()->getConn();
    //检测是否存在用户
    sql_conn->query("select id from user where email='" + email + "'");
    if (sql_conn->next())
    {
        msgBack(conn, "邮箱已被注册");
        return;
    }
    //成功注册
    std::string sql(
        "insert into user (email,username,passwd) values ('" +
        email + "','" +
        username + "','" +
        passwd + "')");
    sql_conn->update(sql);
    //返回账号
    sql_conn->query("select id from user where email='" + email + "'");
    sql_conn->next();
    std::string account = sql_conn->value(0);
    msgBack(conn, "注册成功，账号:" + account);
}