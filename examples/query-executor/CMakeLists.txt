set(EXECUTOR_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/executor_demo.cpp"
)

foreach(src IN LISTS EXECUTOR_SRCS)
    message(STATUS "Source file: ${src}")
    get_filename_component(name ${src} NAME_WE)
    add_executable(${name} ${src})
    target_link_libraries(${name} PRIVATE dblib)
    set_target_properties(${name} PROPERTIES
       RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endforeach()
