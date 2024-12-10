cmake_minimum_required(VERSION 3.16)

project(GameCollectionManager VERSION 0.1 LANGUAGES CXX)

# Включаем автоматическую обработку .ui, .qrc и .moc файлов
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Находим необходимые компоненты Qt
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql Network Core Charts)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql Network Core Charts)

# Указываем все исходные файлы проекта
set(PROJECT_SOURCES
    main.cpp

    # Модели
    models/User.h
    models/User.cpp
    models/Game.h
    models/Game.cpp
    models/Genre.h
    models/Genre.cpp
    models/GameGenre.h
    models/GameGenre.cpp
    models/UserGame.h
    models/UserGame.cpp
    models/borrowing.h
    models/Borrowing.cpp
    models/Rating.h
    models/Rating.cpp
    models/ChatMessage.h
    models/ChatMessage.cpp
    models/Log.h
    models/Log.cpp

    # Контроллеры
    controllers/AuthController.h
    controllers/AuthController.cpp
    controllers/GameController.h
    controllers/GameController.cpp
    controllers/BorrowingController.h
    controllers/BorrowingController.cpp
    controllers/ChatController.h
    controllers/ChatController.cpp
    controllers/RatingController.h
    controllers/RatingController.cpp
    controllers/UserController.h
    controllers/UserController.cpp

    # Представления
    views/MainWindow.h
    views/MainWindow.cpp
    views/MainWindow.ui
    views/LoginWindow.h
    views/LoginWindow.cpp
    views/LoginWindow.ui
    views/RegistrationWindow.h
    views/RegistrationWindow.cpp
    views/RegistrationWindow.ui
    views/GameEditWindow.h
    views/GameEditWindow.cpp
    views/GameEditWindow.ui
    views/chatwindow.h
    views/ChatWindow.cpp
    views/ChatWindow.ui

    # Утилиты
    utils/DatabaseManager.h
    utils/DatabaseManager.cpp
    utils/ConfigManager.h
    utils/ConfigManager.cpp
    utils/Logger.h
    utils/Logger.cpp
    utils/Utils.h
    utils/Utils.cpp

    # Ресурсы
    resources/resources.qrc
    resources/styles.qss
    resources/styles/chat.qss
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(GameCollectionManager
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        controllers/usergamecontroller.h controllers/usergamecontroller.cpp
        controllers/admincontroller.h controllers/admincontroller.cpp
        views/gamedetailswindow.h views/gamedetailswindow.cpp views/gamedetailswindow.ui
        views/addgamewindow.h views/addgamewindow.cpp views/addgamewindow.ui
        views/borrowingswindow.h views/borrowingswindow.cpp views/borrowingswindow.ui
        views/adminpanelwindow.h views/adminpanelwindow.cpp views/adminpanelwindow.ui
    )
else()
    if(ANDROID)
        add_library(GameCollectionManager SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(GameCollectionManager
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# Линкуем необходимые библиотеки Qt
target_link_libraries(GameCollectionManager PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Charts)

# Настройки для разных платформ
set_target_properties(GameCollectionManager PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.GameCollectionManager
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Настройки установки приложения
install(TARGETS GameCollectionManager
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(GameCollectionManager)
endif()
