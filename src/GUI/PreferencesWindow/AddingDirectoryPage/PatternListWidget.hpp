/*
 This file is part of Venturous.
 Copyright (C) 2014, 2015 Igor Kushnir <igorkuo AT Google mail>

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

# ifndef VENTUROUS_PATTERN_LIST_WIDGET
# define VENTUROUS_PATTERN_LIST_WIDGET

# include "FilePattern.hpp"

# include <QtWidgetsUtilities/TooltipShower.hpp>

# include <QtGlobal>
# include <QString>
# include <QListWidget>

# include <set>


QT_FORWARD_DECLARE_CLASS(QListWidgetItem)

/// A list widget of unique checkable file patterns.
class PatternListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit PatternListWidget(QWidget * parent = nullptr);

    /// @brief Replaces the existing patterns with the supplied ones.
    void setUiPatterns(const FilePatternList & patterns);
    /// @return All patterns in the list.
    FilePatternList getUiPatterns() const;

    /// @brief Adds patterns to the list and scrolls the list to bottom.
    void addPatterns(const FilePatternList & patterns);

    /// @return All selected patterns.
    FilePatternList getSelectedPatterns() const;

public slots:
    /// @brief Adds a new pattern and starts editing it.
    void addPattern();

public:
    /// This type alias is for internal use only. It is public for
    /// technical reasons (QVariant and Q_DECLARE_METATYPE).
    using PatternSet = std::set<QString>;

private:
    /// @brief If no such pattern is present in the list, adds it to the list
    /// and to the set; otherwise does nothing.
    void addUniquePattern(const FilePattern & pattern);
    /// @brief Calls addUniquePattern() for each element of patterns.
    void addUniquePatterns(const FilePatternList & patterns);
    /// @brief Adds pattern to the UI list, sets the specified check state;
    /// stores iterator in the newly added item's user data.
    /// @return The newly added item.
    QListWidgetItem * addPatternToUiList(const QString & pattern, bool checked,
                                         PatternSet::iterator setIterator);

    void keyPressEvent(QKeyEvent *) override;
    template <typename ItemUser>
    void applyToSelectedItems(ItemUser itemUser);

    PatternSet patternSet_;

    QtUtilities::Widgets::TooltipShower tooltipShower_;

private slots:
    void onItemChanged(QListWidgetItem * item);
};

# endif // VENTUROUS_PATTERN_LIST_WIDGET
