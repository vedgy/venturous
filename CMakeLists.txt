cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# Depends on Qt, QtCoreUtilities, QtXmlUtilities, SimpleFugue, VenturousCore.
set(Target_Name Venturous)
set(Tool_Name Ventool)

set(PATH_TO_MODULES modules)
set(PATH_TO_VEDG_TOOLS ${PATH_TO_MODULES}/vedgTools)

set(PATH_TO_CMAKE_MODULES
        ${CMAKE_CURRENT_SOURCE_DIR}/${PATH_TO_VEDG_TOOLS}/CMakeModules)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PATH_TO_CMAKE_MODULES})
include(vedgTools/CMakeLicense)

include(vedgTools/ExecutableQtStart)

set(PREFERENCES_DIR ~/.config/venturous CACHE STRING
            "Directory that contains user preferences and playlist.")
option(ALWAYS_EMBED_ICONS
        "Always bundle icons with the executable. If disabled, embedding will be done only for WIN32 target."
            ON)
option(ADAPT_CUSTOM_ACTIONS_IF_WIN32
        "Adapt custom actions for MS Windows for WIN32 target. This option has no effect for other targets."
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

include(vedgTools/EnableAutomoc)


if(DEBUG_VENTUROUS)
    add_definitions(
        -DDEBUG_VENTUROUS_APPLICATION -DDEBUG_VENTUROUS_MAIN
        -DDEBUG_VENTUROUS_ICONS -DDEBUG_VENTUROUS_CUSTOM_ACTIONS
        -DDEBUG_VENTUROUS_CUSTOM_ACTIONS_PAGE
        -DDEBUG_VENTUROUS_PREFERENCES_COMPONENT
        -DDEBUG_VENTUROUS_TREE_WIDGET -DDEBUG_VENTUROUS_PLAYLIST_COMPONENT
        -DDEBUG_VENTUROUS_WINDOW_UTILITIES -DDEBUG_VENTUROUS_MAIN_WINDOW
    )
    set(EXECUTABLE_ARGS "")
else()
    set(EXECUTABLE_ARGS WIN32)
endif()

include(vedgTools/StringAppendSlashIfAbsent)
stringAppendSlashIfAbsent(PREFERENCES_DIR)

set(Executable_Name venturous)
set(Icon_Name ${Executable_Name})

add_definitions(
    -DPREFERENCES_DIR=\"${PREFERENCES_DIR}\"
    -DERROR_PREFIX=\"**\ ${Target_Name}\ CRITICAL\ **\ \"
    -DAPPLICATION_NAME=\"${Target_Name}\"
    -DTOOL_NAME=\"${Tool_Name}\"
    -DICON_NAME=\"${Icon_Name}\"
    -DSHARED_MEMORY_KEY=\"oS2huoventurousyiOventoolhJkNQqC{\"
)


set(Sources_Path src)
set(GUI_Path ${Sources_Path}/GUI)
set(PreferencesWindow_Path ${GUI_Path}/PreferencesWindow)
set(GeneralPage_Path ${PreferencesWindow_Path}/GeneralPage)
set(PlaybackPage_Path ${PreferencesWindow_Path}/PlaybackPage)
set(AddingDirectoryPage_Path ${PreferencesWindow_Path}/AddingDirectoryPage)
set(CustomActionsPage_Path ${PreferencesWindow_Path}/CustomActionsPage)
set(MainWindow_Path ${GUI_Path}/MainWindow)
set(PreferencesComponent_Path ${MainWindow_Path}/PreferencesComponent)
set(PlaybackComponent_Path ${MainWindow_Path}/PlaybackComponent)
set(PlaylistComponent_Path ${MainWindow_Path}/PlaylistComponent)
set(Resources_Path resources)

include_directories(
    ${QtCoreUtilities_PublicHeaders_Path} ${QtXmlUtilities_PublicHeaders_Path}
    ${TemplateUtilities_PublicHeaders_Path} ${VenturousCore_PublicHeaders_Path}
    ${Sources_Path} ${GUI_Path}
    ${PreferencesWindow_Path} ${GeneralPage_Path} ${PlaybackPage_Path}
    ${AddingDirectoryPage_Path} ${CustomActionsPage_Path}
    ${MainWindow_Path} ${PreferencesComponent_Path} ${PlaybackComponent_Path}
    ${PlaylistComponent_Path}
)

set(Sources
    ${Sources_Path}/Application.cpp ${Sources_Path}/Preferences.cpp
    ${Sources_Path}/main.cpp

    ${GUI_Path}/GuiUtilities.cpp
    ${GUI_Path}/Icons.cpp ${GUI_Path}/Actions.cpp ${GUI_Path}/CustomActions.cpp

    ${GeneralPage_Path}/GeneralPage.cpp
    ${PlaybackPage_Path}/HistoryPreferencesFrame.cpp
    ${PlaybackPage_Path}/PlaybackPage.cpp
    ${AddingDirectoryPage_Path}/AddingPolicyFrame.cpp
    ${AddingDirectoryPage_Path}/PatternListWidget.cpp
    ${AddingDirectoryPage_Path}/AddingDirectoryPage.cpp
    ${CustomActionsPage_Path}/CustomActionsPage.cpp
    ${PreferencesWindow_Path}/RevertAndRestoreDefaultsTab.cpp
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
    qtAddResources(Resources ${Resources_Path}/SimpleFugue/SimpleFugue.qrc)
    add_definitions(-DEMBEDDED_ICONS)
endif()

if(WIN32 AND ADAPT_CUSTOM_ACTIONS_IF_WIN32)
    add_definitions(
        -DWIN32_DEFAULT_CUSTOM_ACTIONS
        -DWIN32_CUSTOM_ACTIONS_USE_BACKSLASH
    )
endif()


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

install(FILES "${Executable_Name}.desktop" DESTINATION share/applications)

foreach(Icon_Size 16 22 24 32 48 96 256 "scalable")
    set(End_Of_Path "${Icon_Size}/apps")
    if (${Icon_Size} STREQUAL "scalable")
        set(Icon_Extension "svg")
    else()
        set(End_Of_Path "${Icon_Size}x${End_Of_Path}")
        set(Icon_Extension "png")
    endif()
    install(FILES
    "${PATH_TO_MODULES}/SimpleFugue/${End_Of_Path}/${Icon_Name}.${Icon_Extension}"
            DESTINATION share/icons/hicolor/${End_Of_Path})
endforeach()
