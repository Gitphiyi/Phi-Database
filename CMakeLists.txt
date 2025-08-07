cmake_minimum_required(VERSION 3.20)
project(PhiDatabase)

set(CMAKE_CXX_STANDARD 17)

add_library( dblib STATIC 
    src/DbFile.cpp 
    #src/PageCache.cpp
)
target_include_directories(dblib PUBLIC ${PROJECT_SOURCE_DIR}/include) #directories refers to folder so any #include refers to the include folder

add_executable(main main.cpp)
target_link_libraries(main PRIVATE dblib)

