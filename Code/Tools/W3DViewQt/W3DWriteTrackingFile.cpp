/*
** Command & Conquer Renegade(tm)
** Copyright 2025 Electronic Arts Inc.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
*/

#include "W3DWriteTrackingFile.h"

#include <limits>

W3DWriteTrackingFile::W3DWriteTrackingFile(FileClass &file) : file_(file)
{
    if (file_.Is_Open()) {
        Capture_Position(true);
    }
}

const char *W3DWriteTrackingFile::File_Name() const
{
    return file_.File_Name();
}

const char *W3DWriteTrackingFile::Set_Name(const char *filename)
{
    const char *result = file_.Set_Name(filename);
    position_known_ = false;
    return result;
}

int W3DWriteTrackingFile::Create()
{
    const int result = file_.Create();
    position_known_ = false;
    return result;
}

int W3DWriteTrackingFile::Delete()
{
    const int result = file_.Delete();
    position_known_ = false;
    return result;
}

bool W3DWriteTrackingFile::Is_Available(int forced)
{
    return file_.Is_Available(forced);
}

bool W3DWriteTrackingFile::Is_Open() const
{
    return file_.Is_Open();
}

int W3DWriteTrackingFile::Open(const char *filename, int rights)
{
    const int result = file_.Open(filename, rights);
    if (result) {
        Capture_Position(true);
    } else {
        position_known_ = false;
    }
    return result;
}

int W3DWriteTrackingFile::Open(int rights)
{
    const int result = file_.Open(rights);
    if (result) {
        Capture_Position(true);
    } else {
        position_known_ = false;
    }
    return result;
}

int W3DWriteTrackingFile::Read(void *buffer, int size)
{
    const int result = file_.Read(buffer, size);
    if (result >= 0) {
        Advance_Position(result);
    } else {
        position_known_ = false;
    }
    return result;
}

int W3DWriteTrackingFile::Seek(int pos, int dir)
{
    bool expected_known = false;
    long long expected = 0;
    if (dir == SEEK_SET) {
        expected = pos;
        expected_known = true;
    } else if (dir == SEEK_CUR && position_known_) {
        expected = static_cast<long long>(position_) + pos;
        expected_known = true;
    }

    if (expected_known &&
        (expected < std::numeric_limits<int>::min() ||
         expected > std::numeric_limits<int>::max())) {
        error_ = true;
        expected_known = false;
    }

    const int result = file_.Seek(pos, dir);
    if (result < 0 || (expected_known && result != static_cast<int>(expected))) {
        error_ = true;
    }

    position_known_ = result >= 0;
    if (position_known_) {
        position_ = result;
    }
    return result;
}

int W3DWriteTrackingFile::Tell()
{
    return Seek(0, SEEK_CUR);
}

int W3DWriteTrackingFile::Size()
{
    return file_.Size();
}

int W3DWriteTrackingFile::Write(const void *buffer, int size)
{
    const int result = file_.Write(buffer, size);
    if (result != size) {
        error_ = true;
    }
    if (result >= 0) {
        Advance_Position(result);
    } else {
        position_known_ = false;
    }
    return result;
}

void W3DWriteTrackingFile::Close()
{
    file_.Close();
    position_known_ = false;
}

unsigned int W3DWriteTrackingFile::Get_Date_Time()
{
    return file_.Get_Date_Time();
}

bool W3DWriteTrackingFile::Set_Date_Time(unsigned int datetime)
{
    return file_.Set_Date_Time(datetime);
}

void W3DWriteTrackingFile::Error(int error, int canretry, const char *filename)
{
    file_.Error(error, canretry, filename);
}

HANDLE_TYPE W3DWriteTrackingFile::Get_File_Handle()
{
    return file_.Get_File_Handle();
}

void W3DWriteTrackingFile::Bias(int start, int length)
{
    file_.Bias(start, length);
    if (file_.Is_Open()) {
        Capture_Position(true);
    } else {
        position_known_ = false;
    }
}

void W3DWriteTrackingFile::Capture_Position(bool record_failure)
{
    const int position = file_.Tell();
    position_known_ = position >= 0;
    if (position_known_) {
        position_ = position;
    } else if (record_failure) {
        error_ = true;
    }
}

void W3DWriteTrackingFile::Advance_Position(int amount)
{
    if (!position_known_) {
        return;
    }

    const long long next = static_cast<long long>(position_) + amount;
    if (next < std::numeric_limits<int>::min() ||
        next > std::numeric_limits<int>::max()) {
        error_ = true;
        position_known_ = false;
        return;
    }
    position_ = static_cast<int>(next);
}
