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

/** @file QtRadioButton.hpp */

#ifndef QT_RADIO_BUTTON_HPP
#define QT_RADIO_BUTTON_HPP

// Include 3D Forest.
#include <RadioButton.hpp>

// Include Qt.
#include <QRadioButton>

// Include local.
#include <ExportUiQt.hpp>
#include <WarningsDisable.hpp>

/** QtRadioButton. */
class EXPORT_UI_QT QtRadioButton : public QRadioButton
{
public:
    explicit QtRadioButton(RadioButton *radioButton, QWidget *parent = nullptr);
    virtual ~QtRadioButton();

protected:
    void nextCheckState() override;

private:
    RadioButton *radioButton_;
};

#include <WarningsEnable.hpp>

#endif /* QT_RADIO_BUTTON_HPP */
