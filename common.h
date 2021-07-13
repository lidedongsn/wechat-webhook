#ifndef _COMMON_H
#define _COMMON_H
#include <iostream>

#include "httplib.h"
#include "nlohmann/json.hpp"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"
using namespace httplib;
using json = nlohmann::json;

// https://qyapi.weixin.qq.com/cgi-bin/webhook/send?key=<token id>
const std::string kWechatRobotWebhookUrl = "https://qyapi.weixin.qq.com";

class WeChatRobot : public httplib::Server {
 public:
  WeChatRobot(int port);

  void WebhookHandler(const Request &req, Response &res);
  void BuildPushEventAndSendWechat(std::string type, std::string id, json body);
  void Run();

 private:
  inline std::vector<std::string> _split(const std::string &s, char delim);

 private:
  std::string _ip;   // webhook http server bind ip
  int _port;         // webhook http server port
  std::string _url;  // wechat robot url;
  std::shared_ptr<spdlog::logger> _logger;
  httplib::Server _svr;
};

#endif