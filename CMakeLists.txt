cmake_minimum_required(VERSION 3.0.0)
project(
  FitLog
  VERSION 0.1.0
  LANGUAGES C CXX)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/3rdparty/tgbot-cpp")
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/3rdparty/SQLiteCpp")
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/3rdparty/fmt")
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/3rdparty/sciplot")
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/3rdparty/json")
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/3rdparty/date")

set(${CMAKE_CXX_FLAGS} "-I${CMAKE_CURRENT_LIST_DIR}/src")

file(
  GLOB_RECURSE FIT_LOG_SOURCES
  RELATIVE "${CMAKE_CURRENT_LIST_DIR}"
  CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.[ch]pp")

add_executable(FitLog ${FIT_LOG_SOURCES} "${CMAKE_CURRENT_LIST_DIR}/3rdparty/date/src/tz.cpp")

add_library(Sqlite "${CMAKE_CURRENT_LIST_DIR}/3rdparty/sqlite_a10n/sqlite3.c")

target_link_libraries(Sqlite PUBLIC m)

target_include_directories(
  FitLog PUBLIC "${CMAKE_CURRENT_LIST_DIR}/3rdparty/sqlite_a10n")
target_include_directories(FitLog PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src")

target_link_libraries(
  FitLog
  PUBLIC TgBot
         Sqlite
         SQLiteCpp
         fmt::fmt
         sciplot
         nlohmann_json::nlohmann_json
         date::date)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
