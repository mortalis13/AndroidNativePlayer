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

include_directories(. wav_parser)

add_library(oboeplayer SHARED ${APP_SOURCES})


# ffmpeg-4.4.3
set(FFMPEG_DIR "${CMAKE_CURRENT_LIST_DIR}/ffmpeg/${ANDROID_ABI}")
include_directories(${FFMPEG_DIR}/include)

add_library(avformat SHARED IMPORTED)
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION ${FFMPEG_DIR}/lib/libavformat.so)

add_library(avutil SHARED IMPORTED)
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION ${FFMPEG_DIR}/lib/libavutil.so)

add_library(avcodec SHARED IMPORTED)
set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION ${FFMPEG_DIR}/lib/libavcodec.so)

add_library(swresample SHARED IMPORTED)
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION ${FFMPEG_DIR}/lib/libswresample.so)
# end ffmpeg


target_link_libraries(oboeplayer oboe::oboe android log avformat avutil avcodec swresample)
