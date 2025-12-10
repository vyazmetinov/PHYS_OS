cmake_minimum_required(VERSION 3.30)
project(Phys_OS)

set(CMAKE_CXX_STANDARD 17)

add_executable(Phys_OS pths/integral.c
        msg_queue/itg.c
        msg_queue/cs.c
        pths/itg.c
)
