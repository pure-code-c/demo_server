#include "../include/mysqlconnpool.h"
#include <iostream>

namespace demo
{

    MysqlConnPool::MysqlConnPool()
    {
        parseConfig();
        for (int i = 0; i < pool_min_size; ++i)
        {
            addConn();
        }
        std::thread producer(&MysqlConnPool::produceConnection, this);
        std::thread recycler(&MysqlConnPool::recycleConnection, this);
        producer.detach();
        recycler.detach();
    }

    MysqlConnPool::~MysqlConnPool()
    {
        while (!pool.empty())
        {
            auto conn = pool.front();
            pool.pop();
            delete conn;
        }
    }

    void MysqlConnPool::produceConnection()
    {
        while (is_running)
        {
            std::unique_lock<std::mutex> lock(cond_mutex);
            while (pool.size() >= pool_min_size)
            {
                cond.wait(lock);
            }
            addConn();
            cond.notify_all();
        }
    }

    void MysqlConnPool::recycleConnection()
    {
        while (is_running)
        {
            while (pool.size() >= pool_max_size)
            {
                std::lock_guard<std::mutex> lock(pool_mutex);
                MysqlConn *conn = pool.front();
                if (conn->aliveTime() >= max_idle_time)
                {
                    pool.pop();
                    delete conn;
                }
                break;
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    std::shared_ptr<MysqlConn> MysqlConnPool::getConn()
    {
        std::unique_lock<std::mutex> lock(cond_mutex);
        while (pool.empty())
        {
            cond.wait(lock);
        }
        //外界不使用连接后，连接自动返回到连接池中
        std::shared_ptr<MysqlConn> conn(pool.front(),
                                        [this](MysqlConn *conn)
                                        {
                                            std::lock_guard<std::mutex> lock(this->pool_mutex);
                                            this->pool.push(conn);
                                            conn->flushTimeStamp();
                                            conn->freeResult();
                                        });
        pool.pop();
        cond.notify_one();
        return conn;
    }

    void MysqlConnPool::addConn()
    {
        MysqlConn *con = new MysqlConn;
        if (con->connect(ip, user, passwd, dbname, port))
        {
            con->flushTimeStamp();
            pool.push(con);
        }
    }

    MysqlConnPool *MysqlConnPool::getConnPool()
    {
        static MysqlConnPool *pool = new MysqlConnPool;
        return pool;
    }

    void MysqlConnPool::close()
    {
        is_running = false;
        cond.notify_all();
    }

    void MysqlConnPool::parseConfig()
    {
        std::fstream ifs("../conf/dbconf.json");
        assert(ifs);
        Json::Reader rd;
        Json::Value value;
        rd.parse(ifs, value);
        assert(value.isObject());

        port = value["port"].asUInt();
        max_idle_time = value["max_idle_time"].asInt();
        pool_max_size = value["pool_max_size"].asUInt();
        pool_min_size = value["pool_min_size"].asUInt();
        ip = value["ip"].asString();
        user = value["user"].asString();
        passwd = value["passwd"].asString();
        dbname = value["dbname"].asString();
    }
}