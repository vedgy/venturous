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

# ifndef VENTUROUS_ADDING_POLICY_FRAME_HPP
# define VENTUROUS_ADDING_POLICY_FRAME_HPP

# include <QFrame>
# include <QCheckBox>


namespace AddingItems
{
struct Policy;
}

class AddingPolicyFrame : public QFrame
{
    Q_OBJECT
public:
    explicit AddingPolicyFrame(QWidget * parent = nullptr,
                               Qt::WindowFlags f = 0);

    void setUiPreferences(const AddingItems::Policy & source);
    void writeUiPreferencesTo(AddingItems::Policy & destination) const;

private:
    QCheckBox addFilesCheckBox;
    QCheckBox addMediaDirsCheckBox;
    QCheckBox ifBothAddFilesCheckBox;
    QCheckBox ifBothAddMediaDirsCheckBox;

    QFrame * primaryFrame;

private slots:
    void onPrimaryCheckBoxToggled();
};

# endif // VENTUROUS_ADDING_POLICY_FRAME_HPP
