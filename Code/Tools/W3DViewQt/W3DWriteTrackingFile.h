/*
** Command & Conquer Renegade(tm)
** Copyright 2025 Electronic Arts Inc.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
*/

#pragma once

#include "wwfile.h"

class W3DWriteTrackingFile final : public FileClass
{
public:
    explicit W3DWriteTrackingFile(FileClass &file);

    bool Has_Error() const { return error_; }

    const char *File_Name() const override;
    const char *Set_Name(const char *filename) override;
    int Create() override;
    int Delete() override;
    bool Is_Available(int forced = false) override;
    bool Is_Open() const override;
    int Open(const char *filename, int rights = READ) override;
    int Open(int rights = READ) override;
    int Read(void *buffer, int size) override;
    int Seek(int pos, int dir = SEEK_CUR) override;
    int Tell() override;
    int Size() override;
    int Write(const void *buffer, int size) override;
    void Close() override;
    unsigned int Get_Date_Time() override;
    bool Set_Date_Time(unsigned int datetime) override;
    void Error(int error, int canretry = false, const char *filename = nullptr) override;
    HANDLE_TYPE Get_File_Handle() override;
    void Bias(int start, int length = -1) override;

private:
    void Capture_Position(bool record_failure);
    void Advance_Position(int amount);

    FileClass &file_;
    bool error_ = false;
    int position_ = 0;
    bool position_known_ = false;
};
