cc_library(
    name = "pagecache_lib",
    srcs = glob(["*.cpp"]),        
    hdrs = glob(["*.hpp"]),
    includes = [".",".."],         
    deps = [
        "//src/DbFile:dbfile_lib"
        "//src:types_lib",
        "//src:page_lib",
    ],
    visibility = ["//visibility:public"],
)
