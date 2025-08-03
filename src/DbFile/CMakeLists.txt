add_library(dbfile DbFile.cpp DbFile.hpp)
target_include_directories(dbfile PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(dbfile helperLib) 

