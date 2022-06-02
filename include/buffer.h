#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <sys/uio.h>

/*
    +-------------------+------------------+------------------+
    | prependable data  |  readable data   |  writable data   |
    |                   |                  |                  |
    +-------------------+------------------+------------------+
    |                   |                  |                  |
    0      <=      read_index   <=   write_index    <=     size
*/
namespace demo
{
    class Buffer
    {
    public:
        static const size_t PRE_SIZE;  //前面可添加空间大小
        static const size_t INIT_SIZE; //初始化大小
        static const char *CRLF;

    public:
        explicit Buffer(size_t init_size = INIT_SIZE);
        size_t readableSize() const { return write_index - read_index; }
        size_t writeableSize() const { return buf.size() - write_index; }
        size_t prependableSize() const { return read_index; }

    public:
        //开始读的位置
        const char *peek() const { return &*buf.begin() + read_index; }
        //开始写的位置
        const char *writeBegin() const { return &*buf.begin() + write_index; }
        //获取缓冲区中第一个crlf
        const char *findCRLF() const;
        //移除缓冲区中内容
        void retrieve(size_t len);
        void retrieveInt64() { retrieve(sizeof(int64_t)); }
        void retrieveInt32() { retrieve(sizeof(int32_t)); }
        void retrieveInt16() { retrieve(sizeof(int16_t)); }
        void retrieveInt8() { retrieve(sizeof(int8_t)); }
        void retrieveUntil(const char *end);
        void retrieveAll();
        //插入数据
        void append(const char *str, size_t len);
        void append(const std::string &str) { append(str.data(), str.size()); };
        void append(const void *data, size_t len) { append((const char *)data, len); }
        void appendInt64(int64_t val) { append(&val, sizeof(int64_t)); }
        void appendInt32(int32_t val) { append(&val, sizeof(int32_t)); }
        void appendInt16(int16_t val) { append(&val, sizeof(int16_t)); }
        void appendInt8(int8_t val) { append(&val, sizeof(int8_t)); }
        //向前插入数据
        void prepend(const void *data, size_t len);
        void prepend(const char *str, size_t len) { prepend(str, len); }
        void prepend(const std::string &str) { prepend((const char *)str.data(), str.size()); }
        void prependInt64(int64_t data) { prepend(&data, sizeof(int64_t)); }
        void prependInt32(int32_t data) { prepend(&data, sizeof(int32_t)); }
        void prependInt16(int16_t data) { prepend(&data, sizeof(int16_t)); }
        void prependInt8(int8_t data) { prepend(&data, sizeof(int8_t)); }
        //读取数据
        int64_t readInt64();
        int32_t readInt32();
        int16_t readInt16();
        int8_t readInt8();
        //从文件描述符读取数据
        ssize_t readFromFd(int fd);
        //重规划缓冲区
        void shrink()
        {
            buf.shrink_to_fit();
            makeSpace(INIT_SIZE);
        }

        //转换为string
        std::string toString()
        {
            return std::string(peek(), readableSize());
        }

    private:
        //确保缓冲区空间足够
        void makeSpace(size_t len);

    private:
        std::vector<char> buf;
        size_t read_index;
        size_t write_index;
    };
}