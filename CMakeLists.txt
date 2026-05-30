cmake_minimum_required(VERSION 3.16)

project(security-server LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS HttpServer)
find_package(Qt6 REQUIRED COMPONENTS Sql)

add_subdirectory(external/simple-mail)

add_executable(security-server
  main.cpp
  database/dbdriver.cpp
  database/dbdriver.h
  configmanager/configmanager.cpp
  configmanager/configmanager.h
  otp/otp.cpp
  otp/otp.h

)
target_link_libraries(security-server PRIVATE Qt6::Core)
target_link_libraries(security-server PRIVATE Qt6::HttpServer)
target_link_libraries(security-server PRIVATE Qt6::Sql)
target_link_libraries(security-server PRIVATE SimpleMail::Core)

target_include_directories(security-server PRIVATE database otp configmanager)

include(GNUInstallDirs)
install(TARGETS security-server
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
