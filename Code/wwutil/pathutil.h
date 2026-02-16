#pragma once

#include "wwstring.h"

class cPathUtil
{
public:
	static StringClass GetWorkingDirectory(bool trailing_separator=true);

	static StringClass ExtractFilename(const char *path);

	static bool PathExists(const char *path);
};
