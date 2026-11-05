/*
** Command & Conquer Renegade(tm)
** Copyright 2026 OpenW3D Contributors.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ww3d.h"

#include "ffactory.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <vector>

namespace
{
class RecordingFileFactory final : public FileFactoryClass
{
public:
    explicit RecordingFileFactory(FileFactoryClass *delegate) : Delegate(delegate)
    {
    }

    FileClass *Get_File(const char *filename) override
    {
        RequestedNames.emplace_back(filename != nullptr ? filename : "");
        return Delegate->Get_File(filename);
    }

    void Return_File(FileClass *file) override
    {
        ++ReturnedFiles;
        Delegate->Return_File(file);
    }

    std::vector<std::string> RequestedNames;
    int ReturnedFiles = 0;

private:
    FileFactoryClass *Delegate;
};

class ScopedFileFactoryOverride final
{
public:
    explicit ScopedFileFactoryOverride(FileFactoryClass *replacement) : Previous(_TheFileFactory)
    {
        _TheFileFactory = replacement;
    }

    ~ScopedFileFactoryOverride()
    {
        _TheFileFactory = Previous;
    }

private:
    FileFactoryClass *Previous;
};
}

int main()
{
    if (_TheFileFactory == nullptr) {
        return 1;
    }

    RecordingFileFactory file_factory(_TheFileFactory);
    ScopedFileFactoryOverride override_file_factory(&file_factory);

    if (WW3D::Make_Back_Buffer_Screen_Shot(nullptr) != 0 ||
        WW3D::Make_Back_Buffer_Screen_Shot("") != 0 ||
        !file_factory.RequestedNames.empty()) {
        return 2;
    }

    const std::string filename_base =
        (std::filesystem::temp_directory_path() /
         ("OpenW3DScreenshotApiTest" +
          std::to_string(reinterpret_cast<std::uintptr_t>(&file_factory)))).string();
    const std::string first_filename = filename_base + "01.tga";
    const std::string second_filename = filename_base + "02.tga";

    std::error_code ignored_error;
    std::filesystem::remove(first_filename, ignored_error);
    std::filesystem::remove(second_filename, ignored_error);
    {
        std::ofstream existing_screenshot(first_filename, std::ios::binary);
        if (!existing_screenshot) {
            return 3;
        }
        existing_screenshot.put('\0');
    }

    // No D3D device is initialized in this test, so capture must fail cleanly after
    // skipping the existing 01 file and choosing the available 02 filename.
    const int capture_result = WW3D::Make_Back_Buffer_Screen_Shot(filename_base.c_str());
    const bool filename_selection_passed =
        capture_result == 0 &&
        file_factory.RequestedNames.size() == 2 &&
        file_factory.RequestedNames[0] == first_filename &&
        file_factory.RequestedNames[1] == second_filename &&
        file_factory.ReturnedFiles == 2;

    std::filesystem::remove(first_filename, ignored_error);
    std::filesystem::remove(second_filename, ignored_error);

    return filename_selection_passed ? 0 : 4;
}
