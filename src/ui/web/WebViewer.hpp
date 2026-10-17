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

/** @file WebViewer.hpp */

#pragma once

// Include 3D Forest.
#include <Box.hpp>
#include <Camera.hpp>
#include <WebWidget.hpp>
class Viewer;

// Include std.
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

// Include local.
#include <ExportUiWeb.hpp>
#include <WarningsDisable.hpp>

/** WebViewer.

    Session-thread adapter. It never owns or retains an editor Page.
*/
class WebViewer final : public WebWidget
{
public:
    WebViewer(WebApplication &application, Viewer *viewer);
    void bind() override;
    void sync(bool enabled, bool visible) override;
    void handleEvent(const JsonCpp::Value &event) override;

    static constexpr std::size_t ChunkPoints = 16384;
    static constexpr std::size_t MaxGpuBytes = 128 * 1024 * 1024;

private:
    Viewer *viewer_;
    Camera camera_;
    Box<double> boundary_;
    bool perspective_{false}, lock2d_{false};
    bool resetBounds_{true}, resetView_{true}, sceneDirty_{true};

    std::vector<int> commands_;
    std::uint32_t cameraRevision_{0}, cameraAck_{0}, cameraClient_{0};
    std::uint64_t sceneRevision_{0};
    std::uint64_t streamRevision_{std::numeric_limits<std::uint64_t>::max()};
    std::uint32_t stream_{0}, request_{0}, epoch_{0};
    bool streaming_{false}, begun_{false};
    std::size_t pageIndex_{0}, pointIndex_{0}, sentPoints_{0};
    std::size_t segmentIndex_{0}, meshIndex_{0}, meshPoint_{0};
    using PageKey = std::pair<std::size_t, std::size_t>;
    std::vector<PageKey> sourceKeys_;
    std::uint64_t sourceContentRevision_{
        std::numeric_limits<std::uint64_t>::max()};
    bool streamComplete_{false}, streamLimited_{false}, stream2d_{false};
    Vector3<double> origin_;

    void direction(const Vector3<double> &direction, const Vector3<double> &up);
    void command(int command);
    JsonCpp::Value cameraJson() const;
    JsonCpp::Value sceneJson(); // Editor mutex must be held.
    void pull(const JsonCpp::Value &event);
    void pick(const JsonCpp::Value &event);
};

#include <WarningsEnable.hpp>
