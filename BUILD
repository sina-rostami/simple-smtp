
cc_library(
    name="libuvw",
    srcs= glob(["uvw-2.12.1/src/**/*.cpp"]),
    hdrs= glob(["uvw-2.12.1/src/**/*.h"]) + glob(["uvw-2.12.1/src/**/*.hpp"]),
    defines = ["UVW_AS_LIB", ],
    linkopts = ["-luv"]
)

cc_library(
    name="server_lib",
    srcs=glob(["src/server/**/*.hh"]),
    hdrs=glob(["src/server/**/*.hh"]),
    deps = [":libuvw"],
    includes = ["uvw-2.12.1/src"],
)

cc_binary (
    name = "server",
    srcs = [
        "src/server/server.cc",
    ],
    deps = [":server_lib"],
    includes = ["src"],
)