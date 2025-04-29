cmake_minimum_required(VERSION 3.20)
project(SimpleChessEngine)
set(CMAKE_CXX_STANDARD 20)
add_executable(${PROJECT_NAME} src/main.cpp)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()
if (CMAKE_BUILD_TYPE MATCHES Release)
    set(COMPILE_OPTS -Wall -Wextra -Werror -O2)
elseif(CMAKE_BUILD_TYPE MATCHES Debug)
    set(COMPILE_OPTS -Wall -Wextra -Werror -pedantic -pedantic-errors
            -O1 -fno-omit-frame-pointer -fno-sanitize-recover=all
            -fsanitize-recover=alignment)
endif()

if(COMPILE_OPTS)
    target_compile_options(${PROJECT_NAME} PUBLIC ${COMPILE_OPTS})
endif()

