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

/** @file TreeSettings.hpp */

#ifndef TREE_SETTINGS_HPP
#define TREE_SETTINGS_HPP

// Include 3D Forest.
#include <Json.hpp>
#include <Vector3.hpp>

// Include local.
#include <ExportEditor.hpp>
#include <WarningsDisable.hpp>

/** Tree Settings. */
class EXPORT_EDITOR TreeSettings
{
public:
    TreeSettings();

    bool useOnlyForSelectedTrees() const;
    void setUseOnlyForSelectedTrees(bool on);

    bool treeAttributesVisible() const;
    void setTreeAttributesVisible(bool visible);

    enum class Position
    {
        BOTTOM,
        TOP
    };

    TreeSettings::Position treePosition() const;
    void setTreePosition(TreeSettings::Position position);

    bool convexHullVisible() const;
    void setConvexHullVisible(bool visible);

    bool convexHullProjectionVisible() const;
    void setConvexHullProjectionVisible(bool visible);

    bool concaveHullVisible() const;
    void setConcaveHullVisible(bool visible);

    bool concaveHullProjectionVisible() const;
    void setConcaveHullProjectionVisible(bool visible);

    double dbhScale() const;
    void setDbhScale(double value);

protected:
    bool useOnlyForSelectedTrees_;
    bool treeAttributesVisible_;
    TreeSettings::Position treePosition_;
    bool convexHullVisible_;
    bool convexHullProjectionVisible_;
    bool concaveHullVisible_;
    bool concaveHullProjectionVisible_;
    double dbhScale_;

    friend void fromJson(TreeSettings &out, const Json &in);
    friend void toJson(Json &out, const TreeSettings &in);
};

void fromJson(TreeSettings &out, const Json &in);
void toJson(Json &out, const TreeSettings &in);
std::string EXPORT_EDITOR toString(const TreeSettings &in);

void fromJson(TreeSettings::Position &out, const Json &in);
void fromJson(
    TreeSettings::Position &out,
    const Json &in,
    const std::string &key,
    TreeSettings::Position defaultValue = TreeSettings::Position::BOTTOM,
    bool optional = true);
void toJson(Json &out, const TreeSettings::Position &in);
void fromString(TreeSettings::Position &out, const std::string &in);
std::string EXPORT_EDITOR toString(const TreeSettings::Position &in);

#include <WarningsEnable.hpp>

#endif /* TREE_SETTINGS_HPP */
