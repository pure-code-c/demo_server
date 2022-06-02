#include "../include/mysqlconn.h"

namespace demo
{

    MysqlConn::MysqlConn()
    {
        sql_conn = mysql_init(nullptr);
        flushTimeStamp();
        mysql_set_character_set(sql_conn, "utf8");
    }

    MysqlConn::~MysqlConn()
    {
        if (sql_conn)
            mysql_close(sql_conn);
        freeResult();
    }

    bool MysqlConn::connect(string ip, string user, string passwd, string dbname, int port)
    {
        sql_conn = mysql_real_connect(sql_conn, ip.c_str(), user.c_str(), passwd.c_str(), dbname.c_str(), port, nullptr, 0);
        return (bool)sql_conn;
    }

    void MysqlConn::update(string sql)
    {
        int ret = mysql_query(sql_conn, sql.c_str());
        assert(ret == 0);
    }

    void MysqlConn::query(string sql)
    {
        freeResult();
        update(sql);
        sql_result = mysql_store_result(sql_conn);
        assert(sql_result);
    }

    bool MysqlConn::next()
    {
        if (sql_result == nullptr)
            return false;
        sql_row = mysql_fetch_row(sql_result);
        return (bool)sql_row;
    }

    string MysqlConn::value(int index)
    {
        int row_count = mysql_num_fields(sql_result);
        assert(index < row_count && index >= 0);
        unsigned long len = mysql_fetch_lengths(sql_result)[index]; //值长度
        return string(sql_row[index], len);
    }

    void MysqlConn::freeResult()
    {
        if (sql_result)
            mysql_free_result(sql_result);
        sql_result = nullptr;
        sql_row = nullptr;
    }
}