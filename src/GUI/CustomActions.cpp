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

# ifdef DEBUG_VENTUROUS_CUSTOM_ACTIONS
# include <QtCoreUtilities/String.hpp>
# include <iostream>
# endif


# include "CustomActions.hpp"

# include "GuiUtilities.hpp"

# include <QPoint>
# include <QString>
# include <QStringList>
# include <QObject>
# include <QDir>
# include <QFileInfo>
# include <QProcess>
# include <QAction>
# include <QMenu>

# include <cstddef>
# include <utility>
# include <array>
# include <tuple>
# include <memory>


namespace
{
class CustomMenu : public QObject
{
    Q_OBJECT
public:
    explicit CustomMenu(const CustomActions::Actions & actions,
                        QString commonItemPrefix, QStringList itemNames);
    ~CustomMenu();

    /// @brief Shows popup menu if it is not empty at the specified position.
    /// Takes care of deleting this CustomMenu.
    /// @return true if menu was shown, false if there were no displayable
    /// actions.
    bool popup(const QPoint & position);

private slots:
    void onActionTriggered(QAction * action);

private:
    QString commonItemPrefix_;
    QStringList itemNames_;
    std::unique_ptr<QMenu> menu_;
};



class Validator
{
public:
    typedef CustomActions::Action Action;

    explicit Validator(const QString & commonItemPrefix,
                       const QStringList & itemNames)
        : commonItemPrefix_(commonItemPrefix), itemNames_(itemNames) {}

    bool isDisplayable(const Action & action) {
        return action.enabled &&
               isDisplayable(action.minArgN, action.maxArgN) &&
               isDisplayable(action.type);
    }

private:
    bool isDisplayable(int minArgN, int maxArgN) const {
        return itemNames_.size() >= minArgN &&
               (maxArgN == -1 || itemNames_.size() <= maxArgN);
    }

    bool isDisplayable(Action::Type type);


