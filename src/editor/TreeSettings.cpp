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

/** @file TreeSettings.cpp */

// Include 3D Forest.
#include <TreeSettings.hpp>

// Include local.
#define LOG_MODULE_NAME "TreeSettings"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

TreeSettings::TreeSettings()
    : useOnlyForSelectedTrees_(false),
      treeAttributesVisible_(true),
      treePosition_(TreeSettings::Position::BOTTOM),
      convexHullVisible_(true),
      convexHullProjectionVisible_(true),
      concaveHullVisible_(true),
      concaveHullProjectionVisible_(true),
      dbhScale_(5.0)
{
    // empty
}

bool TreeSettings::useOnlyForSelectedTrees() const
{
    return useOnlyForSelectedTrees_;
}

void TreeSettings::setUseOnlyForSelectedTrees(bool on)
{
    useOnlyForSelectedTrees_ = on;
}

bool TreeSettings::treeAttributesVisible() const
{
    return treeAttributesVisible_;
}

void TreeSettings::setTreeAttributesVisible(bool visible)
{
    treeAttributesVisible_ = visible;
}

TreeSettings::Position TreeSettings::treePosition() const
{
    return treePosition_;
}

void TreeSettings::setTreePosition(TreeSettings::Position position)
{
    treePosition_ = position;
}

bool TreeSettings::convexHullVisible() const
{
    return convexHullVisible_;
}

void TreeSettings::setConvexHullVisible(bool visible)
{
    convexHullVisible_ = visible;
}

bool TreeSettings::convexHullProjectionVisible() const
{
    return convexHullProjectionVisible_;
}

void TreeSettings::setConvexHullProjectionVisible(bool visible)
{
    convexHullProjectionVisible_ = visible;
}

bool TreeSettings::concaveHullVisible() const
{
    return concaveHullVisible_;
}

void TreeSettings::setConcaveHullVisible(bool visible)
{
    concaveHullVisible_ = visible;
}

bool TreeSettings::concaveHullProjectionVisible() const
{
    return concaveHullProjectionVisible_;
}

void TreeSettings::setConcaveHullProjectionVisible(bool visible)
{
    concaveHullProjectionVisible_ = visible;
}

double TreeSettings::dbhScale() const
{
    return dbhScale_;
}

void TreeSettings::setDbhScale(double value)
{
    dbhScale_ = value;
}

void fromJson(TreeSettings &out, const Json &in)
{
    fromJson(out.useOnlyForSelectedTrees_, in, "useOnlyForSelectedTrees");
    fromJson(out.treeAttributesVisible_, in, "treeAttributesVisible", true);
    fromJson(out.treePosition_, in, "treePosition");
    fromJson(out.convexHullVisible_, in, "convexHullVisible");
    fromJson(out.convexHullProjectionVisible_,
             in,
             "convexHullProjectionVisible");
    fromJson(out.concaveHullVisible_, in, "concaveHullVisible");
    fromJson(out.concaveHullProjectionVisible_,
             in,
             "concaveHullProjectionVisible");
    fromJson(out.dbhScale_, in, "dbhScale", 5.0);
}

void toJson(Json &out, const TreeSettings &in)
{
    toJson(out["useOnlyForSelectedTrees"], in.useOnlyForSelectedTrees_);
    toJson(out["treeAttributesVisible"], in.treeAttributesVisible_);
    toJson(out["treePosition"], in.treePosition_);
    toJson(out["convexHullVisible"], in.convexHullVisible_);
    toJson(out["convexHullProjectionVisible"], in.convexHullProjectionVisible_);
    toJson(out["concaveHullVisible"], in.concaveHullVisible_);
    toJson(out["concaveHullProjectionVisible"],
           in.concaveHullProjectionVisible_);
    toJson(out["dbhScale"], in.dbhScale_);
}

std::string toString(const TreeSettings &in)
{
    Json json;
    toJson(json, in);
    return json.serialize(0);
}

void fromJson(TreeSettings::Position &out, const Json &in)
{
    std::string tmp;
    fromJson(tmp, in);
    fromString(out, tmp);
}

void fromJson(TreeSettings::Position &out,
              const Json &in,
              const std::string &key,
              TreeSettings::Position defaultValue,
              bool optional)
{
    std::string tmp;
    fromJson(tmp, in, key, "", optional);
    if (tmp.empty())
    {
        out = defaultValue;
    }
    else
    {
        fromString(out, tmp);
    }
}

void toJson(Json &out, const TreeSettings::Position &in)
{
    toJson(out, toString(in));
}

void fromString(TreeSettings::Position &out, const std::string &in)
{
    if (in == "BOTTOM")
    {
        out = TreeSettings::Position::BOTTOM;
    }
    else if (in == "TOP")
    {
        out = TreeSettings::Position::TOP;
    }
    else
    {
        LOG_ERROR(<< "Fix unknown position <" << in << "> to <BOTTOM>.");
        out = TreeSettings::Position::BOTTOM;
    }
}

std::string toString(const TreeSettings::Position &in)
{
    switch (in)
    {
        case TreeSettings::Position::TOP:
            return "TOP";
        case TreeSettings::Position::BOTTOM:
        default:
            return "BOTTOM";
    }
}