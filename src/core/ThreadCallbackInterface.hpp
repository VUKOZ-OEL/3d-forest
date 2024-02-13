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

/** @file ThreadCallbackInterface.hpp */

#ifndef THREAD_CALLBACK_INTERFACE_HPP
#define THREAD_CALLBACK_INTERFACE_HPP

// Include local.
#include <ExportCore.hpp>
#include <WarningsDisable.hpp>

/** Thread Callback Interface. */
class EXPORT_CORE ThreadCallbackInterface
{
public:
    virtual ~ThreadCallbackInterface() = default;
    virtual void threadProgress(bool finished) = 0;
};

#include <WarningsEnable.hpp>

#endif /* THREAD_CALLBACK_INTERFACE_HPP */
