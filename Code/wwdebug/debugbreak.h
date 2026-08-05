/*
**	Command & Conquer Renegade(tm)
**	Copyright 2026 OpenW3D.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#if defined _MSC_VER
#define debugbreak() __debugbreak()
#elif defined __has_builtin && __has_builtin(__builtin_debugtrap)
#define debugbreak() __builtin_debugtrap()
/* If we have GCC or compiler that tries to be compatible, use GCC inline assembly. */
#elif defined __GNUC__ || defined __clang__
#if defined(__i386__) || defined(__x86_64__)
__attribute__((always_inline)) __inline__ static void debugbreak(void)
{
    __asm__ volatile("int3");
}
#elif defined(__thumb__)
__attribute__((always_inline)) __inline__ static  void debugbreak(void)
{
    __asm__ volatile(".inst 0xde01");
}
#elif defined(__arm__)
__attribute__((always_inline)) __inline__ static  void debugbreak(void)
{
    __asm__ volatile(".inst 0xe7f001f0");
}
#elif defined(__aarch64__)
__attribute__((always_inline)) __inline__ static  void debugbreak(void)
{
    __asm__ volatile(".inst 0xd4200000");
}
#elif defined(__powerpc__)
__attribute__((always_inline)) __inline__ static  void debugbreak(void)
{
    __asm__ volatile(".4byte 0x7d821008");
}
#elif defined(__riscv)
__attribute__((always_inline)) __inline__ static  void debugbreak(void)
{
    __asm__ volatile(".4byte 0x00100073");
}
#else
#error debugbreak not currently supported on this processor platform, see debugbreak.h
#endif /* CPU architectures on GCC like compilers */
#else
#error debugbreak not currently supported on this processor platform, see debugbreak.h
#endif
