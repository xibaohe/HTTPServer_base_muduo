
#ifndef MUDUO_NET_TCPCONNECTION_H
#define MUDUO_NET_TCPCONNECTION_H

#include <muduo/base/noncopyable.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/Callbacks.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/Socket.h>

#include <memory>

namespace muduo {
namespace net {

class Channel;
class EventLoop;

///
/// TCP connection, for both client and server usage.
///
class TcpConnection : noncopyable,
                      public std::enable_shared_from_this<TcpConnection> {
 public:
  /// Constructs a TcpConnection with a connected sockfd
  ///
  /// User should not create this object.
  TcpConnection(EventLoop* loop, const std::string& name, Socket&& sockfd,
                const InetAddress& localAddr, const InetAddress& peerAddr);
  ~TcpConnection();

  EventLoop* getLoop() const { return loop_; }
  const std::string& name() const { return name_; }
  const InetAddress& localAddress() { return localAddr_; }
  const InetAddress& peerAddress() { return peerAddr_; }
  bool connected() const { return state_ == kConnected; }

  // void send(const void* message, size_t len);
  // Thread safe.
  void send(const std::string& message);
  // Thread safe.
  void shutdown();

  // cannot use rvalue
  void setConnectionCallback(const ConnectionCallback& cb) {
    connectionCallback_ = cb;
  }

  void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }

  /// Internal use only.
  void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }

  // called when TcpServer accepts a new connection
  void connectEstablished();  // should be called only once

  // called when TcpServer has removed me from its map
  void connectDestroyed();  // should be called only once

 private:
  enum StateE {
    kConnecting,
    kConnected,
    kDisconnecting,
    kDisconnected,
  };

  void setState(StateE s) { state_ = s; }
  void handleRead(Timestamp receiveTime);
  void handleWrite();
  void handleClose();
  void handleError();
  void sendInLoop(const std::string& message);
  void shutdownInLoop();

  EventLoop* loop_;
  std::string name_;
  StateE state_;  // FIXME: use atomic variable
  // we don't expose those classes to client.
  // std::unique_ptr<Socket> socket_;
  const Socket socket_;
  const std::unique_ptr<Channel> channel_;
  InetAddress localAddr_;
  InetAddress peerAddr_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  CloseCallback closeCallback_;

  Buffer inputBuffer_;
  Buffer outputBuffer_;
};

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

}  // net
}

#endif  // MUDUO_NET_TCPCONNECTION_H