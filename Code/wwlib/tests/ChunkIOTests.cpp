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

#include "chunkio.h"
#include "ramfile.h"

#include <array>
#include <cstdint>
#include <cstdio>
#include <optional>
#include <vector>

namespace
{
class FaultInjectRAMFileClass final : public RAMFileClass
{
public:
	FaultInjectRAMFileClass(void *buffer, int size) :
		RAMFileClass(buffer, size)
	{
		Open(WRITE);
	}

	int Seek(int offset, int direction = SEEK_CUR) override
	{
		++SeekCallCount;
		if (FailSeekCall && (*FailSeekCall == SeekCallCount)) {
			FailSeekCall.reset();
			++FailedSeekCount;
			return -1;
		}
		return RAMFileClass::Seek(offset, direction);
	}

	int Write(void const *buffer, int size) override
	{
		++WriteCallCount;
		if (FailWriteCall && (*FailWriteCall == WriteCallCount)) {
			FailWriteCall.reset();
			++ShortWriteCount;
			return RAMFileClass::Write(buffer, (size > 0) ? size - 1 : 0);
		}
		return RAMFileClass::Write(buffer, size);
	}

	void Fail_Next_Write()
	{
		FailWriteCall = WriteCallCount + 1;
	}

	void Fail_Seek_On_Call(size_t call)
	{
		FailSeekCall = call;
	}

	size_t Get_Seek_Call_Count() const
	{
		return SeekCallCount;
	}

	size_t Get_Short_Write_Count() const
	{
		return ShortWriteCount;
	}

	size_t Get_Failed_Seek_Count() const
	{
		return FailedSeekCount;
	}

private:
	size_t WriteCallCount = 0;
	size_t SeekCallCount = 0;
	size_t ShortWriteCount = 0;
	size_t FailedSeekCount = 0;
	std::optional<size_t> FailWriteCall;
	std::optional<size_t> FailSeekCall;
};

#define CHECK(expression) if (!(expression)) { std::puts("check failed: " #expression); return false; }

bool Test_Golden_Nested_And_Micro_Chunks()
{
	std::array<std::uint8_t, 256> buffer{};
	FaultInjectRAMFileClass file(buffer.data(), static_cast<int>(buffer.size()));
	ChunkSaveClass save(&file);

	CHECK(!save.Has_Write_Error());
	CHECK(save.Begin_Chunk(0x11223344));
	CHECK(save.Begin_Chunk(0x55667788));

	const uint16 micro_value = 0xBEEF;
	WRITE_MICRO_CHUNK(save, 0xAB, micro_value);

	CHECK(save.End_Chunk());
	CHECK(save.End_Chunk());
	CHECK(save.Cur_Chunk_Depth() == 0);
	CHECK(!save.Has_Write_Error());

	const std::vector<std::uint8_t> actual(buffer.begin(), buffer.begin() + file.Size());
	const std::vector<std::uint8_t> expected = {
		0x44, 0x33, 0x22, 0x11,
		0x0C, 0x00, 0x00, 0x80,
		0x88, 0x77, 0x66, 0x55,
		0x04, 0x00, 0x00, 0x00,
		0xAB, 0x02, 0xEF, 0xBE,
	};
	CHECK(actual == expected);
	return true;
}

bool Test_Short_Payload_Write_Remains_Sticky_After_Close()
{
	std::array<std::uint8_t, 256> buffer{};
	FaultInjectRAMFileClass file(buffer.data(), static_cast<int>(buffer.size()));
	ChunkSaveClass save(&file);

	CHECK(save.Begin_Chunk(0x01020304));
	const uint32 payload = 0xAABBCCDD;
	file.Fail_Next_Write();
	CHECK(save.Write(&payload, sizeof(payload)) == 0);
	CHECK(file.Get_Short_Write_Count() == 1);
	CHECK(save.Has_Write_Error());

	// The legacy local result stays true because this header rewrite succeeds.
	CHECK(save.End_Chunk());
	CHECK(save.Cur_Chunk_Depth() == 0);
	CHECK(save.Has_Write_Error());
	return true;
}

bool Test_Begin_Header_Failure_Is_Sticky()
{
	std::array<std::uint8_t, 256> buffer{};
	FaultInjectRAMFileClass file(buffer.data(), static_cast<int>(buffer.size()));
	ChunkSaveClass save(&file);

	file.Fail_Next_Write();
	CHECK(!save.Begin_Chunk(0x01020304));
	CHECK(file.Get_Short_Write_Count() == 1);
	CHECK(save.Has_Write_Error());

	// Preserve upstream behavior: Begin pushes before the placeholder write.
	CHECK(save.Cur_Chunk_Depth() == 1);
	return true;
}

bool Test_End_Header_Rewrite_Failure_Is_Sticky()
{
	std::array<std::uint8_t, 256> buffer{};
	FaultInjectRAMFileClass file(buffer.data(), static_cast<int>(buffer.size()));
	ChunkSaveClass save(&file);

	CHECK(save.Begin_Chunk(0x01020304));
	file.Fail_Next_Write();
	CHECK(!save.End_Chunk());
	CHECK(file.Get_Short_Write_Count() == 1);
	CHECK(save.Has_Write_Error());

	// Preserve upstream behavior: End pops before rewriting the header.
	CHECK(save.Cur_Chunk_Depth() == 0);
	return true;
}

bool Test_Seek_Failure_Is_Sticky_Without_Changing_End_Result()
{
	std::array<std::uint8_t, 256> buffer{};
	FaultInjectRAMFileClass file(buffer.data(), static_cast<int>(buffer.size()));
	ChunkSaveClass save(&file);

	CHECK(save.Begin_Chunk(0x01020304));

	// End first queries the current position, then seeks to the header.
	file.Fail_Seek_On_Call(file.Get_Seek_Call_Count() + 2);
	CHECK(save.End_Chunk());
	CHECK(file.Get_Failed_Seek_Count() == 1);
	CHECK(save.Cur_Chunk_Depth() == 0);
	CHECK(save.Has_Write_Error());
	return true;
}

struct TestCase
{
	const char *Name;
	bool (*Run)();
};
}

int main()
{
	const TestCase tests[] = {
		{"golden nested and micro chunks", Test_Golden_Nested_And_Micro_Chunks},
		{"short payload remains sticky", Test_Short_Payload_Write_Remains_Sticky_After_Close},
		{"begin header failure", Test_Begin_Header_Failure_Is_Sticky},
		{"end header rewrite failure", Test_End_Header_Rewrite_Failure_Is_Sticky},
		{"seek failure", Test_Seek_Failure_Is_Sticky_Without_Changing_End_Result},
	};

	for (const TestCase &test : tests) {
		if (!test.Run()) {
			std::puts("FAILED");
			std::puts(test.Name);
			return 1;
		}
		std::puts(test.Name);
	}
	return 0;
}
