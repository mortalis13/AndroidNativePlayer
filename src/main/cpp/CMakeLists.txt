cmake_minimum_required(VERSION 3.4.1)
project(oboe-player LANGUAGES CXX)

find_package(oboe REQUIRED CONFIG)

set (
  APP_SOURCES
  
  FilePlayer.cpp
  OboePlayerJNI.cpp
  
  wav_parser/stream/FileInputStream.cpp
  wav_parser/stream/InputStream.cpp
  wav_parser/stream/MemInputStream.cpp
  wav_parser/wav/AudioEncoding.cpp
  wav_parser/wav/WavChunkHeader.cpp
  wav_parser/wav/WavFmtChunkHeader.cpp
  wav_parser/wav/WavRIFFChunkHeader.cpp
  wav_parser/wav/WavStreamReader.cpp
  
  resampler/IntegerRatio.cpp
  resampler/LinearResampler.cpp
  resampler/MultiChannelResampler.cpp
  resampler/PolyphaseResampler.cpp
  resampler/PolyphaseResamplerMono.cpp
  resampler/PolyphaseResamplerStereo.cpp
  resampler/SincResampler.cpp
  resampler/SincResamplerStereo.cpp
)

message(CMAKE_CURRENT_LIST_DIR=${CMAKE_CURRENT_LIST_DIR})

include_directories(. wav_parser filters)

add_library(oboeplayer SHARED ${APP_SOURCES})

target_link_libraries(oboeplayer oboe::oboe android log)

# target_compile_options(oboeplayer PRIVATE -Wall -Wno-unused-private-field -Wno-unused-variable -Werror "$<$<CONFIG:RELEASE>:-Ofast>")
# target_compile_options(oboeplayer PRIVATE -w "$<$<CONFIG:RELEASE>:-Ofast>")
