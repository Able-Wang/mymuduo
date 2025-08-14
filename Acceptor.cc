#include "Acceptor.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Logger.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

static int createNonblocking()
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0)
    {
        LOG_FATAL("%s:%s:%d listen socket create error:%d\n", __FILE__, __FUNCTION__, __LINE__, errno);
    }
    return sockfd;
}

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenaddr, bool reuseport)
    : loop_(loop), acceptSocket_(createNonblocking()), accpetChannel_(loop, acceptSocket_.fd()), listenning_(false)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(true);
    acceptSocket_.bindAddress(listenaddr); // 绑定
    // TcpServer::start(),Acceptor.listen 有新用户连接，执行一个回调(connfd_ => channel => subloop)
    // baseloop => accpetChannel_(listenfd) => handleRead
    accpetChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
    accpetChannel_.disableAll();
    accpetChannel_.remove();
}

void Acceptor::listen()
{
    listenning_ = true;
    acceptSocket_.listen();
    accpetChannel_.enableReading(); // Acceptor => Poller
}

// listenfd有事件发生，有新用户连接
void Acceptor::handleRead()
{
    InetAddress peerAddr;
    int connfd = acceptSocket_.accpet(&peerAddr);
    if (connfd >= 0)
    {
        if (newConnectionCallback_)
        {
            newConnectionCallback_(connfd, peerAddr); // 轮询找到subloop，唤醒，分发当前新客户端的channel
        }
        else
        {
            ::close(connfd);
        }
    }
    else
    {
        LOG_ERROR("%s:%s:%d accept error:%d\n", __FILE__, __FUNCTION__, __LINE__, errno);
        if (errno = EMFILE)
        {
            LOG_ERROR("%s:%s:%d sockfa reached limit!\n", __FILE__, __FUNCTION__, __LINE__);
        }
    }
}