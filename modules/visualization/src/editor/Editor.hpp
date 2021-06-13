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

#include <Camera.hpp>
#include <EditorBase.hpp>
#include <QObject>
#include <ThreadRender.hpp>
#include <mutex>

/** Editor. */
class Editor : public QObject, public EditorBase
{
    Q_OBJECT

public:
    Editor(QObject *parent = nullptr);
    ~Editor();

    void lock();
    void unlock();

    void cancelThreads();
    void restartThreads();

signals:
    void renderRequested();

public slots:
    void render(size_t viewportId, const Camera &camera);
    void render();

protected:
    ThreadRender thread_;
    std::mutex mutex_;
};

#endif /* EDITOR_HPP */
