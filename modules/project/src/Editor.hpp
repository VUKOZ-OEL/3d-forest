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

/**
    @file Editor.hpp
*/

#ifndef EDITOR_HPP
#define EDITOR_HPP

#include <Database.hpp>
#include <Node.hpp>
#include <string>
#include <vector>

/** Editor. */
class Editor
{
public:
    Editor();
    ~Editor();

    void open(const std::string &path);
    void close();

    const std::vector<std::shared_ptr<Node>> &getScene() const
    {
        return nodes_;
    }

protected:
    std::string path_;
    Database db_;
    std::vector<std::shared_ptr<Node>> nodes_;
};

#endif /* EDITOR_HPP */
