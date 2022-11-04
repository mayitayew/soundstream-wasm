workspace(name = "soundstream")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# GoogleTest/GoogleMock framework.
git_repository(
    name = "com_google_googletest",
    remote = "https://github.com/google/googletest.git",
    tag = "release-1.10.0",
)

# Google Abseil Libs
git_repository(
    name = "com_google_absl",
    remote = "https://github.com/abseil/abseil-cpp.git",
    branch = "lts_2020_09_23",
)

# Filesystem
# The new_* prefix is used because it is not a bazel project and there is
# no BUILD file in that repo.
FILESYSTEM_BUILD = """
cc_library(
  name = "filesystem",
  hdrs = glob(["include/ghc/*"]),
  visibility = ["//visibility:public"],
)
"""

new_git_repository(
    name = "gulrak_filesystem",
    remote = "https://github.com/gulrak/filesystem.git",
    tag = "v1.3.6",
    build_file_content = FILESYSTEM_BUILD
)

# Begin Tensorflow WORKSPACE subset required for TFLite

git_repository(
    name = "org_tensorflow",
    remote = "https://github.com/tensorflow/tensorflow.git",
    # Below is reproducible and equivalent to `tag = "v2.9.0"`
    commit = "8a20d54a3c1bfa38c03ea99a2ad3c1b0a45dfa95",
    shallow_since = "1652465115 -0700"
)

# Check bazel version requirement, which is stricter than TensorFlow's.
load(
    "@org_tensorflow//tensorflow:version_check.bzl",
    "check_bazel_version_at_least",
)

check_bazel_version_at_least("3.7.2")

# TF WORKSPACE Loading functions
# This section uses a subset of the tensorflow WORKSPACE loading by reusing its contents.
# There are four workspace() functions create repos for the dependencies.
# TF's loading is very complicated, and we only need a subset for TFLite.
# If we use the full TF loading sequence, we also run into conflicts and errors on some platforms.

load("@org_tensorflow//tensorflow:workspace3.bzl", "workspace")
workspace()

load("@org_tensorflow//tensorflow:workspace2.bzl", workspace2 = "workspace")
workspace2()

# End Tensorflow WORKSPACE subset required for TFLite

# Webassembly bazel
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")
http_archive(
    name = "emsdk",
    sha256 = "d55e3c73fc4f8d1fecb7aabe548de86bdb55080fe6b12ce593d63b8bade54567",
    strip_prefix = "emsdk-3891e7b04bf8cbb3bc62758e9c575ae096a9a518/bazel",
    url = "https://github.com/emscripten-core/emsdk/archive/3891e7b04bf8cbb3bc62758e9c575ae096a9a518.tar.gz",
)

load("@emsdk//:deps.bzl", emsdk_deps = "deps")
emsdk_deps()

load("@emsdk//:emscripten_deps.bzl", emsdk_emscripten_deps = "emscripten_deps")
emsdk_emscripten_deps(emscripten_version = "2.0.31")