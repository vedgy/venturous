cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# Depends on Qt, QtCoreUtilities, QtXmlUtilities, SimpleFugue, VenturousCore.
set(Target_Name Venturous)
set(Tool_Name Ventool)

set(PATH_TO_MODULES modules)
set(PATH_TO_VEDG_TOOLS ${PATH_TO_MODULES}/vedgTools)

set(CMAKE_MODULE_PATH
        "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/${PATH_TO_VEDG_TOOLS}/CMakeModules")
include(vedgTools/CMakeLicense)

include(vedgTools/ExecutableQtStart)

set(PREFERENCES_DIR ~/.config/venturous CACHE STRING
            "Directory that contains user preferences and playlist.")
option(ALWAYS_EMBED_ICONS
        "Always bundle icons with the executable. If disabled, embedding will be done only for WIN32 target."
            ON)
option(DEBUG_VENTUROUS "Print details of internal workflow to stdout." OFF)
option(BUILD_VENTOOL "Build ${Tool_Name} along with ${Target_Name}." ON)


project(${Target_Name})
include(vedgTools/SetCxxFlags)


include(vedgTools/ExecutableFindQt)
executableFindQt(Qt5Core Qt5Xml Qt5Widgets . QTCORE QTXML QTGUI)

include(vedgTools/ExecutableGetPublicHeadersProperty)
add_subdirectory(${PATH_TO_VEDG_TOOLS}/QtCoreUtilities)
executableGetPublicHeadersProperty(QtCoreUtilities)
add_subdirectory(${PATH_TO_VEDG_TOOLS}/QtXmlUtilities)
executableGetPublicHeadersProperty(QtXmlUtilities)
add_subdirectory(${PATH_TO_MODULES}/VenturousCore)
executableGetPublicHeadersProperty(VenturousCore)
executableGetPublicHeadersProperty(TemplateUtilities)

set(CMAKE_AUTOMOC ON)


if(DEBUG_VENTUROUS)
    add_definitions(
        -DDEBUG_VENTUROUS_APPLICATION -DDEBUG_VENTUROUS_MAIN
        -DDEBUG_VENTUROUS_ICONS -DDEBUG_VENTUROUS_PREFERENCES_COMPONENT
        -DDEBUG_VENTUROUS_TREE_WIDGET -DDEBUG_VENTUROUS_PLAYLIST_COMPONENT
        -DDEBUG_VENTUROUS_WINDOW_UTILITIES -DDEBUG_VENTUROUS_MAIN_WINDOW
    )
    set(EXECUTABLE_ARGS "")
else()
    set(EXECUTABLE_ARGS WIN32)
endif()

include(vedgTools/StringAppendSlashIfAbsent)
stringAppendSlashIfAbsent(PREFERENCES_DIR)

add_definitions(
    -DPREFERENCES_DIR=\"${PREFERENCES_DIR}\"
    -DERROR_PREFIX=\"**\ ${Target_Name}\ CRITICAL\ **\ \"
    -DAPPLICATION_NAME=\"${Target_Name}\"
    -DTOOL_NAME=\"${Tool_Name}\"
    -DSHARED_MEMORY_KEY=\"oS2huoventurousyiOventoolhJkNQqC{\"
)


set(Sources_Path src)
set(GUI_Path ${Sources_Path}/GUI)
set(PreferencesWindow_Path ${GUI_Path}/PreferencesWindow)
set(MainWindow_Path ${GUI_Path}/MainWindow)
set(PreferencesComponent_Path ${MainWindow_Path}/PreferencesComponent)
set(PlaybackComponent_Path ${MainWindow_Path}/PlaybackComponent)
set(PlaylistComponent_Path ${MainWindow_Path}/PlaylistComponent)
set(Resources_Path resources)

include_directories(
    ${QtCoreUtilities_PublicHeaders_Path} ${QtXmlUtilities_PublicHeaders_Path}
    ${TemplateUtilities_PublicHeaders_Path} ${VenturousCore_PublicHeaders_Path}
    ${Sources_Path} ${GUI_Path} ${PreferencesWindow_Path} ${MainWindow_Path}
    ${PreferencesComponent_Path} ${PlaybackComponent_Path}
    ${PlaylistComponent_Path}
)

set(Sources
    ${Sources_Path}/Application.cpp ${Sources_Path}/Preferences.cpp
    ${Sources_Path}/main.cpp

    ${GUI_Path}/Icons.cpp ${GUI_Path}/Actions.cpp

    ${PreferencesWindow_Path}/GeneralPage.cpp
    ${PreferencesWindow_Path}/HistoryPreferencesFrame.cpp
    ${PreferencesWindow_Path}/PlaybackPage.cpp
    ${PreferencesWindow_Path}/PatternListWidget.cpp
    ${PreferencesWindow_Path}/FilenamePatternsPage.cpp
    ${PreferencesWindow_Path}/AddingPolicyPage.cpp
    ${PreferencesWindow_Path}/PreferencesWindow.cpp

    ${PreferencesComponent_Path}/PreferencesComponent.cpp
    ${PlaybackComponent_Path}/HistoryWidget.cpp
    ${PlaybackComponent_Path}/PlaybackComponent.cpp
    ${PlaylistComponent_Path}/TreeWidget.cpp
    ${PlaylistComponent_Path}/PlaylistComponent.cpp
    ${MainWindow_Path}/WindowUtilities.cpp
    ${MainWindow_Path}/MainWindow.cpp ${MainWindow_Path}/MainWindow-init.cpp
    ${MainWindow_Path}/MainWindow-help.cpp
)

include(vedgTools/QtAddResources)
qtAddResources(Resources ${Resources_Path}/Venturous.qrc)

if(ALWAYS_EMBED_ICONS OR WIN32)
    qtAddResources(Resources ${PATH_TO_MODULES}/SimpleFugue.qrc)
    add_definitions(-DEMBEDDED_ICONS)
endif()


set(Executable_Name venturous)
add_executable(${Executable_Name} ${EXECUTABLE_ARGS} ${Sources} ${Resources})

include(vedgTools/LinkQt)
linkQt(${Executable_Name}
    Core Xml Widgets .
    ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTGUI_LIBRARY}
)

target_link_libraries(${Executable_Name}
    QtCoreUtilities QtXmlUtilities VenturousCore
)

set(Targets ${Executable_Name})

if(BUILD_VENTOOL)
    set(Tool_Executable ventool)
    add_definitions(-DTOOL_EXECUTABLE=\"${Tool_Executable}\")

    add_executable(${Tool_Executable}
                    "${Sources_Path}/${Tool_Name}/${Tool_Executable}.cpp")
    linkQt(${Tool_Executable} Core . ${QT_QTCORE_LIBRARY})
    set(Targets ${Targets} ${Tool_Executable})
endif()


install(TARGETS ${Targets} RUNTIME DESTINATION bin)

install(FILES venturous.desktop DESTINATION share/applications)
install(FILES ${Resources_Path}/icons/venturous.png DESTINATION share/icons)
