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

/** @file EditorViewports.hpp */

#ifndef EDITOR_VIEWPORTS_HPP
#define EDITOR_VIEWPORTS_HPP

#include <EditorQuery.hpp>

class EditorDatabase;

/** Editor Viewports. */
class EditorViewports
{
public:
    EditorViewports();
    ~EditorViewports();

    void resize(EditorDatabase *database, size_t n);
    void clearContent();

    void selectBox(const Box<double> &box);
    void selectCamera(size_t viewport, const Camera &camera);

    void setState(EditorPage::State state);
    bool nextState();

    size_t pageSize(size_t viewport) const
    {
        return viewports_[viewport]->cacheSize();
    }

    EditorPage &page(size_t viewport, size_t index)
    {
        return viewports_[viewport]->cache(index);
    }

protected:
    std::vector<std::shared_ptr<EditorQuery>> viewports_;
};

#endif /* EDITOR_VIEWPORTS_HPP */
