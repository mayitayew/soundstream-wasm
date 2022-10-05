package(default_visibility = ["//visibility:public"])


cc_library(
    name = "inverse_spectrogram",
    srcs = ["inverse_spectrogram.cc"],
    hdrs = ["inverse_spectrogram.h"],
    deps = [
        "//audio/dsp:number_util",
        "//audio/third_party/fft2d",
    ],
)

cc_library(
    name = "spectrogram",
    srcs = ["spectrogram.cc"],
    hdrs = ["spectrogram.h"],
    deps = [
        "//audio/dsp:number_util",
        "//audio/dsp:porting",
        "//audio/dsp:window_functions",
        "//audio/third_party/fft2d:fft2d",
    ],
)
