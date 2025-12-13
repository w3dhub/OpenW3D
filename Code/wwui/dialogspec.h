/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 OpenW3D
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

#include <cstdlib>

enum DialogResourceControlType {
    CONTROL_INVALID = -1,
    CONTROL_BUTTON	= 0x0080,
    CONTROL_EDIT,
    CONTROL_STATIC,
    CONTROL_LIST_BOX,
    CONTROL_SCROLL_BAR,
    CONTROL_COMBOBOX,
    CONTROL_SLIDER,
    CONTROL_LIST_CTRL,
    CONTROL_TAB,
    CONTROL_MAP,
    CONTROL_VIEWER,
    CONTROL_HOTKEY,
    CONTROL_SHORTCUT_BAR,
    CONTROL_MERCHANDISE_CTRL,
    CONTROL_TREE_CTRL,
    CONTROL_PROGRESS_BAR,
    CONTROL_HEALTH_BAR
};

typedef struct DialogResourceControl {
    DialogResourceControlType type;
    int x;
    int y;
    int cx;
    int cy;
    int id;
    int style;
    const wchar_t *text;
} DialogResourceControl;

typedef struct DialogResource {
    int id;
    int x;
    int y;
    int cx;
    int cy;
    const wchar_t *caption;
    const DialogResourceControl *controls;
    size_t count_controls;
} DialogResource;
