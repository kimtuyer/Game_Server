cmake_minimum_required(VERSION 3.19)

project(GameServer)

include(cmake/CPM.cmake)

CPMAddPackage(
  NAME
  couchbase_cxx_client
  GIT_TAG
  1.0.4
  VERSION
  1.0.4
  GITHUB_REPOSITORY
  "couchbase/couchbase-cxx-client"
  OPTIONS
  "COUCHBASE_CXX_CLIENT_STATIC_BORINGSSL ON")



add_executable(GameServer GameServer.cpp)
target_link_libraries(GameServer PRIVATE couchbase_cxx_client::couchbase_cxx_client)

# C++ 표준 설정
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)