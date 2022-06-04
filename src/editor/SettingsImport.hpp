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

/** @file SettingsImport.hpp */

#ifndef SETTINGS_IMPORT_HPP
#define SETTINGS_IMPORT_HPP

#include <IndexFileBuilder.hpp>
#include <ExportEditor.hpp>

/** Settings Import. */
class EXPORT_EDITOR SettingsImport
{
public:
    SettingsImport();

    bool isCenterEnabled() const;
    void setCenterEnabled(bool b);

    const IndexFileBuilder::Settings &indexSettings() const
    {
        return indexSettings_;
    }

protected:
    bool centerEnabled_;
    IndexFileBuilder::Settings indexSettings_;
};

#endif /* SETTINGS_IMPORT_HPP */
