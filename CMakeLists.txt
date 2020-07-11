cmake_minimum_required(VERSION 3.15)
project(Minisql)

set(CMAKE_CXX_STANDARD 14)

add_executable(Minisql API.h  API.cpp Attribute.h Attribute.cpp BPlusTree.h BufferManager.cpp BufferManager.h CatalogManager.cpp CatalogManager.h Condition.h Condition.cpp IndexManager.cpp IndexManager.h Interpreter.h Interpreter.cpp main.cpp  RecordManager.cpp RecordManager.h)
