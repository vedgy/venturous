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

# ifndef VENTUROUS_PATTERN_LIST_WIDGET
# define VENTUROUS_PATTERN_LIST_WIDGET

# include <QtGlobal>
# include <QString>
# include <QListWidget>

# include <functional>
# include <array>
# include <vector>


QT_FORWARD_DECLARE_CLASS(QStringList)
QT_FORWARD_DECLARE_CLASS(QListWidgetItem)

class PatternListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit PatternListWidget(QWidget * parent = nullptr);

    /// @brief All previously specified known patterns are unchecked, unknown
    /// patterns are removed. Then supplied patterns are added (if unknown) and
    /// checked.
    void setUiPatterns(const QStringList & patterns);
    /// @return All (unique!) checked patterns.
    QStringList getUiPatterns() const;

    /// @brief Adds unchecked patterns.
    void addUnknownPatterns(const QStringList & unknownPatterns);

    /// @return All (unique!) selected unknown patterns.
    QStringList getSelectedUnknownPatterns() const;

public slots:
    /// @brief Adds unknown pattern, checks it and starts editing it.
    void addUnknownPattern();

private:
    struct KnownPattern {
        explicit KnownPattern(const QString & pattern, QListWidgetItem * item);
        /// @brief Creates item and sets appropriate flags.
        explicit KnownPattern(const QString & pattern);

        bool operator<(const KnownPattern & rhs) const {
            return pattern < rhs.pattern;
        }

        QString pattern;
        QListWidgetItem * item;
    };

    using ItemUser = std::function<void(QListWidgetItem *)>;

    void unselectAllItems();

    void removeAllUnknownPatterns();

    /// @brief Adds patterns to knownPatterns_ and as ListWidgetItems.
    void addKnownPatterns(const QStringList & patterns);

    void keyPressEvent(QKeyEvent *) override;

    void removeSelectedUnknownItems();

    void applyToSelectedItems(ItemUser itemUser);

    /// Indices of caption rows. Is not changed after constructor.
    std::array<int, 3> captionRows_;
    /// Sorted collection of known patterns. Is not changed after constructor.
    std::vector<KnownPattern> knownPatterns_;
};

# endif // VENTUROUS_PATTERN_LIST_WIDGET
