cmake_minimum_required(VERSION 3.20)
project(PhiDatabase)

set(CMAKE_CXX_STANDARD 20)
#add_compile_options(-fsanitize=address)
#add_link_options(-fsanitize=address)

add_library( dblib STATIC 
    src/DbFile.cpp 
    src/Database.cpp
    #src/Table.cpp
    src/PageCache.cpp
)
target_include_directories(dblib PUBLIC ${PROJECT_SOURCE_DIR}/include) #directories refers to folder so any #include refers to the include folder

add_executable(main main.cpp)
target_link_libraries(main PRIVATE dblib)

