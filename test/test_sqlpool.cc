#include "../include/mysqlconnpool.h"
#include "../include/threadpool.h"
#include <iostream>
#include <chrono>

//随机字符串
std::string randStr()
{
    int len = rand() % 50 + 10;
    std::string str;
    for (int i = 0; i < len; i++)
    {
        str.push_back('a' + rand() % 26);
    }
    return str;
}

//清空数据表
void clearSqlTable()
{
    demo::MysqlConn conn;
    conn.connect("localhost", "root", "root", "db_test");
    conn.update("delete from user");
}

//单线程，不使用连接池
void test_1(int count)
{
    clearSqlTable();
    auto begin = std::chrono::steady_clock::now();
    for (int i = 0; i < count; ++i)
    {
        demo::MysqlConn conn;
        conn.connect("localhost", "root", "root", "db_test");
        char sql[1024];
        sprintf(sql, "insert into user (name) values('%s')", randStr().c_str());
        conn.update(sql);
    }
    auto end = std::chrono::steady_clock::now();
    std::cout << "test_1 spend " << (end - begin).count() << " ns" << std::endl;
}

//单线程，使用连接池
void test_2(int count)
{
    clearSqlTable();
    demo::MysqlConnPool *pool = demo::MysqlConnPool::getConnPool();
    auto begin = std::chrono::steady_clock::now();
    for (int i = 0; i < count; ++i)
    {
        std::shared_ptr<demo::MysqlConn> conn = pool->getConn();
        char sql[1024];
        sprintf(sql, "insert into user (name) values('%s')", randStr().c_str());
        conn->update(sql);
    }
    auto end = std::chrono::steady_clock::now();
    std::cout << "test_2 spend " << (end - begin).count() << " ns" << std::endl;
}

//多线程，不使用用连接池
void test_3(int count)
{
    clearSqlTable();
    demo::ThreadPool pool(10);
    pool.start();
    auto begin = std::chrono::steady_clock::now();
    for (int i = 0; i < count; ++i)
    {
        pool.addTask(
            []()
            {
                demo::MysqlConn conn;
                conn.connect("localhost", "root", "root", "db_test");
                char sql[1024];
                sprintf(sql, "insert into user (name) values('%s')", randStr().c_str());
                conn.update(sql);
            });
    }
    pool.stop();
    auto end = std::chrono::steady_clock::now();
    std::cout << "test_2 spend " << (end - begin).count() << " ns" << std::endl;
}

//多线程，使用连接池
void test_4(int count)
{
    clearSqlTable();
    demo::MysqlConnPool *sql_pool = demo::MysqlConnPool::getConnPool();
    demo::ThreadPool thread_pool(10);
    thread_pool.start();
    auto begin = std::chrono::steady_clock::now();
    for (int i = 0; i < count; ++i)
    {
        thread_pool.addTask(
            [&sql_pool]()
            {
                std::shared_ptr<demo::MysqlConn> conn = sql_pool->getConn();
                char sql[1024];
                sprintf(sql, "insert into user (name) values('%s')", randStr().c_str());
                conn->update(sql);
            });
    }
    thread_pool.stop();
    auto end = std::chrono::steady_clock::now();
    std::cout << "test_2 spend " << (end - begin).count() << " ns" << std::endl;
}

int main()
{
    srand(time(nullptr));

    test_1(50000); // 45s左右
    test_2(50000); // 32s左右
    test_3(50000); // 10s左右
    test_4(50000); // 6s左右

    return 0;
}