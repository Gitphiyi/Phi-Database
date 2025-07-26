cc_library(
    name = "phi_lib",
    deps = [
        "//src/DbFile:dbfile_lib",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "types_lib",
    hdrs = ["Types.hpp"],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "startup",
    srcs = ["main.cpp"],
    deps = [
        ":phi_lib"
    ]
)
