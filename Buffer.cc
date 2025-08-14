#include "Buffer.h"

#include "errno.h"
#include <sys/uio.h>

/**
 * 从fd上读取模式 Poller工作在LT模式
 * Buffer缓冲区是有大小的，但是从fd上读数据时候，不知道tcp数据大小
 */
ssize_t Buffer::readFd(int fd, int *saveErrno)
{
    char extrabuf[65536] = {0}; // 栈上内存空间 64K
    struct iovec vec[2];
    const size_t writable = writeableBytes(); // Buffer底层缓冲区剩下可写大小
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;

    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;

    const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);
    if (n < 0)
    {
        *saveErrno = errno;
    }
    else if (n <= writable) // buffer可写缓冲区可以存储数据
    {
        writerIndex_ += n;
    }
    else // extrabuf也写入了数据
    {
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable); // 从writeIndex_开始写
    }

    return n;
}

ssize_t Buffer::writeFd(int fd, int *saveErrno)
{
    ssize_t n = ::write(fd, peek(), readableBytes());
    if (n < 0)
    {
        *saveErrno = errno;
    }
    return n;
}