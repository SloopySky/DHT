cmake_minimum_required(VERSION 3.5.0)
project(DHT VERSION 0.1.0 LANGUAGES C)

set(SRC "src/dht.c")


if (DHT_TEST_ENABLE)
    set(DHT_STUBS_ENABLE TRUE)
endif()

if(DHT_STUBS_ENABLE)
    list(APPEND SRC "src/stubs.c")
endif()

add_library(DHT STATIC ${SRC})
target_include_directories(DHT PUBLIC include)

if (DHT_TEST_ENABLE)
    add_executable(DhtTest "tests/main.c")
    target_link_libraries(DhtTest PRIVATE DHT)
endif()

