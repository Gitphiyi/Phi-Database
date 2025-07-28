cc_library(
    name = "table_lib",
    srcs = glob(["*.cpp"]),        
    hdrs = glob(["*.hpp"]),
    includes = [".",".."],         
    deps = [
        "//src:types_lib",
        "//src/DbFile:dbfile_lib",
        "//src/PageCache:pagecache_lib"
    ],
    visibility = ["//visibility:public"],
)
