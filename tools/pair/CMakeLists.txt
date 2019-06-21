add_executable(pair
    main.cpp
)

target_include_directories(pair PRIVATE ${PROJECT_SOURCE_DIR}/include)

llvm_map_components_to_libnames(REQ_LLVM_LIBRARIES ${LLVM_TARGETS_TO_BUILD}
        asmparser core linker bitreader irreader ipo scalaropts
        analysis support
)

target_link_libraries(pair ${REQ_LLVM_LIBRARIES})

# Platform dependencies.
find_package(Threads REQUIRED)
find_package(Curses REQUIRED)

target_link_libraries(pair
${CMAKE_THREAD_LIBS_INIT}
${CMAKE_DL_LIBS}
${CURSES_LIBRARIES}
)

set_target_properties(pair
                      PROPERTIES
                      LINKER_LANGUAGE CXX
                      PREFIX ""
)

install(TARGETS pair
  RUNTIME DESTINATION bin
)