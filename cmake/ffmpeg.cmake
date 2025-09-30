find_package(FFmpeg REQUIRED COMPONENTS AVCODEC AVFORMAT AVUTIL SWSCALE)

add_library(ffmpeg INTERFACE)
target_link_libraries(ffmpeg INTERFACE FFmpeg::AVCODEC FFmpeg::AVFORMAT FFmpeg::AVUTIL FFmpeg::SWSCALE)
