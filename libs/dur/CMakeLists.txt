add_library(dur MODULE
  DurPass.cpp
)

message(${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(dur PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})