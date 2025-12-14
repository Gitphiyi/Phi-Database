cmake_minimum_required(VERSION 3.24)
project(PhiDatabase)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

add_library( dblib STATIC
    src/sql-compiler/Lexer.cpp
    src/sql-compiler/Parser.cpp

    src/page-manager/DbFile.cpp
    src/page-manager/PageCache.cpp

    src/storage-manager/Table.cpp
    src/storage-manager/ops/StorageOps.cpp
    src/storage-manager/ops/Selection.cpp
    src/storage-manager/HeapFile.cpp

    src/query-executor/QueryExecutor.cpp
    # src/transaction-processor/TScheduler.cpp
)
target_include_directories(dblib PUBLIC ${PROJECT_SOURCE_DIR}/include) # includes headers publicly, so any target that links dblib will also include headers
# do not need to include every subfolder as compiler resolves them by the path in the #include


# --- Examples ---
# These examples are for in development testing of components and showing code of how these components are to be used
add_subdirectory(examples)

# --- Testing ---
# RUN TESTS: ctest
# RUN SPECIFIC TEST: ctest -R compiler_tests //only run compiler tests. follows name in file not filename
# include(CTest)          # sets BUILD_TESTING option
# if(BUILD_TESTING)
#   add_subdirectory(tests)
# endif()
