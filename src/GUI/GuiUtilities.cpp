/*
 This file is part of Venturous.
 Copyright (C) 2014 Igor Kushnir <igorkuo AT Google mail>

 Venturous is free software: you can redistribute it and/or
 modify it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Venturous is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with
 Venturous.  If not, see <http://www.gnu.org/licenses/>.
*/

# include "GuiUtilities.hpp"

# include <QPoint>
# include <QString>
# include <QFont>
# include <QToolTip>
# include <QLabel>

# include <utility>


namespace GuiUtilities
{
QLabel * getCaptionLabel(const QString & text, QWidget * const parent,
                         const bool boldText, const int fontPointSizeIncrement)
{
    QLabel * const captionLabel = new QLabel(text, parent);
    if (boldText)
        captionLabel->setText("<b>" + captionLabel->text() + "</b>");
    if (fontPointSizeIncrement != 0) {
        QFont font = captionLabel->font();
        font.setPointSize(font.pointSize() + fontPointSizeIncrement);
        captionLabel->setFont(font);
    }
    return captionLabel;
}


void TooltipShower::show(QPoint position, QString text, QWidget * const widget)
{
    position_ = std::move(position);
    text_ = std::move(text);
    widget_ = widget;
    killTimer(timerId_);
    timerId_ = startTimer(300);
}

void TooltipShower::timerEvent(QTimerEvent *)
{
    QToolTip::showText(position_, text_, widget_);
    killTimer(timerId_);
    timerId_ = 0;
}

}
