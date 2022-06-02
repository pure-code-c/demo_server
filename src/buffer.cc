#include "../include/buffer.h"

namespace demo
{

    const size_t Buffer::PRE_SIZE = 16;    //前面可添加空间大小
    const size_t Buffer::INIT_SIZE = 1024; //初始化大小
    const char *Buffer::CRLF = "\r\n";

    Buffer::Buffer(size_t init_size)
        : read_index(PRE_SIZE),
          write_index(PRE_SIZE),
          buf(init_size + PRE_SIZE)
    {
        assert(init_size >= 0);
    }

    const char *Buffer::findCRLF() const
    {
        const char *crlf = std::search(peek(), writeBegin(), CRLF, CRLF + 2);
        return crlf == writeBegin() ? nullptr : crlf;
    }

    void Buffer::retrieve(size_t len)
    {
        assert(len <= readableSize());
        if (len <= readableSize())
        {
            read_index += len;
        }
        else
        {
            retrieveAll();
        }
    }

    void Buffer::retrieveUntil(const char *end)
    {
        assert(peek() <= end);
        assert(end <= writeBegin());
        retrieve(end - peek());
    }

    void Buffer::retrieveAll()
    {
        read_index = PRE_SIZE;
        read_index = PRE_SIZE;
    }

    void Buffer::makeSpace(size_t len)
    {
        //需要重新分配空间
        if (writeableSize() < len)
        {
            buf.resize(write_index + len);
        }
        //将readable data前移
        else
        {
            assert(PRE_SIZE < read_index);
            size_t readable = readableSize();
            char *begin = &*buf.begin();
            memcpy(begin + PRE_SIZE, begin + read_index, readable);

            read_index = PRE_SIZE;
            write_index = read_index + readable;
            //移动前后数据必须保持不变
            assert(readable == readableSize());
        }
    }

    void Buffer::prepend(const void *data_, size_t len)
    {
        assert(len <= prependableSize());
        read_index -= len;
        const char *data = static_cast<const char *>(data_);
        std::copy(data, data + len, &*buf.begin() + read_index);
    }

    void Buffer::append(const char *data, size_t len)
    {
        //确保可写空间足够
        if (writeableSize() < len)
        {
            makeSpace(len);
        }
        assert(writeableSize() >= len);

        memcpy((void *)writeBegin(), data, len);
        write_index += len;
    }

    int64_t Buffer::readInt64()
    {
        assert(readableSize() >= sizeof(int64_t));
        int64_t result = *(int64_t *)peek();
        retrieveInt64();
        return result;
    }

    int32_t Buffer::readInt32()
    {
        assert(readableSize() >= sizeof(int32_t));
        int64_t result = *(int32_t *)peek();
        retrieveInt32();
        return result;
    }

    int16_t Buffer::readInt16()
    {
        assert(readableSize() >= sizeof(int16_t));
        int16_t result = *(int16_t *)peek();
        retrieveInt16();
        return result;
    }

    int8_t Buffer::readInt8()
    {
        assert(readableSize() >= sizeof(int8_t));
        int8_t result = *(int8_t *)peek();
        retrieveInt8();
        return result;
    }

    ssize_t Buffer::readFromFd(int fd)
    {
        /*
            通过散布读，将数据写入缓冲区和栈上空间
            确保缓冲区不会溢出
        */
        char extrabuf[65536]; //额外增加64k空间
        struct iovec vec[2];
        const size_t writable = writeableSize();
        vec[0].iov_base = &*buf.begin() + write_index;
        vec[0].iov_len = writable;
        vec[1].iov_base = extrabuf;
        vec[1].iov_len = sizeof extrabuf;
        const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
        const ssize_t len = readv(fd, vec, iovcnt);
        if (len <= writable && len > 0)
        {
            write_index += len;
        }
        else if (len > writable)
        {
            write_index = buf.size();
            append(extrabuf, len - writable);
        }
        return len;
    }

}