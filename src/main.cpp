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

# include "MainWindow.hpp"
# include "Application.hpp"
# include "SharedMemory.hpp"

# include <QtCoreUtilities/String.hpp>

# include <QString>
# include <QObject>
# include <QSharedMemory>

# include <utility>
# include <memory>
# include <iostream>


namespace
{
inline QString quittingMessage() { return QObject::tr("\nQuitting ..."); }
}


int main(int argc, char * argv[])
{
# ifdef DEBUG_VENTUROUS_MAIN
    std::cout << "main() started." << std::endl;
# endif

    Application app(argc, argv);

    const QString key = SharedMemory::key();
    // WORKAROUND for recovering after crash in Unix.
    QSharedMemory(key).attach();

    std::unique_ptr<QSharedMemory> shared(new QSharedMemory(key));
    if (! shared->create(sizeof(char), QSharedMemory::ReadWrite)) {
        if (shared->error() == QSharedMemory::AlreadyExists) {
            shared->attach(QSharedMemory::ReadWrite);
            shared->lock();
            *(char *)shared->data() = SharedMemory::Symbol::show();
            shared->unlock();

            std::cout << QtUtilities::qStringToString(
                          QObject::tr("Another instance of %1 is running.\n"
                                      "Issued a command to show other "
                                      "instance's window.").
                          arg(APPLICATION_NAME) + quittingMessage())
                      << std::endl;
            return 1;
        }
        else {
            std::cerr << QtUtilities::qStringToString(shared->errorString())
                      << '.' << std::endl;
            return 2;
        }
    }
    {
        shared->lock();
        *(char *)shared->data() = SharedMemory::Symbol::noCommand();
        shared->unlock();
    }

    bool cancelled;
    MainWindow mainWindow(std::move(shared), cancelled);
# ifdef DEBUG_VENTUROUS_MAIN
    std::cout << "MainWindow constructed." << std::endl;
# endif
    if (cancelled) {
        std::cout << QtUtilities::qStringToString(
                      QObject::tr("Launching %1 was cancelled.").
                      arg(APPLICATION_NAME) + quittingMessage())
                  << std::endl;
        return 3;
    }

    return app.exec();
}
