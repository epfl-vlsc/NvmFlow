add_library(cons MODULE
  ConsPass.cpp
)

message(${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(cons PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})