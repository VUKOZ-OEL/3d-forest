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

/** @file WebViewer.cpp */

// Include 3D Forest.
#include <Editor.hpp>
#include <Time.hpp>
#include <Viewer.hpp>
#include <WebApplication.hpp>
#include <WebViewer.hpp>

// Include std.
#include <algorithm>
#include <cmath>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <unordered_set>

// Include local.
#define LOG_MODULE_NAME "WebViewer"
#include <Log.hpp>

namespace
{
using J = JsonCpp::Value;
J vectorJson(const Vector3<double> &v)
{
    J a(JsonCpp::arrayValue);
    for (int i = 0; i < 3; ++i)
        a.append(v[i]);
    return a;
}
J boxJson(const Box<double> &box)
{
    J a(JsonCpp::arrayValue);
    if (!box.empty())
    {
        for (int i = 0; i < 3; ++i)
            a.append(box.min(i));
        for (int i = 0; i < 3; ++i)
            a.append(box.max(i));
    }
    return a;
}
Vector3<double> vectorValue(const J &v)
{
    if (!v.isArray() || v.size() != 3)
        throw std::invalid_argument("Invalid camera vector");
    Vector3<double> r;
    for (int i = 0; i < 3; ++i)
    {
        if (!v[i].isNumeric() || !std::isfinite(v[i].asDouble()) ||
            std::abs(v[i].asDouble()) > 1e15)
            throw std::invalid_argument("Invalid camera coordinate");
        r[i] = v[i].asDouble();
    }
    return r;
}
std::uint32_t uintValue(const J &v)
{
    if (!v.isUInt())
        throw std::invalid_argument("Invalid viewer sequence");
    return v.asUInt();
}
void u32(std::string &out, std::size_t at, std::uint32_t v)
{
    for (unsigned i = 0; i < 4; ++i)
        out[at + i] = char(v >> (i * 8));
}
void f32(std::string &out, std::size_t at, float v)
{
    std::uint32_t bits;
    static_assert(sizeof(v) == sizeof(bits), "Float32 required");
    std::memcpy(&bits, &v, sizeof(v));
    u32(out, at, bits);
}
void f64(std::string &out, std::size_t at, double v)
{
    std::uint64_t bits;
    static_assert(sizeof(v) == sizeof(bits), "Float64 required");
    std::memcpy(&bits, &v, sizeof(v));
    for (unsigned i = 0; i < 8; ++i)
        out[at + i] = char(bits >> (i * 8));
}
bool filtered(Editor &e, const Segment &s)
{
    if (!s.id || !e.segmentsFilter().enabled(s.id) ||
        !e.speciesFilter().enabled(s.speciesId) ||
        !e.managementStatusFilter().enabled(s.managementStatusId))
        return true;
    const auto &p = s.treeAttributes.position;
    return s.treeAttributes.isPositionValid() &&
           e.clipFilter().shape == Region::Shape::BOX &&
           !e.clipFilter().box.contains(p[0], p[1], p[2]);
}
} // namespace
WebViewer::WebViewer(WebApplication &app, Viewer *viewer)
    : WebWidget(app, viewer, "Viewer"),
      viewer_(viewer)
{
    camera_.center.set(0, 0, 0);
    camera_.eye.set(0, 0, 1);
    direction(Vector3<double>(-1, -1, -1), Vector3<double>(-1, -1, 1));
    viewer_->setCamera(camera_);
}
void WebViewer::bind()
{
    WebWidget::bind();
    const std::weak_ptr<WebWidget> weak = shared_from_this();
    auto use = [weak](auto fn)
    {
        if (auto base = weak.lock(); base && base->alive())
            fn(static_cast<WebViewer &>(*base));
    };
    // Signals can arrive while Application holds editorMutex_. Defer all reads.
    // updateRequested wakes the session; renderRevision tracks actual scene
    // changes.
    viewer_->resetRequested.connect(
        [use]
        { use([](WebViewer &w) { w.resetBounds_ = w.sceneDirty_ = true; }); });
    viewer_->resetViewRequested.connect(
        [use]
        {
            use([](WebViewer &w)
                { w.resetView_ = w.resetBounds_ = w.sceneDirty_ = true; });
        });
    viewer_->command.connect(
        [use](int c)
        {
            use(
                [c](WebViewer &w)
                {
                    if (w.commands_.size() < 64)
                        w.commands_.push_back(c);
                });
        });
}
void WebViewer::direction(const Vector3<double> &dir, const Vector3<double> &up)
{
    double distance = (camera_.eye - camera_.center).length();
    if (!(distance > 1e-6))
        distance = 1;
    const auto forward = dir.normalized();
    const auto right = forward.crossProduct(up).normalized();
    camera_.up = right.crossProduct(forward).normalized();
    camera_.eye = camera_.center - forward * distance;
}
void WebViewer::command(int c)
{
    auto offset = camera_.eye - camera_.center;
    if (c == Viewer::ViewOrthographic || c == Viewer::ViewPerspective)
    {
        perspective_ = c == Viewer::ViewPerspective;
        bool was2d = lock2d_;
        lock2d_ = false;
        if (was2d)
            direction(Vector3<double>(-1, -1, -1), Vector3<double>(-1, -1, 1));
    }
    else if (c == Viewer::View2d)
    {
        perspective_ = false;
        lock2d_ = true;
        direction(Vector3<double>(0, 0, -1), Vector3<double>(0, -1, 0));
    }
    else if (c == Viewer::ViewResetCenter)
    {
        if (!boundary_.empty())
            camera_.center = boundary_.center();
        camera_.eye = camera_.center + offset;
    }
    else if (c == Viewer::ViewResetDistance)
    {
        double distance = boundary_.empty() ? 1 : boundary_.radius() * 2;
        if (!(distance > 1e-6))
            distance = 1;
        camera_.eye = camera_.center + offset.normalized() * distance;
    }
    else if (!lock2d_)
    {
        if (c == Viewer::ViewTop)
            direction(Vector3<double>(0, 0, -1), Vector3<double>(0, -1, 0));
        if (c == Viewer::ViewFront)
            direction(Vector3<double>(0, -1, 0), Vector3<double>(0, 0, 1));
        if (c == Viewer::ViewRight)
            direction(Vector3<double>(1, 0, 0), Vector3<double>(0, 0, 1));
        if (c == Viewer::View3d)
            direction(Vector3<double>(-1, -1, -1), Vector3<double>(-1, -1, 1));
    }
}
J WebViewer::cameraJson() const
{
    J c(JsonCpp::objectValue);
    c["eye"] = vectorJson(camera_.eye);
    c["center"] = vectorJson(camera_.center);
    c["up"] = vectorJson(camera_.up);
    c["fov"] = camera_.fov;
    c["perspective"] = perspective_;
    c["lock2d"] = lock2d_;
    c["revision"] = cameraRevision_;
    c["ack"] = cameraAck_;
    c["client"] = cameraClient_;
    return c;
}
J WebViewer::sceneJson()
{
    auto &editor = application_.editor();
    const auto &view = editor.settings().viewSettings();
    const auto &tree = editor.settings().treeSettings();
    J result(JsonCpp::objectValue);
    result["bounds"] = boxJson(editor.datasets().boundary());
    result["background"] = vectorJson(view.backgroundColor());
    result["pointSize"] = view.pointSize();
    result["fog"] = view.distanceBasedFadingVisible();
    result["box"] = view.sceneBoundingBoxVisible();
    result["clip"] = editor.clipFilter().shape == Region::Shape::BOX
                         ? boxJson(editor.clipFilter().box)
                         : J(JsonCpp::arrayValue);
    result["segments"] = J(JsonCpp::arrayValue);
    result["overlayLimited"] = false;
    // Bound JSON overlay metadata independently from the binary point budget.
    for (std::size_t i = 0; i < editor.segments().size(); ++i)
    {
        const auto &s = editor.segments()[i];
        if (filtered(editor, s) || s.boundary.empty())
            continue;
        if (result["segments"].size() >= 4096)
        {
            result["overlayLimited"] = true;
            break;
        }
        const auto &a = s.treeAttributes;
        J item(JsonCpp::objectValue);
        item["id"] = std::to_string(s.id);
        item["label"] = s.label.substr(0, 256);
        item["bounds"] = boxJson(s.boundary);
        item["selected"] = s.selected;
        item["attributes"] = tree.treeAttributesVisible() &&
                             (!tree.useOnlyForSelectedTrees() || s.selected);
        item["color"] = vectorJson(editor.segmentColor(s));
        item["dbhValid"] = a.isDbhValid();
        item["dbhPosition"] =
            vectorJson(a.isDbhValid()        ? a.dbhPosition
                       : a.isPositionValid() ? a.position
                                             : s.boundary.center());
        item["diameter"] =
            (a.isDbhValid()
                 ? a.dbh
                 : .2 * editor.settings().unitsSettings().pointsPerMeter()[0]) *
            tree.dbhScale();
        item["position"] = a.isPositionValid() ? vectorJson(a.position)
                                               : J(JsonCpp::arrayValue);
        item["height"] = a.isHeightValid() ? a.height : 0;
        item["crownStart"] = a.crownStartHeight;
        item["positionTop"] =
            tree.treePosition() == TreeSettings::Position::TOP;
        result["segments"].append(item);
    }
    return result;
}
void WebViewer::sync(bool enabled, bool visible)
{
    WebWidget::sync(enabled, visible);
    bool changed = false;
    J p(JsonCpp::objectValue);
    auto &editor = application_.editor();
    {
        std::unique_lock<std::mutex> lock(editor.editorMutex_);
        if (resetBounds_)
        {
            boundary_ = editor.datasets().boundary();
            resetBounds_ = false;
        }
        if (resetView_)
        {
            command(Viewer::ViewResetCenter);
            command(Viewer::ViewResetDistance);
            resetView_ = false;
            changed = true;
        }
        if (!commands_.empty())
        {
            for (int c : commands_)
                command(c);
            commands_.clear();
            changed = true;
        }
        const auto revision = editor.viewports().renderRevision();
        if (sceneDirty_ || sceneRevision_ != revision)
        {
            p["scene"] = sceneJson();
            sceneDirty_ = false;
            sceneRevision_ = revision;
        }
        p["renderRevision"] = JsonCpp::UInt64(revision);
    }
    if (changed)
    {
        ++cameraRevision_;
        camera_.timeUpdated = Time::realTime();
        viewer_->setCamera(camera_);
        streamRevision_ = std::numeric_limits<std::uint64_t>::max();
        // Never wait for/cancel the worker while holding editorMutex_.
        application_.slotRenderViewport(camera_.viewportId);
    }
    p["camera"] = cameraJson();
    p["maxGpuBytes"] = JsonCpp::UInt64(application_.viewerMemoryBudget());
    p["chunkPoints"] = JsonCpp::UInt(ChunkPoints);
    patch(p);
}
void WebViewer::handleEvent(const J &e)
{
    auto name = e["event"].asString();
    if (name == "viewerPull")
    {
        pull(e);
        return;
    }
    if (name == "viewerPick")
    {
        pick(e);
        return;
    }
    if (name != "viewerCamera")
        throw std::invalid_argument("Unknown viewer event");
    const auto &v = e["camera"];
    Camera next;
    next.eye = vectorValue(v["eye"]);
    next.center = vectorValue(v["center"]);
    next.up = vectorValue(v["up"]);
    const auto delta = next.center - next.eye;
    if (delta.length() < 1e-6 || delta.length() > 1e12 ||
        next.up.length() < 1e-6 ||
        delta.normalized().crossProduct(next.up.normalized()).length() < 1e-6 ||
        !v["perspective"].isBool() || !v["lock2d"].isBool())
        throw std::invalid_argument("Degenerate camera");
    const auto client = uintValue(e["client"]), serial = uintValue(e["serial"]);
    if (!client || !serial)
        throw std::invalid_argument("Invalid camera sequence");
    const auto revision = uintValue(e["revision"]);
    if (client == cameraClient_ && serial <= cameraAck_)
        return;
    if (revision != cameraRevision_)
    {
        // A toolbar/reset command superseded this in-flight browser camera.
        cameraAck_ = serial;
        cameraClient_ = client;
        return;
    }
    next.up.normalize();
    next.fov = 60;
    next.viewportId = camera_.viewportId;
    next.timeUpdated = Time::realTime();
    camera_ = next;
    perspective_ = v["perspective"].asBool();
    lock2d_ = v["lock2d"].asBool();
    cameraAck_ = serial;
    cameraClient_ = client;
    viewer_->setCamera(camera_);
    application_.slotRenderViewport(camera_.viewportId);
}
void WebViewer::pull(const J &e)
{
    const auto stream = uintValue(e["stream"]),
               request = uintValue(e["request"]), ack = uintValue(e["ack"]);
    if (!e["restart"].isBool() || !stream || !request)
        throw std::invalid_argument("Invalid viewer pull");
    if (e["restart"].asBool())
    {
        if (request != 1 || ack != 0)
            throw std::invalid_argument("Invalid viewer restart");
        sourceContentRevision_ = std::numeric_limits<std::uint64_t>::max();
        stream_ = stream;
        request_ = 0;
        streaming_ = true;
        streamRevision_ = std::numeric_limits<std::uint64_t>::max();
    }
    if (!streaming_ || stream != stream_ || request != request_ + 1 ||
        ack != request_)
        throw std::invalid_argument(
            "Viewer packet must be acknowledged before pulling another");
    std::string packet(64 + ChunkPoints * 16, '\0');
    std::size_t count = 0;
    std::uint32_t flags = 0, primitive = 0, options = 0;
    auto &editor = application_.editor();
    {
        std::unique_lock<std::mutex> lock(editor.editorMutex_);
        auto &vp = editor.viewports();
        const auto revision = vp.renderRevision();
        if (streamRevision_ != revision)
        {
            std::vector<PageKey> keys;
            const auto pages = camera_.viewportId < vp.size()
                                   ? vp.pageSize(camera_.viewportId)
                                   : 0;
            const bool bounded = pages <= 65536;
            if (bounded)
                for (std::size_t i = 0; i < pages; ++i)
                {
                    auto &page = vp.page(camera_.viewportId, i);
                    keys.emplace_back(page.datasetId(), page.pageId());
                }
            bool retain = false;
            if (bounded && sourceContentRevision_ == vp.contentRevision() &&
                stream2d_ == lock2d_)
            {
                retain = keys == sourceKeys_;
                if (!retain && streamComplete_ && !streamLimited_ &&
                    keys.size() == sourceKeys_.size())
                {
                    auto previous = sourceKeys_, next = keys;
                    std::sort(previous.begin(), previous.end());
                    std::sort(next.begin(), next.end());
                    retain = previous == next;
                }
            }
            sourceKeys_ = std::move(keys);
            sourceContentRevision_ = vp.contentRevision();
            stream2d_ = lock2d_;
            streamRevision_ = revision;
            if (!retain)
            {
                ++epoch_;
                begun_ = false;
                pageIndex_ = pointIndex_ = sentPoints_ = 0;
                segmentIndex_ = meshIndex_ = meshPoint_ = 0;
                streamComplete_ = streamLimited_ = false;
            }
        }
        if (!begun_)
        {
            flags |= 1;
            begun_ = true;
        }
        origin_ =
            boundary_.empty() ? Vector3<double>(0, 0, 0) : boundary_.center();
        const std::size_t pages = camera_.viewportId < vp.size()
                                      ? vp.pageSize(camera_.viewportId)
                                      : 0;
        // Sequential LRU order is already coarse-to-fine and near-to-far.
        // Only copy one chunk under the editor lock. No Page pointer escapes.
        while (pageIndex_ < pages && !lock2d_)
        {
            Page &page = vp.page(camera_.viewportId, pageIndex_);
            if (page.state() < Page::STATE_RENDER)
                break;
            const auto n =
                (std::min)(page.selectionSize, page.selection.size());
            if (pointIndex_ >= n)
            {
                page.setState(Page::STATE_RENDERED);
                ++pageIndex_;
                pointIndex_ = 0;
                continue;
            }
            if (sentPoints_ >= application_.viewerMemoryBudget() / 16)
            {
                flags |= 2 | 4;
                break;
            }
            while (pointIndex_ < n && count < ChunkPoints &&
                   sentPoints_ < application_.viewerMemoryBudget() / 16)
            {
                const std::size_t index = page.selection[pointIndex_++];
                if (index >= page.size() || !page.position)
                    continue;
                const double *xyz = page.position + 3 * index;
                if (!std::isfinite(xyz[0]) || !std::isfinite(xyz[1]) ||
                    !std::isfinite(xyz[2]))
                    continue;
                if (!count)
                    origin_.set(xyz[0], xyz[1], xyz[2]);
                const auto at = 64 + count * 16;
                for (int c = 0; c < 3; ++c)
                {
                    f32(packet,
                        at + c * 4,
                        static_cast<float>(xyz[c] - origin_[c]));
                    double color = page.renderColor.size() > 3 * index + c
                                       ? page.renderColor[3 * index + c]
                                       : 1;
                    if (!std::isfinite(color))
                        color = 1;
                    packet[at + 12 + c] =
                        char(std::lround(std::clamp(color, 0.0, 1.0) * 255));
                }
                packet[at + 15] = char(255);
                ++count;
                ++sentPoints_;
            }
            if (pointIndex_ >= n)
            {
                page.setState(Page::STATE_RENDERED);
                ++pageIndex_;
                pointIndex_ = 0;
            }
            break; // Each chunk uses its own double-precision origin.
        }
        const bool pointsDone = pageIndex_ >= pages || lock2d_;
        const auto &tree = editor.settings().treeSettings();
        if (!count && pointsDone && !(flags & 4))
        {
            while (segmentIndex_ < editor.segments().size())
            {
                const auto &segment = editor.segments()[segmentIndex_];
                if (filtered(editor, segment) ||
                    !tree.treeAttributesVisible() ||
                    (tree.useOnlyForSelectedTrees() && !segment.selected))
                {
                    ++segmentIndex_;
                    meshIndex_ = meshPoint_ = 0;
                    continue;
                }
                if (meshIndex_ >= segment.meshList.size())
                {
                    ++segmentIndex_;
                    meshIndex_ = meshPoint_ = 0;
                    continue;
                }
                auto it = segment.meshList.begin();
                std::advance(it, meshIndex_);
                const auto &name = it->first;
                const auto &mesh = it->second;
                const bool draw = name == "qsm" ||
                                  (name == "convexHullProjection" &&
                                   tree.convexHullProjectionVisible()) ||
                                  (name == "concaveHullProjection" &&
                                   tree.concaveHullProjectionVisible()) ||
                                  (!lock2d_ && name == "convexHull" &&
                                   tree.convexHullVisible()) ||
                                  (!lock2d_ && name == "concaveHull" &&
                                   tree.concaveHullVisible());
                if (!draw || mesh.mode == Mesh::Mode::MODE_UNKNOWN)
                {
                    ++meshIndex_;
                    meshPoint_ = 0;
                    continue;
                }
                primitive = mesh.mode == Mesh::Mode::MODE_TRIANGLES ? 2
                            : mesh.mode == Mesh::Mode::MODE_LINES   ? 1
                                                                    : 0;
                const std::size_t group = primitive == 2   ? 3
                                          : primitive == 1 ? 2
                                                           : 1;
                const auto vertexCount = mesh.position.size() / 3;
                const auto total =
                    (mesh.indices.empty() ? vertexCount : mesh.indices.size()) /
                    group * group;
                options = name == "qsm" ? 0 : 1;
                while (meshPoint_ < total && count + group <= ChunkPoints)
                {
                    if (sentPoints_ + group >
                        application_.viewerMemoryBudget() / 16)
                    {
                        flags |= 2 | 4;
                        break;
                    }
                    bool valid = true;
                    for (std::size_t k = 0; k < group; ++k)
                    {
                        const auto index = mesh.indices.empty()
                                               ? meshPoint_ + k
                                               : mesh.indices[meshPoint_ + k];
                        if (index >= vertexCount)
                        {
                            valid = false;
                            break;
                        }
                        for (int c = 0; c < 3; ++c)
                            if (!std::isfinite(mesh.position[3 * index + c]))
                                valid = false;
                    }
                    if (!valid)
                    {
                        meshPoint_ += group;
                        continue;
                    }
                    for (std::size_t k = 0; k < group; ++k)
                    {
                        const auto index = mesh.indices.empty()
                                               ? meshPoint_
                                               : mesh.indices[meshPoint_];
                        const auto *xyz = mesh.position.data() + 3 * index;
                        if (!count)
                            origin_.set(xyz[0], xyz[1], xyz[2]);
                        const auto at = 64 + count * 16;
                        for (int c = 0; c < 3; ++c)
                        {
                            f32(packet,
                                at + c * 4,
                                static_cast<float>(xyz[c] - origin_[c]));
                            double color = mesh.color.size() > 3 * index + c
                                               ? mesh.color[3 * index + c]
                                               : segment.color[c];
                            if (!std::isfinite(color))
                                color = 1;
                            packet[at + 12 + c] = char(
                                std::lround(std::clamp(color, 0.0, 1.0) * 255));
                        }
                        packet[at + 15] = char(options ? 64 : 255);
                        ++count;
                        ++sentPoints_;
                        ++meshPoint_;
                    }
                }
                if (meshPoint_ >= total)
                {
                    ++meshIndex_;
                    meshPoint_ = 0;
                }
                if (count || (flags & 4))
                    break;
            }
        }
        if (pointsDone && segmentIndex_ >= editor.segments().size())
            flags |= 2;
        if (sentPoints_ >= application_.viewerMemoryBudget() / 16)
            flags |= 2 | 4;
    }
    streamComplete_ = (flags & 2) != 0;
    streamLimited_ = (flags & 4) != 0;
    packet.resize(64 + count * 16);
    u32(packet, 0, 0x33564657);
    u32(packet, 4, 1);
    u32(packet, 8, static_cast<std::uint32_t>(std::stoul(id_.substr(1))));
    u32(packet, 12, stream);
    u32(packet, 16, request);
    u32(packet, 20, epoch_);
    u32(packet, 24, flags);
    u32(packet, 28, static_cast<std::uint32_t>(count));
    for (int c = 0; c < 3; ++c)
        f64(packet, 32 + c * 8, origin_[c]);
    u32(packet, 56, primitive);
    u32(packet, 60, options);
    request_ = request;
    application_.sendViewerPacket(packet);
}
void WebViewer::pick(const J &e)
{
    auto origin = vectorValue(e["origin"]), dir = vectorValue(e["direction"]);
    if (dir.length() < 1e-6 || !e["ctrl"].isBool())
        throw std::invalid_argument("Invalid picking ray");
    dir.normalize();
    auto &editor = application_.editor();
    application_.suspendThreads();
    bool changed = false;
    {
        std::unique_lock<std::mutex> lock(editor.editorMutex_);
        std::size_t id = 0;
        double nearest = std::numeric_limits<double>::max();
        for (std::size_t i = 0; i < editor.segments().size(); ++i)
        {
            const auto &s = editor.segments()[i];
            if (filtered(editor, s) || s.boundary.empty())
                continue;
            double lo = 0, hi = nearest;
            if (lock2d_)
            {
                const auto &a = s.treeAttributes;
                const auto p = a.isDbhValid()        ? a.dbhPosition
                               : a.isPositionValid() ? a.position
                                                     : s.boundary.center();
                double radius =
                    (a.isDbhValid() ? a.dbh
                                    : .2 * editor.settings()
                                               .unitsSettings()
                                               .pointsPerMeter()[0]) *
                    editor.settings().treeSettings().dbhScale() * .5;
                auto delta = p - origin;
                double t =
                    delta[0] * dir[0] + delta[1] * dir[1] + delta[2] * dir[2];
                if (t < 0 || (delta - dir * t).length() > radius)
                    continue;
                lo = t;
            }
            else
            {
                bool hit = true;
                for (int c = 0; c < 3; ++c)
                {
                    if (std::abs(dir[c]) < 1e-12)
                    {
                        if (origin[c] < s.boundary.min(c) ||
                            origin[c] > s.boundary.max(c))
                            hit = false;
                        continue;
                    }
                    double a = (s.boundary.min(c) - origin[c]) / dir[c],
                           b = (s.boundary.max(c) - origin[c]) / dir[c];
                    if (a > b)
                        std::swap(a, b);
                    lo = (std::max)(lo, a);
                    hi = (std::min)(hi, b);
                }
                if (!hit || hi < lo)
                    continue;
            }
            if (lo < nearest)
            {
                nearest = lo;
                id = s.id;
            }
        }
        auto segments = editor.segments();
        std::unordered_set<std::size_t> ids;
        if (id)
            ids.insert(id);
        changed = segments.updateSelection(ids, e["ctrl"].asBool());
        if (changed)
            editor.setSegments(segments);
    }
    if (changed)
        application_.update(this, Message::TYPE_SEGMENT, Page::STATE_RENDER);
    else
        application_.resumeThreads();
    sceneDirty_ = true;
}
