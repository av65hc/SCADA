cmake_minimum_required(VERSION 3.19)
project(Scada LANGUAGES CXX)
add_compile_options(-Wa,-mbig-obj)

find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core
    Widgets
    SerialPort
    SerialBus
    Network
    Sql
    PrintSupport
)

qt_standard_project_setup()

qt_add_executable(Scada
    WIN32 MACOSX_BUNDLE
    #UI层/主程序入口
    src/ui/main.cpp
    src/ui/mainwindow.cpp
    src/ui/mainwindow.h
    src/ui/mainwindow.ui
    #通信层src/comm
    src/comm/modbusmaster.h src/comm/modbusmaster.cpp
    src/comm/linkworker.h src/comm/linkworker.cpp
    src/comm/linkmanager.h src/comm/linkmanager.cpp
    #实体类
    src/entity/deviceentity.h
    src/entity/registeritem.h
    src/entity/alarmitem.h
    #协议层
    src/protocol/crc16.h src/protocol/crc16.cpp
    #数据库层
    src/database/sqlitedb.h src/database/sqlitedb.cpp
    src/database/dbworker.h src/database/dbworker.cpp
    #工具层
    src/utils/threadqueue.h
    src/utils/loghelper.h src/utils/loghelper.cpp
    src/utils/devconfighelper.h src/utils/devconfighelper.cpp
    #第三方库
    thirdparty/qcustomplot.h thirdparty/qcustomplot.cpp
    #弹窗层
    dialog/deviceconfigdialog.h dialog/deviceconfigdialog.cpp
)

target_link_libraries(Scada
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::SerialPort
        Qt6::SerialBus
        Qt6::Network
        Qt6::Sql
        Qt6::PrintSupport
)

target_include_directories(Scada PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

install(TARGETS Scada
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET Scada
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})