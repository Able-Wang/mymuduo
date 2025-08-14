#pragma once

#include <vector>
#include <unistd.h>
#include <string>
#include <algorithm>

// 网络库底层的缓冲区类型定义
class Buffer
{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize), readerIndex_(kCheapPrepend), writerIndex_(kCheapPrepend)
    {
    }

    size_t readableBytes() const
    {
        return writerIndex_ - readerIndex_;
    }

    size_t writeableBytes() const
    {
        return buffer_.size() - writerIndex_;
    }

    size_t preppeandBytes() const
    {
        return readerIndex_;
    }

    // 返回缓冲区可读数据起始地址
    const char *peek() const
    {
        return begin() + readerIndex_;
    }

    // onMessage string<-Buffer
    void retrieve(size_t len)
    {
        if (len < readableBytes())
        {
            readerIndex_ += len; // 应用只读取len长度数据
        }
        else // len==readableBytes()
        {
            retrieveAll();
        }
    }

    void retrieveAll()
    {
        readerIndex_ = writerIndex_ = kCheapPrepend;
    }

    // 把onMessag上报buffer数据转成string类型返回
    std::string retrieveAllAsString()
    {
        return retrieveAsString(readableBytes()); // 应用可读数据长度
    }

    std::string retrieveAsString(size_t len)
    {
        std::string result(peek(), len);
        retrieve(len); // 对缓冲区进行复位操作
        return result;
    }

    // 确保可写缓冲区长度
    void ensureWriteableBytes(size_t len)
    {
        if (writeableBytes() < len)
        {
            makespace(len);
        }
    }

    void makespace(size_t len)
    {
        //
        if (writeableBytes() + preppeandBytes() < len + kCheapPrepend)
        {
            buffer_.resize(writerIndex_ + len);
        }
        else // 前面有空闲缓冲区
        {
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_,
                      begin() + writerIndex_,
                      begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
    }

    // 把{data data+len}内存上数据，添加到write缓冲区
    void append(const char *data, size_t len)
    {
        ensureWriteableBytes(len);
        std::copy(data, data + len, beginWrite());
        writerIndex_ += len;
    }

    char *beginWrite()
    {
        return begin() + writerIndex_;
    }

    const char *beginWrite() const
    {
        return begin() + writerIndex_;
    }

    // 从fd上读取数据
    ssize_t readFd(int fd, int *saveErrno);
    // 从fd上发送数据
    ssize_t writeFd(int fd, int *saveErrno);

private:
    char *begin()
    {
        // it.operator*()
        return &(*buffer_.begin()); // vetcor底层数组首元素地址
    }

    const char *begin() const
    {
        return &(*buffer_.begin()); // vetcor底层数组首元素地址
    }

    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
};