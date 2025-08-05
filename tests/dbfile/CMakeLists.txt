add_executable(testdbfile test_dbfile.cpp)
target_include_directories(testdbfile PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(testdbfile dbfile helperLib) 
