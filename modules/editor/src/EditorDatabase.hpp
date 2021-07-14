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

/** @file EditorDatabase.hpp */

#ifndef EDITOR_DATABASE_HPP
#define EDITOR_DATABASE_HPP

#include <EditorDataSet.hpp>
#include <FileIndex.hpp>

/** Editor Database. */
class EditorDatabase
{
public:
    EditorDatabase();

    const EditorDataSet &properties() const { return properties_; }
    void setProperties(const EditorDataSet &properties);

    const FileIndex &index() const { return index_; }

protected:
    EditorDataSet properties_;
    FileIndex index_;

    void readIndex(const std::string &pathLas);
};

#endif /* EDITOR_DATABASE_HPP */
