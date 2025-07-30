cc_library(
    name = "phi_lib",
    deps = [
        ":types_lib",
        ":page_lib",
        "//src/DbFile:dbfile_lib",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "types_lib",
    hdrs = ["Types.hpp"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "page_lib",
    hdrs = ["Page.hpp"],
    deps = [":types_lib"],
    visibility = ["//visibility:public"],
)


cc_binary(
    name = "main",
    srcs = ["main.cpp"],
    deps = [
        ":phi_lib"
    ]
)
