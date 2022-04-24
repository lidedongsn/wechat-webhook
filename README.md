## wechat-webhook

wechat-webhook is used for gitlab notification to WeChat Work group.
Inspired by dingding gitlab robot.
### usage

1. mkdir build && cd build && cmake ..
2. just start wechat-webhook instance,
3. gitlab config webhook url

```shell
http://<ip>:port/webhook/gitlab?id=<robot token id>
http://<ip>:port/webhook/grafana?id=<robot token id>
```

### features

- [x] push event
- [x] tag event
- [x] merge event
- [ ] branch create/delete event
- [x] grafana alert

### dependency

- [nlohmann/json.hpp](https://github.com/nlohmann/json)
- [fmt](https://github.com/fmtlib/fmt) 
- [spdlog](https://github.com/gabime/spdlog) 
- [restclient-cpp](https://github.com/mrtazz/restclient-cpp)
- [cpp-httplib](https://github.com/yhirose/cpp-httplib)

