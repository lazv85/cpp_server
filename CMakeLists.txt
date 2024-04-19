cmake_minimum_required(VERSION 3.10)
project(WebServer)

# Find Boost components
find_package(Boost 1.70 REQUIRED COMPONENTS system)

# Add executable target
add_executable(web_server web_server.cpp)

# Link Boost libraries
target_link_libraries(web_server PRIVATE Boost::system pthread)
