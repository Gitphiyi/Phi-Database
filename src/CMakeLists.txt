add_subdirectory(DbFile)
# add_subdirectory(Database)
# add_subdirectory(Logger)
# add_subdirectory(PageCache)
# add_subdirectory(Table)
# add_subdirectory(WriteAheadLog)

add_executable(main main.cpp)
target_include_directories(main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(main dbfile helperLib) 

add_library(helperLib INTERFACE)
target_include_directories(helperLib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})


