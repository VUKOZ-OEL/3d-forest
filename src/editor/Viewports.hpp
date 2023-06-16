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

/** @file Viewports.hpp */

#ifndef VIEWPORTS_HPP
#define VIEWPORTS_HPP

#include <Query.hpp>

class Editor;

#include <ExportEditor.hpp>
#include <WarningsDisable.hpp>

/** Viewports. */
class EXPORT_EDITOR Viewports
{
public:
    Viewports();
    ~Viewports();

    size_t size() const { return viewports_.size(); }
    void resize(Editor *editor, size_t n);
    void clearContent();

    QueryWhere &where() { return viewports_[activeViewport_]->where(); }
    const QueryWhere &where() const
    {
        return viewports_[activeViewport_]->where();
    }

    void applyWhereToAll();

    void applyCamera(size_t viewport, const Camera &camera);

    void setState(Page::State state);
    bool nextState();

    size_t pageSize(size_t viewport) const
    {
        return viewports_[viewport]->cacheSize();
    }

    Page &page(size_t viewport, size_t index)
    {
        return viewports_[viewport]->cache(index);
    }

protected:
    std::vector<std::shared_ptr<Query>> viewports_;
    size_t activeViewport_;
};

#include <WarningsEnable.hpp>

#endif /* VIEWPORTS_HPP */
