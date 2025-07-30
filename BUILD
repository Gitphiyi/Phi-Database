filegroup(
    name = "db",
    srcs = glob(["db/**"]),  # Include all files under db/
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "phi_database",
    srcs = [
        "main.cpp",
    ],
    deps = [
        "//src/DbFile:dbfile_lib",
    ],
    data = [
        "//:db", 
    ],
)
