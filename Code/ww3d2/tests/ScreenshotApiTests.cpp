/*
** Command & Conquer Renegade(tm)
** Copyright 2025 Electronic Arts Inc.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
*/

#include "ww3d.h"

#include <type_traits>

namespace
{
using LegacyScreenshotFunction = void (*)(const char *);
using BackBufferScreenshotFunction = int (*)(const char *);

static_assert(std::is_same_v<decltype(&WW3D::Make_Screen_Shot), LegacyScreenshotFunction>);
static_assert(std::is_same_v<decltype(&WW3D::Make_Back_Buffer_Screen_Shot),
                             BackBufferScreenshotFunction>);
}

int main()
{
    return 0;
}
