#include <fmt/core.h>

#include <sstream>
#include <vector>

#include "common.h"
#include "restclient-cpp/restclient.h"

// g++ -o wechat-webhook wechat-webhook.cpp -std=c++11 -lfmt -lspdlog
// -lpthread
// maybe -lrestclient-cpp
WeChatRobot::WeChatRobot(int port) : _port{port}, _url{kWechatRobotWebhookUrl} {
  auto max_size = 1024 * 1024 * 5;
  auto max_files = 3;
  _logger =
      spdlog::basic_logger_mt("wechat-webhook", "logs/wechat-webhook.log");
  // spdlog::set_default_logger(_logger);
  spdlog::flush_on(spdlog::level::info);
}

void WeChatRobot::Run() {
  // _svr.set_post_routing_handler(
  //    [](const auto &req, auto &res) { WebhookHandler(req, res); });
  _svr.Post("/webhook", [&](const Request &req, Response &res) {
    WebhookHandler(req, res);
  });
  _logger->info("wechat-webhook service run at port: {}", _port);
  _svr.listen("0.0.0.0", _port);
}
void WeChatRobot::WebhookHandler(const httplib::Request &req,
                                 httplib::Response &res) {
  _logger->info("Rcev request >>>>>>>>>> {}", req.body);
  std::string id;
  char buf[1024];

  for (auto it = req.params.begin(); it != req.params.end(); ++it) {
    const auto &x = *it;
    if (!x.first.compare("id")) {
      id = x.second;
    }
  }
  if (!id.empty()) {
    _logger->info(id);
    json body = json::parse(req.body);
    if (body["object_kind"] == "push") {
      BuildPushEventAndSendWechat(id, body);
    }
  }

  res.set_content("Ok", "text/plain");
}
std::vector<std::string> WeChatRobot::_split(const std::string &s, char delim) {
  std::stringstream ss(s);
  std::string item;
  std::vector<std::string> elems;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
    // elems.push_back(std::move(item)); // if C++11 (based on comment from
    // @mchiasson)
  }
  return elems;
}

void WeChatRobot::BuildPushEventAndSendWechat(std::string id, json body) {
  std::vector<std::string> ref = _split(body["ref"].get<std::string>(), '/');
  std::string auther = body["user_name"];
  std::string branch = ref[2];
  std::string repository = body["repository"]["name"];
  std::vector<std::string> commits;

  json msg, markdown;
  std::string content;

  std::string title = fmt::format(
      "{} push to branch <font color=\"warning\">{}</font> at <font "
      "color=\"comment\">{}</font>\n",
      auther, branch, repository);
  commits.push_back(title);
  for (unsigned i = 0; i < body["commits"].size(); i++) {
    std::string commit =
        fmt::format(">[{}]({}): {}",
                    body["commits"].at(i)["id"].get<std::string>().substr(0, 6),
                    body["commits"].at(i)["url"].get<std::string>(),
                    body["commits"].at(i)["message"].get<std::string>());
    _logger->info("commit >>> {}", commit);
    commits.push_back(commit);
  }

  for (unsigned i = 0; i < commits.size(); i++) {
    content = content + commits.at(i);
  }

  markdown["content"] = content;
  msg["msgtype"] = "markdown";
  msg["markdown"] = markdown;
  _logger->info("Post>> to {} msg: {}",
                (kWechatRobotWebhookUrl + "/cgi-bin/webhook/send?key=" + id),
                msg.dump());
  /*
httplib::Client cli(kWechatRobotWebhookUrl.c_str());
_logger->info("Post to {} msg: {}",
  (kWechatRobotWebhookUrl + "/cgi-bin/webhook/send?key=" + id),
  msg.dump());

auto res = cli.Post(
(kWechatRobotWebhookUrl + "/cgi-bin/webhook/send?key=" + id).c_str(),
msg.dump(), "application/json");
_logger->info("{}", res);
*/

  RestClient::Response r = RestClient::post(
      kWechatRobotWebhookUrl + "/cgi-bin/webhook/send?key=" + id,
      "application/json", msg.dump());
  _logger->info("Wechat robot resp code: {} body: {}", r.code, r.body);
}

int main() {
  int port = 9527;
  WeChatRobot robot(port);
  robot.Run();
  return 0;
}