#include <fmt/core.h>

#include <sstream>
#include <vector>

#include "cmdline.h"
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
  _svr.Post("/webhook/gitlab", [&](const Request &req, Response &res) {
    GitLabWebhookHandler(req, res);
  });
  _svr.Post("/webhook/grafana", [&](const Request &req, Response &res) {
    GrafanaWebhookHandler(req, res);
  });
  _logger->info("wechat-webhook service run at port: {}", _port);
  _svr.listen("0.0.0.0", _port);
}

void WeChatRobot::GitLabWebhookHandler(const httplib::Request &req,
                                       httplib::Response &res) {
  _logger->info("Rcev gitlab request >>>>>>>>>> {}", req.body);
  std::string id;
  char buf[1024];

  for (auto it = req.params.begin(); it != req.params.end(); ++it) {
    const auto &x = *it;
    if (!x.first.compare("id")) {
      id = x.second;
    }
  }
  if (!id.empty()) {
    json body = json::parse(req.body);
    if (body["object_kind"] == "push") {
      BuildPushEventAndSendWechat("push", id, body);
    } else if (body["object_kind"] == "tag_push") {
      BuildPushEventAndSendWechat("tag", id, body);
    }
  }

  res.set_content("Ok", "text/plain");
}

void WeChatRobot::GrafanaWebhookHandler(const httplib::Request &req,
                                        httplib::Response &res) {
  _logger->info("Rcev grafana request >>>>>>>>>> {}", req.body);
  std::string id;
  char buf[1024];

  for (auto it = req.params.begin(); it != req.params.end(); ++it) {
    const auto &x = *it;
    if (!x.first.compare("id")) {
      id = x.second;
    }
  }
  if (!id.empty()) {
    json body = json::parse(req.body);
    json msg, markdown;
    markdown["content"] = body["message"];
    msg["msgtype"] = "markdown";
    msg["markdown"] = markdown;
    RestClient::Response r = RestClient::post(
        kWechatRobotWebhookUrl + "/cgi-bin/webhook/send?key=" + id,
        "application/json", msg.dump());
    _logger->info("Wechat robot resp code: {} body: {}", r.code, r.body);
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

void WeChatRobot::BuildPushEventAndSendWechat(std::string type, std::string id,
                                              json body) {
  std::vector<std::string> ref = _split(body["ref"].get<std::string>(), '/');
  std::string branch;
  if (ref.size() > 2) {
    ref.erase(ref.begin(), ref.begin() + 2);

    for (std::vector<std::string>::iterator it = ref.begin(); it != ref.end();
         ++it) {
      branch = branch + "/" + *it;
    }
  }

  std::string auther = body["user_name"];
  // std::string branch = ref[2];
  std::string repository = body["repository"]["name"];
  std::string url = body["repository"]["homepage"];
  std::vector<std::string> commits;

  json msg, markdown;
  std::string content;

  if (!type.compare("tag")) {
    std::string action = "add";
    if (body["ref"].dump().compare(
            "0000000000000000000000000000000000000000")) {
      action = "remove";
    }
    std::string title = fmt::format(
        "{} {} tag <font color=\"warning\">{}</font> at <font "
        "color=\"comment\">[{}]({})</font>\n",
        auther, action, branch, repository, url);
    commits.push_back(title);
  } else if (!type.compare("push")) {
    std::string title = fmt::format(
        "{} pushed to branch <font color=\"warning\">{}</font> at <font "
        "color=\"comment\">[{}]({})</font>\n",
        auther, branch, repository, url);
    commits.push_back(title);
    for (unsigned i = 0; i < body["commits"].size(); i++) {
      std::string commit = fmt::format(
          ">[{}]({}): {}",
          body["commits"].at(i)["id"].get<std::string>().substr(0, 6),
          body["commits"].at(i)["url"].get<std::string>(),
          body["commits"].at(i)["message"].get<std::string>());
      _logger->info("commit >>> {}", commit);
      commits.push_back(commit);
    }
  }
  for (unsigned i = 0; i < commits.size(); i++) {
    content = content + commits.at(i);
  }

  markdown["content"] = content;
  msg["msgtype"] = "markdown";
  msg["markdown"] = markdown;

  RestClient::Response r = RestClient::post(
      kWechatRobotWebhookUrl + "/cgi-bin/webhook/send?key=" + id,
      "application/json", msg.dump());
  _logger->info("Wechat robot resp code: {} body: {}", r.code, r.body);
}

int main(int argc, char *argv[]) {
  cmdline::parser flags;
  int port = 9527;
  flags.add<int>("port", 'p', "port number", false, 9527,
                 cmdline::range(1, 65535));

  flags.parse_check(argc, argv);

  if (flags.exist("port")) {
    port = flags.get<int>("port");
  }
  WeChatRobot robot(port);
  robot.Run();
  return 0;
}