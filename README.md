## wechat-webhook

wechat-webhook is used for gitlab notification to WeChat Work group.

### useage

1. just start wechat-webhook instance,
2. gitlab config webhook url

```shell
http://<ip>:port/webhook?id=<robot token id>
```

### features

- [x] push event
- [ ] tag event
- [ ] merge event
- [ ] branch create/delete event

### dependency

- [nlohmann/json.hpp](https://github.com/nlohmann/json)
- [fmt](https://github.com/fmtlib/fmt) 
- [spdlog](https://github.com/gabime/spdlog) 
- [restclient-cpp](https://github.com/mrtazz/restclient-cpp)
- [cpp-httplib](https://github.com/yhirose/cpp-httplib)

