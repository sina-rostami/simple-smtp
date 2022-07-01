
cc_library(
    name="libuvw",
    srcs= glob(["uvw-2.12.1/src/**/*.cpp"]),
    hdrs= glob(["uvw-2.12.1/src/**/*.h"]) + glob(["uvw-2.12.1/src/**/*.hpp"]) + glob(["uvw-2.12.1/src/**/*.cpp"]),
    defines = ["UVW_AS_LIB", ],
    linkopts = ["-luv"]
)

cc_binary (
    name = "server",
    srcs = [
        "server.cc",
    ],
    deps = [":libuvw"],
    includes = ["uvw-2.12.1/src"],
)