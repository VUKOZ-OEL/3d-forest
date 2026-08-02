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

/** @file Widget.hpp */

#ifndef WIDGET_HPP
#define WIDGET_HPP

// Include std.
#include <set>
#include <string>

// Include 3D Forest.
#include <CloseEvent.hpp>
#include <HideEvent.hpp>
#include <MouseEvent.hpp>
#include <PaintEvent.hpp>
#include <Palette.hpp>
#include <ShowEvent.hpp>
#include <Signal.hpp>
#include <Size.hpp>
#include <SizePolicy.hpp>
#include <ThemeIcon.hpp>
#include <Ui.hpp>
class Application;
class Layout;

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** Widget. */
class EXPORT_UI_COMMON Widget
{
public:
    Widget();
    Widget(Application *app);
    virtual ~Widget();

    void setText(const std::string &str);
    std::string text() const { return text_; }

    void setToolTip(const std::string &str);
    void setFocusPolicy(int focusPolicy);
    void setVisible(bool b);
    void setEnabled(bool b);
    void setDisabled(bool b);

    void setLayout(Layout *layout);
    Layout *layout() const { return layout_; }

    void setSizePolicy(SizePolicy::Policy hor, SizePolicy::Policy ver);
    void setStyleSheet(const std::string &str);

    Palette palette() const { return palette_; }
    void setPalette(const Palette &palette);

    Point mapToGlobal(const Point &point) const;

    bool blockSignals(bool b);
    bool signalsBlocked() const;

    void repaint();

    virtual Size sizeHint() const;
    virtual Size minimumSizeHint() const;

    virtual void paintEvent(PaintEvent *event);
    virtual void mousePressEvent(MouseEvent *event);
    virtual void hideEvent(HideEvent *event);
    virtual void showEvent(ShowEvent *event);
    virtual void closeEvent(CloseEvent *event);

private:
    Application *app_{nullptr};
    Layout *layout_{nullptr};
    std::string text_;
    std::string toolTip_;
    bool signalsBlocked_{false};
    Palette palette_;
};

#include <WarningsEnable.hpp>

#endif /* WIDGET_HPP */
