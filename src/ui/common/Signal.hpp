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

/** @file Signal.hpp */

#ifndef SIGNAL_HPP
#define SIGNAL_HPP

// Include std.
#include <functional>
#include <memory>
#include <vector>

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** Signal. */
template <typename... Args> class Signal
{
public:
    using Slot = std::function<void(Args...)>;

    void connect(Slot slot)
    {
        slots_.push_back(std::make_shared<Slot>(std::move(slot)));
    }

    void operator()(Args... args)
    {
        // A callback may connect another slot or destroy the signal owner.
        // Iterate a stable copy instead of references into the owner's vector.
        const auto slots = slots_;
        for (const auto &slot : slots)
        {
            (*slot)(args...);
        }
    }

private:
    std::vector<std::shared_ptr<Slot>> slots_;
};

#include <WarningsEnable.hpp>

#endif /* SIGNAL_HPP */
