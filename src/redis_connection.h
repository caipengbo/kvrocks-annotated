#pragma once

#include <vector>
#include <string>
#include <utility>
#include <memory>

// libevent
#include <event2/buffer.h>

#include "redis_cmd.h"
#include "redis_request.h"

class Worker;

namespace Redis {
class Connection {
 public:
  enum Flag {
    kSlave           = 1 << 4,
    kMonitor         = 1 << 5,
    kCloseAfterReply = 1 << 6,
  };

  explicit Connection(bufferevent *bev, Worker *owner);
  ~Connection();

  void Close();
  void Detach();
  static void OnRead(struct bufferevent *bev, void *ctx);  // 有数据可读
  static void OnWrite(struct bufferevent *bev, void *ctx);  // buffer 中的数据已读完
  static void OnEvent(bufferevent *bev, int16_t events, void *ctx);  // 其他的一些事件处理，如关闭，超时，出错
  void Reply(const std::string &msg);  // 回复
  void SendFile(int fd);
  std::string ToString();

  // Pub/Sub 相关
  typedef std::function<void(std::string, int)> unsubscribe_callback;
  void SubscribeChannel(const std::string &channel);
  void UnSubscribeChannel(const std::string &channel);
  void UnSubscribeAll(unsubscribe_callback reply = nullptr);
  int SubscriptionsCount();
  void PSubscribeChannel(const std::string &pattern);
  void PUnSubscribeChannel(const std::string &pattern);
  void PUnSubscribeAll(unsubscribe_callback reply = nullptr);
  int PSubscriptionsCount();

  uint64_t GetAge();
  uint64_t GetIdleTime();
  void SetLastInteraction();
  std::string GetFlags();
  void EnableFlag(Flag flag);
  bool IsFlagEnabled(Flag flag);
  bool IsRepl();

  uint64_t GetID() { return id_; }
  void SetID(uint64_t id) { id_ = id; }
  std::string GetName() { return name_; }
  void SetName(std::string name) { name_ = std::move(name); }
  std::string GetAddr() { return addr_; }
  void SetAddr(std::string ip, int port);
  void SetLastCmd(std::string cmd) { last_cmd_ = std::move(cmd); }
  std::string GetIP() { return ip_; }
  int GetPort() { return port_; }
  void SetListeningPort(int port) { listening_port_ = port; }
  int GetListeningPort() { return listening_port_; }

  bool IsAdmin() { return is_admin_; }
  void BecomeAdmin() { is_admin_ = true; }
  void BecomeUser() { is_admin_ = false; }
  std::string GetNamespace() { return ns_; }
  void SetNamespace(std::string ns) { ns_ = std::move(ns); }

  Worker *Owner() { return owner_; }
  int GetFD() { return bufferevent_getfd(bev_); }
  evbuffer *Input() { return bufferevent_get_input(bev_); }
  evbuffer *Output() { return bufferevent_get_output(bev_); }
  bufferevent *GetBufferEvent() { return bev_; }

  // 当前正在处理的命令
  std::unique_ptr<Commander> current_cmd_;

 private:
  uint64_t id_ = 0;
  int flags_ = 0;
  std::string ns_;
  std::string name_;
  std::string ip_;
  int port_ = 0;
  std::string addr_;
  int listening_port_ = 0;
  bool is_admin_ = false;
  std::string last_cmd_;
  time_t create_time_;
  time_t last_interaction_;

  bufferevent *bev_;  // 该连接的 bufferevent
  Request req_;  // 请求
  Worker *owner_;
  std::vector<std::string> subscribe_channels_;
  std::vector<std::string> subcribe_patterns_;
};
}  // namespace Redis
