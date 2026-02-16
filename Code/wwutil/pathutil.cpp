#include "pathutil.h"

#include <filesystem>

StringClass cPathUtil::GetWorkingDirectory(bool trailing_separator)
{
	std::error_code ec;
	auto path = std::filesystem::current_path(ec);
	if (ec) {
		path = ".";
	}
	if (trailing_separator) {
		path += std::filesystem::path::preferred_separator;
	}
	return StringClass{path.generic_string().c_str()};
}

StringClass cPathUtil::ExtractFilename(const char *c_path)
{
	std::filesystem::path path = c_path;
	return StringClass{path.filename().generic_string().c_str()};
}

bool cPathUtil::PathExists(const char *path)
{
	std::error_code ec;
	return std::filesystem::exists(path, ec) && !ec;
}
