cmake_minimum_required(VERSION 3.3)
project(wechat-webhook)
include_directories(./include)
set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -g -fno-strict-aliasing -O2 -Wall -export-dynamic -Wall -pipe  -D_GNU_SOURCE -D_REENTRANT -fPIC -Wno-deprecated -m64)
set(CMAKE_CXX_STANDARD 11)
link_directories("/usr/local/lib")
add_executable(wechat-webhook wechat-webhook.cpp)
target_link_libraries(wechat-webhook -lfmt -lspdlog -lpthread -lrestclient-cpp -lcurl)
