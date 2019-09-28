add_library(log MODULE
  LogPass.cpp
)

message(${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(log PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})