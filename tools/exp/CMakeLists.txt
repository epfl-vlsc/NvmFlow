add_executable(exp
    main.cpp
)

target_include_directories(exp PRIVATE ${PROJECT_SOURCE_DIR}/include)

llvm_map_components_to_libnames(REQ_LLVM_LIBRARIES ${LLVM_TARGETS_TO_BUILD}
        asmparser core linker bitreader irreader ipo scalaropts
        analysis support
)

target_link_libraries(exp ${REQ_LLVM_LIBRARIES})

# Platform dependencies.
find_package(Threads REQUIRED)
find_package(Curses REQUIRED)

target_link_libraries(exp
${CMAKE_THREAD_LIBS_INIT}
${CMAKE_DL_LIBS}
${CURSES_LIBRARIES}
)

set_target_properties(exp
                      PROPERTIES
                      LINKER_LANGUAGE CXX
                      PREFIX ""
)

install(TARGETS exp
  RUNTIME DESTINATION bin
)
