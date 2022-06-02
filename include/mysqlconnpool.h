#include "mysqlconn.h"
#include <queue>
#include <memory>
#include <fstream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <unistd.h>
#include <json/json.h>
#include <chrono>
#include <atomic>

namespace demo
{
    class MysqlConnPool : Noncopyable
    {
    public:
        static MysqlConnPool *getConnPool();
        //获取连接
        std::shared_ptr<MysqlConn> getConn();
        //关闭连接池
        void close();

    private:
        MysqlConnPool();
        ~MysqlConnPool();
        MysqlConnPool(const MysqlConnPool &&) = delete;
        void operator=(const MysqlConnPool &&) = delete;
        //添加连接
        void addConn();
        //解析配置文件
        void parseConfig();
        //生产数据库连接
        void produceConnection();
        //销毁数据库连接
        void recycleConnection();

    public:
        std::atomic_bool is_running = true;
        unsigned short port;
        time_t max_idle_time; //最大空闲
        uint32_t pool_max_size;
        uint32_t pool_min_size;
        string ip;
        string user;
        string passwd;
        string dbname;
        std::mutex cond_mutex;
        std::condition_variable cond;
        std::mutex pool_mutex;
        std::queue<MysqlConn *> pool;
    };
}