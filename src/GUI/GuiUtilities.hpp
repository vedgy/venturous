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

# ifndef VENTUROUS_GUI_UTILITIES_HPP
# define VENTUROUS_GUI_UTILITIES_HPP

# include <QtGlobal>
# include <QPoint>
# include <QString>
# include <QObject>


QT_FORWARD_DECLARE_CLASS(QWidget)
QT_FORWARD_DECLARE_CLASS(QLabel)

namespace GuiUtilities
{
QLabel * getCaptionLabel(const QString & text, QWidget * parent = nullptr,
                         bool boldText = true, int fontPointSizeIncrement = 2);

class TooltipShower : public QObject
{
public:
    /// @brief Shows tooltip with passed text at the specified position after
    /// a short delay.
    /// @param widget Is used to determine the appropriate screen on
    /// multi-head systems.
    void show(QPoint position, QString text, QWidget * widget = nullptr);

private:
    void timerEvent(QTimerEvent *) override;

    QPoint position_;
    QString text_;
    QWidget * widget_;
    int timerId_ = 0;
};

}

# endif // VENTUROUS_GUI_UTILITIES_HPP
