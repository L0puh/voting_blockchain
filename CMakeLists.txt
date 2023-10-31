cmake_minimum_required(VERSION 3.10)
project(voting)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lcrypto -static-libstdc++ -Iinclude -lpthread")

include_directories(include)

find_package(nlohmann_json REQUIRED)
find_package(OpenSSL REQUIRED)

set(SRC main.cpp) 
set(LIBS src/block.cpp src/network.cpp src/vote.cpp)

add_library(blockchain STATIC ${LIBS})
add_executable(voting ${SRC})

target_link_libraries(voting blockchain crypto stdc++ pthread nlohmann_json::nlohmann_json)
