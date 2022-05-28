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

/** @file ClassificationList.cpp */

#include <ClassificationList.hpp>

ClassificationList::ClassificationList() : enabled_(false)
{
    clear();
}

void ClassificationList::setEnabled(bool b)
{
    enabled_ = b;
}

void ClassificationList::setEnabledAll(bool b)
{
    for (size_t i = 0; i < classes_.size(); i++)
    {
        classes_[i].enabled = b;
    }
}

void ClassificationList::setInvertAll()
{
    for (size_t i = 0; i < classes_.size(); i++)
    {
        classes_[i].enabled = !classes_[i].enabled;
    }
}

void ClassificationList::clear()
{
    classes_.resize(256);

    classes_[0].label = "Never classified";
    classes_[1].label = "Unassigned";
    classes_[2].label = "Ground";
    classes_[3].label = "Low Vegetation";
    classes_[4].label = "Medium Vegetation";
    classes_[5].label = "High Vegetation";
    classes_[6].label = "Building";
    classes_[7].label = "Low Point (Noise)";
    classes_[8].label = "Reserved";
    classes_[9].label = "Water";
    classes_[10].label = "Rail";
    classes_[11].label = "Road Surface";
    classes_[12].label = "Reserved";
    classes_[13].label = "Wire - Guard";
    classes_[14].label = "Wire - Conductor";
    classes_[15].label = "Transmission Tower";
    classes_[16].label = "Wire - Insulator";
    classes_[17].label = "Bridge Deck";
    classes_[18].label = "High Noise";

    classes_[19].label = "Overhead Structure";
    classes_[20].label = "Ignored Ground";
    classes_[21].label = "Snow";
    classes_[22].label = "Temporal Exclusion";

    for (size_t i = 23; i < 64; i++)
    {
        classes_[i].label = "Reserved";
    }

    for (size_t i = 64; i < 256; i++)
    {
        classes_[i].label = "User";
    }

    // Enable
    for (size_t i = 0; i < classes_.size(); i++)
    {
        classes_[i].enabled = false;
    }

    for (size_t i = 2; i < 16; i++)
    {
        classes_[i].enabled = true;
    }
}

void ClassificationList::read(const Json &in)
{
    if (in.contains("enabled"))
    {
        enabled_ = in["enabled"].isTrue();
    }
    else
    {
        enabled_ = true;
    }

    if (in.contains("classes"))
    {
        size_t i = 0;
        size_t n = in["classes"].array().size();

        clear();
        classes_.resize(n);

        for (auto const &it : in["classes"].array())
        {
            if (it.contains("label"))
            {
                classes_[i].label = it["label"].string();
            }

            if (it.contains("enabled"))
            {
                classes_[i].enabled = it["enabled"].isTrue();
            }
            else
            {
                classes_[i].enabled = true;
            }

            i++;
        }
    }
}

Json &ClassificationList::write(Json &out) const
{
    out["enabled"] = enabled_;

    size_t i = 0;

    for (auto const &it : classes_)
    {
        Json &obj = out["classes"][i];
        obj["label"] = it.label;
        obj["enabled"] = it.enabled;

        i++;
    }

    return out;
}
