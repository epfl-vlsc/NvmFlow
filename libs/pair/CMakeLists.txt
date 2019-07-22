add_library(pair MODULE
  PairPass.cpp
)

message(${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(pair PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})