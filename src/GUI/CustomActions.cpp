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

# include <QPoint>
# include <QString>
# include <QStringList>
# include <QObject>
# include <QFileInfo>
# include <QProcess>
# include <QAction>
# include <QMenu>

# include <cstddef>
# include <utility>
# include <array>
# include <memory>


namespace
{
class CustomMenu : public QObject
{
    Q_OBJECT
public:
    explicit CustomMenu(const CustomActions::Actions & actions,
                        QStringList arguments);
    ~CustomMenu();

    /// @brief Shows popup menu if it is not empty at the specified position.
    /// Takes care of deleting this CustomMenu.
    void popup(const QPoint & position);

private slots:
    void onActionTriggered(QAction * action);

private:
    QStringList arguments_;
    std::unique_ptr<QMenu> menu_;
};

class Validator
{
public:
    typedef CustomActions::Action Action;

    explicit Validator(const QStringList & arguments) : arguments_(arguments) {}

    bool isDisplayable(const Action & action) {
        return action.enabled &&
               isDisplayable(action.minArgN, action.maxArgN) &&
               isDisplayable(action.type);
    }

private:
    bool isDisplayable(int minArgN, int maxArgN) const {
        return arguments_.size() >= minArgN &&
               (maxArgN == -1 || arguments_.size() < maxArgN);
    }

    bool isDisplayable(Action::Type type);


    const QStringList & arguments_;
    std::array<bool, 3> displayedType_;
    bool checkedType_ = false;
};


CustomMenu::CustomMenu(const CustomActions::Actions & actions,
                       QStringList arguments) : arguments_(std::move(arguments))
{
    Validator validator(arguments_);
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

void CustomMenu::popup(const QPoint & position)
{
    if (menu_ == nullptr) {
        deleteLater();
        return;
    }
    connect(menu_.get(), SIGNAL(aboutToHide()), SLOT(deleteLater()));
    connect(menu_.get(), SIGNAL(triggered(QAction *)),
            SLOT(onActionTriggered(QAction *)));
    menu_->popup(position);
}


void CustomMenu::onActionTriggered(QAction * const action)
{
# ifdef DEBUG_VENTUROUS_CUSTOM_ACTIONS
    std::cout << "Custom action was triggered: \"" +
              QtUtilities::qStringToString(action->text()) + "\"." << std::endl;
# endif
    const auto escapeQuotes = [](QString & s) { s.replace('"', "\"\"\""); };

    QString command = action->toolTip();
    escapeQuotes(command);
    for (QString & arg : arguments_)
        escapeQuotes(arg);
    const QString args = '"' + arguments_.join("\" \"") + '"';

    for (int i = 0; i < command.size(); ++i) {
        const char c = '?';
        if (command[i] == c) {
            if (i + 1 != command.size() && command[i + 1] == c) {
                // escaped substitution character ("??").
                command.remove(i, 1);
            }
            else {
                command.replace(i, 1, args);
                i += args.size() - 1;
            }
        }
    }

    QProcess::startDetached(command);
}



bool Validator::isDisplayable(const Action::Type type)
{
    const auto displayedType = [this](Action::Type type) -> bool & {
        return displayedType_[static_cast<std::size_t>(type)];
    };

    if (! checkedType_) {
        displayedType_.fill(true);
        for (const QString & name : arguments_) {
            if (QFileInfo(name).isDir()) {
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

void showMenu(const Actions & actions, QStringList arguments,
              const QPoint & position)
{
    CustomMenu * const menu = new CustomMenu(actions, std::move(arguments));
    menu->popup(position);
}

}


# ifdef INCLUDE_MOC
# include "CustomActions.moc"
# endif
