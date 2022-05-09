/*
    Copyright 2020 VUKOZ

    This file is part of 3D Forest.

    3D Forest is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    3D Forest is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with 3D Forest.  If not, see <https://www.gnu.org/licenses/>.
*/

/** @file EditorProcessorInterface.hpp */

#ifndef EDITOR_PROCESSOR_INTERFACE_HPP
#define EDITOR_PROCESSOR_INTERFACE_HPP

#include <EditorPage.hpp>

/** Editor Processor Interface. */
class EditorProcessorInterface
{
public:
    virtual ~EditorProcessorInterface() = default;
    virtual bool isFilterEnabled() = 0;
    virtual void filterPage(EditorPage *page) = 0;
};

#endif /* EDITOR_PROCESSOR_INTERFACE_HPP */
