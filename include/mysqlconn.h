#pragma once

#include <mysql/mysql.h>
#include <string>
#include <assert.h>
#include "noncopyable.h"

/*
    数据库结构

    filed_1 | filed_2 | ... | filed_n
    --------|---------|-----|--------
    value1_1| value1_2| ... | value1_n
    --------|---------|-----|--------
    value2_1| value2_2| ... | value2_n
*/

namespace demo
{
    using std::string;
    class MysqlConn : Noncopyable
    {
    public:
        MysqlConn();
        ~MysqlConn();
        //连接数据库
        bool connect(string ip, string user, string passwd, string dbname, int port = 3306);
        //更新数据库
        void update(string sql);
        //查询数据库
        void query(string sql);
        //进入下一行
        bool next();
        //获取字段值
        string value(int index);
        //刷新时间戳
        void flushTimeStamp() { time_stamp = time(nullptr); }
        //存活总时长
        time_t aliveTime() { return time(nullptr) - time_stamp; }
        //释放结果集
        void freeResult();

    private:
        time_t time_stamp;
        MYSQL *sql_conn = nullptr;
        MYSQL_RES *sql_result = nullptr;
        MYSQL_ROW sql_row = nullptr;
    };
}