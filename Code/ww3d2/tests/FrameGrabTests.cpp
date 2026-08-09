/*
** Command & Conquer Renegade(tm)
** Copyright 2025 Electronic Arts Inc.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
*/

#include "framgrab.h"
#include "ww3d.h"

#include <cstdio>
#include <limits>
#include <type_traits>

static_assert(std::is_same_v<decltype(&WW3D::Start_Movie_Capture),
                             void (*)(const char *, float)>);
static_assert(std::is_same_v<decltype(&WW3D::Try_Start_Movie_Capture),
                             bool (*)(const char *, float)>);
static_assert(std::is_same_v<decltype(&WW3D::Update_Movie_Capture), void (*)()>);
static_assert(std::is_same_v<decltype(&WW3D::Try_Update_Movie_Capture), bool (*)()>);
static_assert(std::is_same_v<decltype(&WW3D::Try_Update_Movie_Capture_From_Back_Buffer),
                             bool (*)()>);

namespace
{
bool CheckStride(int width, int bitdepth, unsigned int expected)
{
    const unsigned int actual = FrameGrabClass::Calculate_Row_Stride(width, bitdepth);
    if (actual == expected) {
        return true;
    }

    std::fprintf(stderr,
                 "row stride for %dx%d was %u, expected %u\n",
                 width,
                 bitdepth,
                 actual,
                 expected);
    return false;
}
}

int main()
{
    bool passed = true;
    passed &= CheckStride(0, 24, 0);
    passed &= CheckStride(1, 24, 4);
    passed &= CheckStride(2, 24, 8);
    passed &= CheckStride(3, 24, 12);
    passed &= CheckStride(4, 24, 12);
    passed &= CheckStride(955, 24, 2868);
    passed &= CheckStride(std::numeric_limits<int>::max(), 32, 0);

    FrameGrabClass invalid_dimensions("unused", FrameGrabClass::AVI, 0, 8, 24, 30.0f);
    if (invalid_dimensions.IsReady() || invalid_dimensions.GetBuffer() != nullptr ||
        invalid_dimensions.GetBufferSize() != 0 ||
        invalid_dimensions.GetLastError() != E_INVALIDARG) {
        std::fprintf(stderr, "invalid capture dimensions did not leave a safe, failed writer\n");
        passed = false;
    }

    FrameGrabClass invalid_path("?:\\OpenW3D\\FrameGrabFailure",
                                FrameGrabClass::AVI,
                                4,
                                2,
                                24,
                                30.0f);
    if (invalid_path.IsReady() || invalid_path.GetBuffer() != nullptr ||
        !FAILED(invalid_path.GetLastError()) || invalid_path.Grab(nullptr)) {
        std::fprintf(stderr, "AVI open failure did not remain observable and safe\n");
        passed = false;
    }
    invalid_path.ConvertGrab(nullptr);

    return passed ? 0 : 1;
}
