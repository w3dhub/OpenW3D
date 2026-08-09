/*
**	Command & Conquer Renegade(tm)
**	Copyright 2026 OpenW3D Contributors.
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

#include "part_ldr.h"

#include "chunkio.h"
#include "ramfile.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <iostream>

namespace
{
class TestEmitterDef : public ParticleEmitterDefClass
{
public:
	using ParticleEmitterDefClass::Save_Blur_Time_Keyframes;
	using ParticleEmitterDefClass::Save_Frame_Keyframes;
	using ParticleEmitterDefClass::Save_Rotation_Keyframes;
};

#if defined(_MSC_VER)
#define W3D_NOINLINE __declspec(noinline)
#elif defined(__GNUC__)
#define W3D_NOINLINE __attribute__((noinline))
#else
#define W3D_NOINLINE
#endif

W3D_NOINLINE void PoisonStack(unsigned char value)
{
	volatile unsigned char bytes[4096];
	for (std::size_t index = 0; index < sizeof(bytes); ++index) {
		bytes[index] = value;
	}
}

#undef W3D_NOINLINE

bool Expect(bool condition, const char *message)
{
	if (!condition) {
		std::cerr << message << '\n';
	}
	return condition;
}
}

int main()
{
	std::array<unsigned char, 128> storage{};
	RAMFileClass file(storage.data(), static_cast<int>(storage.size()));
	if (!Expect(file.Open(FileClass::WRITE), "Could not open the RAM file for writing")) {
		return 1;
	}

	ChunkSaveClass chunkSave(&file);
	TestEmitterDef emitter;

	PoisonStack(0xa5);
	bool passed = Expect(
		emitter.Save_Rotation_Keyframes(chunkSave) == WW3D_ERROR_OK,
		"Rotation keyframe serialization failed");
	PoisonStack(0x5a);
	passed &= Expect(
		emitter.Save_Frame_Keyframes(chunkSave) == WW3D_ERROR_OK,
		"Frame keyframe serialization failed");
	PoisonStack(0x3c);
	passed &= Expect(
		emitter.Save_Blur_Time_Keyframes(chunkSave) == WW3D_ERROR_OK,
		"Blur-time keyframe serialization failed");
	passed &= Expect(chunkSave.Cur_Chunk_Depth() == 0,
		"Emitter keyframe serializers left a chunk open");

	constexpr std::size_t ExpectedSize = 92;
	std::array<unsigned char, ExpectedSize> expected{};
	expected[0] = 0x0a;
	expected[1] = 0x05;
	expected[4] = 0x18;
	expected[32] = 0x0b;
	expected[33] = 0x05;
	expected[36] = 0x18;
	expected[64] = 0x0c;
	expected[65] = 0x05;
	expected[68] = 0x14;

	passed &= Expect(file.Size() == static_cast<int>(expected.size()),
		"Emitter keyframe serialization wrote an unexpected byte count");
	file.Close();

	const auto mismatch = std::mismatch(
		expected.begin(), expected.end(), storage.begin());
	if (mismatch.first != expected.end()) {
		const std::size_t offset =
			static_cast<std::size_t>(mismatch.first - expected.begin());
		std::cerr << "Emitter keyframe golden mismatch at offset " << offset
			<< ": expected 0x" << std::hex
			<< static_cast<unsigned int>(*mismatch.first)
			<< ", got 0x" << static_cast<unsigned int>(*mismatch.second)
			<< std::dec << '\n';
		passed = false;
	}

	return passed ? 0 : 1;
}
