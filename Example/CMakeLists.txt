cmake_minimum_required(VERSION 3.8)
project(PeIteratorExample LANGUAGES CXX)

add_executable(PeIteratorExample Main.cpp)
target_link_libraries(PeIteratorExample PUBLIC PeIterator)