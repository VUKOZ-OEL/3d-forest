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

/** @file Message.hpp */

#ifndef MESSAGE_HPP
#define MESSAGE_HPP

// Include std.
#include <iostream>
#include <set>
#include <string>
#include <vector>

// Include 3D Forest.

// Include local.
#include <ExportEditor.hpp>
#include <WarningsDisable.hpp>

/** Message. */
class EXPORT_EDITOR Message
{
public:
    /** Message Type. */
    enum Type
    {
        TYPE_NONE = 0,
        TYPE_FILTER = 1U << 0,
        TYPE_CLASSIFICATION = 1U << 1,
        TYPE_CLIP_FILTER = 1U << 2,
        TYPE_DATA_SET = 1U << 3,
        TYPE_DESCRIPTOR = 1U << 4,
        TYPE_ELEVATION = 1U << 5,
        TYPE_INTENSITY = 1U << 6,
        TYPE_SEGMENT = 1U << 7,
        TYPE_SPECIES = 1U << 8,
        TYPE_MANAGEMENT_STATUS = 1U << 9,
        TYPE_PROJECT_NAME = 1U << 10,
        TYPE_SETTINGS = 1U << 11
    };

    Message();
    Message(void *sender, int type);

    void *sender() const { return sender_; }
    bool empty() const { return type_ == 0; }
    bool contains(int t) const { return type_ & t; }

private:
    void *sender_{nullptr};
    int type_{0};
};

inline EXPORT_EDITOR std::ostream &operator<<(std::ostream &out,
                                              const Message::Type &in)
{
    switch (in)
    {
        case Message::TYPE_CLASSIFICATION:
            out << "CLASSIFICATION";
            break;
        case Message::TYPE_CLIP_FILTER:
            out << "CLIP_FILTER";
            break;
        case Message::TYPE_DATA_SET:
            out << "DATA_SET";
            break;
        case Message::TYPE_DESCRIPTOR:
            out << "DESCRIPTOR";
            break;
        case Message::TYPE_ELEVATION:
            out << "ELEVATION";
            break;
        case Message::TYPE_INTENSITY:
            out << "INTENSITY";
            break;
        case Message::TYPE_SEGMENT:
            out << "SEGMENT";
            break;
        case Message::TYPE_PROJECT_NAME:
            out << "PROJECT_NAME";
            break;
        case Message::TYPE_SETTINGS:
            out << "SETTINGS";
            break;
        default:
            out << "UNKNOWN";
            break;
    }

    return out;
}

#include <WarningsEnable.hpp>

#endif /* MESSAGE_HPP */
