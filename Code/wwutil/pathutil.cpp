#include "pathutil.h"

#include <filesystem>

bool cPathUtil::PathExists(const char *path)
{
	std::error_code ec;
	return std::filesystem::exists(path, ec) && !ec;
}
