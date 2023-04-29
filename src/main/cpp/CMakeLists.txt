cmake_minimum_required(VERSION 3.4.1)
project(oboe-player LANGUAGES CXX)

find_package(oboe REQUIRED CONFIG)

set (APP_SOURCES SimpleNoiseMaker.cpp OboePlayerJNI.cpp)

add_library(oboeplayer SHARED ${APP_SOURCES})

target_link_libraries(oboeplayer oboe::oboe android log)

target_compile_options(oboeplayer PRIVATE -Wall -Wno-unused-private-field -Wno-unused-variable -Werror "$<$<CONFIG:RELEASE>:-Ofast>")
