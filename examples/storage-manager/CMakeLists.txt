set(STORAGE_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/table.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/storageops.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/heapfile.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/bplustree_demo.cpp"
    )

foreach(src IN LISTS STORAGE_SRCS)
    message(STATUS "Source file: ${src}")
    get_filename_component(name ${src} NAME_WE) # NAME_WE gets name without extension
    add_executable(${name} ${src})
    target_link_libraries(${name} PRIVATE dblib)
    set_target_properties(${name} PROPERTIES
       RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin 
    )
endforeach()
