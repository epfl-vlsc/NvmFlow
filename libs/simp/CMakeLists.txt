add_library(simp MODULE
  SimpPass.cpp
)

message(${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(simp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})