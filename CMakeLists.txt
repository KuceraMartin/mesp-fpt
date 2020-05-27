cmake_minimum_required(VERSION 3.13)
project(mesp-fpt)

set(CMAKE_CXX_STANDARD 17)

set(DISJOINT_PATHS disjoint_paths/disjoint_paths.hpp)
set(MESP mesp/constrained_set_cover.hpp mesp/mesp_inner.hpp mesp/mesp_multithread.hpp)

if (DEFINED ENV{USE_STATIC_LIBS})
    set(Boost_USE_STATIC_LIBS ON)
    set(LINK_LIBS -static-libstdc++)
endif ()

find_package(Boost 1.72.0 REQUIRED COMPONENTS filesystem thread chrono)

add_executable(paths disjoint_paths/main.cpp ${DISJOINT_PATHS} common/executor.hpp)
target_link_libraries(paths Boost::chrono Boost::filesystem ${LINK_LIBS})

add_executable(mesp mesp/main.cpp ${MESP} common/common.hpp common/graph.hpp common/input.hpp common/executor.hpp)
target_link_libraries(mesp Boost::chrono Boost::filesystem Boost::thread ${LINK_LIBS})

add_executable(test test/main.cpp ${MESP} ${DISJOINT_PATHS} common/graph.hpp common/executor.hpp)
target_link_libraries(test Boost::chrono Boost::filesystem Boost::thread ${LINK_LIBS})