    const QString & commonItemPrefix_;
    const QStringList & itemNames_;
    std::array<bool, 2> displayedType_;
    bool checkedType_ = false;
};

std::pair<QString, QStringList> getProgramAndArgs(
    QString command, QString commonItemPrefix, QStringList itemNames);


CustomMenu::CustomMenu(const CustomActions::Actions & actions,
                       QString commonItemPrefix, QStringList itemNames)
    : commonItemPrefix_(std::move(commonItemPrefix)),
      itemNames_(std::move(itemNames))
{
    Validator validator(commonItemPrefix_, itemNames_);
    for (const CustomActions::Action & a : actions) {
        if (validator.isDisplayable(a)) {
            if (menu_ == nullptr)
                menu_.reset(new QMenu(tr("Custom actions")));
            QAction * const action = new QAction(a.text, menu_.get());
            action->setToolTip(a.command);
            menu_->addAction(action);
        }
    }
}

CustomMenu::~CustomMenu()
{
# ifdef DEBUG_VENTUROUS_CUSTOM_ACTIONS
    std::cout << "Entered CustomMenu destructor." << std::endl;
# endif
}

bool CustomMenu::popup(const QPoint & position)
{
    if (menu_ == nullptr) {
        deleteLater();
        return false;
    }
    connect(menu_.get(), SIGNAL(aboutToHide()), SLOT(deleteLater()));
    connect(menu_.get(), SIGNAL(triggered(QAction *)),
            SLOT(onActionTriggered(QAction *)));
    menu_->popup(position);
    return true;
}

void CustomMenu::onActionTriggered(QAction * const action)
{
# ifdef DEBUG_VENTUROUS_CUSTOM_ACTIONS
    std::cout << "Custom action was triggered: \"" +
              QtUtilities::qStringToString(action->text()) + "\"." << std::endl;
# endif

    QString command;
    QStringList args;
    std::tie(command, args) =
        getProgramAndArgs(action->toolTip(), std::move(commonItemPrefix_),
                          std::move(itemNames_));
    if (! command.isEmpty())
        QProcess::startDetached(command, args);
}


bool Validator::isDisplayable(const Action::Type type)
{
    if (type == Action::Type::anyItem)
        return true;
    const auto displayedType = [this](Action::Type type) -> bool & {
        return displayedType_[static_cast<std::size_t>(type)];
    };

    if (! checkedType_) {
        displayedType_.fill(true);
        for (const QString & name : itemNames_) {
            if (QFileInfo(commonItemPrefix_ + name).isDir()) {
                displayedType(Action::Type::file) = false;
                if (! displayedType(Action::Type::directory))
                    break; // both file and dir were found.
            }
            else {
                displayedType(Action::Type::directory) = false;
                if (! displayedType(Action::Type::file))
                    break; // both file and dir were found.
            }
        }
        checkedType_ = true;
    }
    return displayedType(type);
}


bool needsEscapingWithinDoubleQuotes(char c)
{
    // See https://www.gnu.org/software/bash/manual/html_node/Double-Quotes.html
    return c == '$' || c == '`' || c == '"' || c == '\\';
}

char toChar(const QChar & qc)
{
    return qc.toLatin1();
}

# ifdef WIN32_CUSTOM_ACTIONS_USE_BACKSLASH
QString & inPlaceReplaceSlashesWithBackslashes(QString & source)
{
    return source.replace('/', '\\');
}
QString replaceSlashesWithBackslashes(QString source)
{
    return source.replace('/', '\\');
}

# include <algorithm>

QStringList replaceSlashesWithBackslashes(const QStringList & source)
{
    QStringList result = source;
    std::for_each(result.begin(), result.end(), [](QString & str) {
        inPlaceReplaceSlashesWithBackslashes(str);
    });
    return result;
}
# endif // WIN32_CUSTOM_ACTIONS_USE_BACKSLASH

QString joinArgs(const QString & commonItemPrefix,
                 const QStringList & itemNames)
{
    if (itemNames.empty())
        return "";
# ifdef WIN32_CUSTOM_ACTIONS_USE_BACKSLASH
    const QString commonPrefix =
        replaceSlashesWithBackslashes(commonItemPrefix);
    const QStringList names = replaceSlashesWithBackslashes(itemNames);
    return commonPrefix + names.join(' ' + commonPrefix);
# else
    return commonItemPrefix + itemNames.join(' ' + commonItemPrefix);
# endif
}

QStringList getArgs(const QString & commonItemPrefix,
                    const QStringList & itemNames)
{
    QStringList args;
    args.reserve(itemNames.size());
# ifdef WIN32_CUSTOM_ACTIONS_USE_BACKSLASH
    const QString commonPrefix =
        replaceSlashesWithBackslashes(commonItemPrefix);
    QStringList names = replaceSlashesWithBackslashes(itemNames);
    for (QString & name : names)
        args.push_back(commonPrefix + std::move(name));
# else
    for (const QString & name : itemNames)
        args.push_back(commonItemPrefix + name);
# endif
    return args;
}

std::pair<QString, QStringList> getProgramAndArgs(
    QString command, QString commonItemPrefix, QStringList itemNames)
{
    QStringList args;

    QString current;
    enum class Mode : unsigned char { plain, inSingleQuotes, inDoubleQuotes };
    Mode mode = Mode::plain;

    for (int i = 0; i < command.size(); ++i) {
        const auto pushToCurrent = [&] { current += command[i]; };
        switch (toChar(command[i])) {
            case '\\':
                if (mode == Mode::plain) {
                    if (++i == command.size())
                        return { QString(), QStringList() }; // Wrong syntax.
                    else
                        pushToCurrent();
                }
                else if (mode == Mode::inDoubleQuotes) {
                    if (++i != command.size()) { // Wrong syntax otherwise.
                        const char c = toChar(command[i]);
                        if (c == '?' || needsEscapingWithinDoubleQuotes(c))
                            pushToCurrent(); // escaped.
                        else if (c != '\n') { // escaped newline is removed.
                            --i; // appending backslash symbol.
                            pushToCurrent();
                        }
                    }
                }
                else
                    pushToCurrent();
                break;
            case '"':
                if (mode == Mode::plain)
                    mode = Mode::inDoubleQuotes;
                else if (mode == Mode::inDoubleQuotes)
                    mode = Mode::plain;
                else
                    pushToCurrent();
                break;
            case '\'':
                if (mode == Mode::plain)
                    mode = Mode::inSingleQuotes;
                else if (mode == Mode::inDoubleQuotes)
                    pushToCurrent();
                else
                    mode = Mode::plain;
                break;
            case ' ':
            case '\t':
            case '\n':
                if (mode == Mode::plain) {
                    if (! current.isEmpty()) {
                        args << std::move(current);
                        current.clear();
                    }
                }
                else
                    pushToCurrent();
                break;
            case '?':
                if (mode == Mode::plain) {
                    QStringList a = getArgs(commonItemPrefix, itemNames);
                    if (! a.empty()) {
                        a.front().prepend(std::move(current));
                        current = std::move(a.back());
                        a.pop_back();
                        args << std::move(a);
                    }
                }
                else if (mode == Mode::inDoubleQuotes)
                    current += joinArgs(commonItemPrefix, itemNames);
                else
                    pushToCurrent();
                break;
            case '~':
                if (mode == Mode::plain) {
                    QString homePath = QDir::homePath();
# ifdef WIN32_CUSTOM_ACTIONS_USE_BACKSLASH
                    inPlaceReplaceSlashesWithBackslashes(homePath);
# endif
                    current += std::move(homePath);
                }
                else
                    pushToCurrent();
                break;
            default:
                pushToCurrent();
        }
    }

    if (mode != Mode::plain)
        return { QString(), QStringList() }; // Wrong syntax.
    if (! current.isEmpty())
        args << std::move(current);
    if (args.empty())
        command.clear();
    else {
        command = std::move(args.front());
        args.removeFirst();
    }
    return { std::move(command), std::move(args) };
}

}

namespace CustomActions
{
constexpr Action::TypeUnderlyingType Action::maxType;
constexpr int Action::minMinArgN, Action::maxMinArgN,
          Action::minMaxArgN, Action::maxMaxArgN;

bool operator == (const Action & lhs, const Action & rhs)
{
    return lhs.text == rhs.text && lhs.command == rhs.command &&
           lhs.minArgN == rhs.minArgN && lhs.maxArgN == rhs.maxArgN &&
           lhs.type == rhs.type && lhs.comment == rhs.comment &&
           lhs.enabled == rhs.enabled;
}

bool showMenu(const Actions & actions, QString commonItemPrefix,
              QStringList itemNames, const QPoint & position)
{
    CustomMenu * const menu = new CustomMenu(
        actions, std::move(commonItemPrefix), std::move(itemNames));
    return menu->popup(position);
}

void showMenu(const Actions & actions, QString commonItemPrefix,
              QStringList itemNames, const QPoint & position,
              GuiUtilities::TooltipShower & tooltipShower,
              QWidget * const widget)
{
    const int nItems = itemNames.size();
    if (! showMenu(actions, std::move(commonItemPrefix), std::move(itemNames),
                   position)) {
        tooltipShower.show(
            position,
            QObject::tr("No custom actions are enabled for selected %1.").
            arg(nItems == 1 ? QObject::tr("item") : QObject::tr("items")),
            widget);
    }
}

}


# ifdef INCLUDE_MOC
# include "CustomActions.moc"
# endif
