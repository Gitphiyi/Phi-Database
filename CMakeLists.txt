# TO BUILD CMAKE: cmake -S . -B build
# make -j //to compile in build folder
cmake_minimum_required(VERSION 3.20)
project(PhiDatabase)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

add_library( dblib STATIC 
    src/compiler/QueryProcessor.cpp
    src/DbFile.cpp 
    # src/Database.cpp
    # src/Table.cpp
    src/PageCache.cpp
    # src/TScheduler.cpp
)
target_include_directories(dblib PUBLIC ${PROJECT_SOURCE_DIR}/include) #directories refers to folder so any #include refers to the include folder

add_executable(main main.cpp)
target_link_libraries(main PRIVATE dblib)

# --- Testing ---
# RUN TESTS: ctest
# RUN SPECIFIC TEST: ctest -R compiler_tests //only run compiler tests. follows name in file not filename
include(CTest)          # sets BUILD_TESTING option
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()