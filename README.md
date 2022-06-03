#### 简介
* 进行简单封装后，实现的简单即时通讯服务器
* 实现了注册、登录、公共聊天室、私人连接等功能
* 目前没有实现客户端代码，直接使用netcat进行测试
* 个人学习用途，没有进行性能测试

---

#### 环境

* 开发环境为`Ubuntu 22`

* 需要部署`mysql-server`以及对C提供的API

    ```shell
    #安装mysql
    sudo apt install mysql-server
    
    #配置API
    sudo apt install libmysqlclient-dev
    ```

* 使用`jsoncpp`库用作json解析

    [open-source-parsers/jsoncpp: A C++ library for interacting with JSON. (github.com)](https://github.com/open-source-parsers/jsoncpp)

* 准备数据表

    ```mysql
    CREATE TABLE `user` (
      `id` int NOT NULL AUTO_INCREMENT,
      `email` varchar(255) DEFAULT NULL,
      `username` varchar(255) DEFAULT NULL,
      `passwd` varchar(255) DEFAULT NULL,
      PRIMARY KEY (`id`)
    ) ENGINE=InnoDB AUTO_INCREMENT=1000012 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
    ```

---

#### 目录结构

```
├── char_server					通讯服务器（应该是chat_server，问题不大）
|
├── conf						配置文件
│   └── dbconf.json					数据库相关配置
├── http_server					http服务器（只用来实现了注册功能）
|
├── include						相关头文件
│   ├── buffer.h					应用层缓冲区（参考muduo库）
│   ├── connection.h				封装client socket
│   ├── epoller.h					reactor模型
│   ├── http						http相关解析（参考muduo库）
│   │   ├── httpcontext.h
│   │   ├── http.h
│   │   ├── httprequest.h
│   │   └── httpresponse.h
│   ├── http.h					
│   ├── listener.h					封装client socket
│   ├── mysqlconn.h					封装mysql api
│   ├── mysqlconnpool.h				mysql连接池
│   ├── noncopyable.h			
│   └── threadpool.h				线程池
├── src							实现代码
|
└── test						测试代码
```

