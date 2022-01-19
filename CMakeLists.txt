cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

project(experiment_service
        VERSION 2019.1.0
        DESCRIPTION "experiment services"
        LANGUAGES CXX)

find_package(Dependencies)

install_dependency(https://github.com/germanespinosa/tcp-messages Tcp_messages)
install_dependency(https://github.com/germanespinosa/agent_tracking Agent_tracking)
install_dependency(https://github.com/germanespinosa/cellworld Cellworld)

string(APPEND CMAKE_CXX_FLAGS " -fno-strict-aliasing -pthread ")

include_directories(include)

set(experiment_service_files
        src/experiment_client.cpp
        src/experiment_service.cpp
        )

add_library(experiment_service ${experiment_service_files})

target_link_libraries( experiment_service
        LINK_PUBLIC
        cellworld
        agent_tracking )

add_executable(experiment_server src/tools/server.cpp)

target_link_libraries(experiment_server experiment_service)

###
### TESTS
###
install_dependency(https://github.com/germanespinosa/catch CatchTests)
test_library(experiment_service catchtests/client_test.cpp)
