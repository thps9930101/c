QT       += core gui network multimedia multimediawidgets widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
CONFIG += console
CONFIG += utf8_source

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    element/login.cpp \
    element/setting.cpp \
    main.cpp \
    mainwindow.cpp \
    tool/apiclient.cpp \
    tool/jsonfilemanager.cpp \
    tool/pressevent.cpp \
    tool/VideoDecoder.cpp \
    tool/ToastMessage.cpp \
    ui/loginUI.cpp\
    Data/Struct/AoFunSetting/AoFunSetting.cpp \
    Data/Struct/App/app.cpp \
    Data/Struct/AppSettings/appSettings.cpp \
    Data/Struct/CamInfo/CamInfo.cpp \
    Data/Struct/Cam_Set/Cam_Set.cpp \
    Data/Struct/Decoder_param/decoder_class_mode.cpp \
    Data/Struct/Encoder_param/encoder_param.cpp \
    Data/Struct/Record_param/Record_param.cpp \
    Data/Struct/RTSP_param/RTSP_param.cpp \
    Data/Struct/multiple_vid_ctx_param/multiple_vid_ctx_param.cpp \
    Data/Struct/rot_param/rot_param.cpp \
    mode/AOCamStream/sync_frame_model2.cpp \
    mode/ao_cameram_fun.cpp \
    mode/cam/ao_readcam.cpp \
    mode/cam/vid_record/multiple_vid_ctx.cpp \
    mode/cam/vid_record/multiple_vid_record.cpp \
    mode/decode/decode.cpp \
    mode/encoder/output_rtmp.cpp \
    mode/encoder/output_rtsp2.cpp \
    mode/encoder/output_file.cpp \
    mode/encoder/encode.cpp \
    mode/encoder/encode_h264_nvenc.cpp \
    mode/splice/splice.cpp \
    mode/buffer/FrameBuffer.cpp \
    mode/buffer/AudioAnalyzer.cpp \
    mode/buffer/DecoderThreadProcessor.cpp \
    mode/buffer/AudioWaveformWidget.cpp \
    mode/buffer/jsonfilemanager.cpp \
    mode/buffer/VideoEncoder.cpp \
    mode/buffer/AudioWaveformWidget.cpp \

HEADERS += \
    Data/Enum/Drag_mode.h \
    element/login.h \
    element/setting.h \
    mainwindow.h \
    struct/FrameData.h \
    tool/Slider.h \
    tool/apiclient.h \
    tool/jsonfilemanager.h \
    tool/playthread.h \
    tool/pressevent.h \
    tool/VideoDecoder.h \
    tool/ToastMessage.h \
    Data/Define/DefineData.h \
    Data/Enum/SP_Mode.h \
    Data/Enum/SP_Rot.h \
    Data/Enum/appStatus.h \
    Data/Struct/AoFunSetting/AoFunSetting.h \
    Data/Struct/App/app.h \
    Data/Struct/AppSettings/appSettings.h \
    Data/Struct/CamInfo/CamInfo.h \
    Data/Struct/Cam_Set/Cam_Set.h \
    Data/Struct/Decoder_param/decoder_class_mode.h \
    Data/Struct/Encoder_param/encoder_param.h \
    Data/Struct/Record_param/Record_param.h \
    Data/Struct/RTSP_param/RTSP_param.h \
    Data/Struct/rot_param/rot_param.h \
    Data/Struct/multiple_vid_ctx_param/multiple_vid_ctx_param.h \
    mode/AOCamStream/sync_frame_model2.h \
    mode/ao_cameram_fun.h \
    mode/cam/ao_readcam.h \
    mode/cam/vid_record/multiple_vid_ctx.h \
    mode/cam/vid_record/multiple_vid_record.h \
    mode/decode/decode.h \
    mode/encoder/output_rtmp.h \
    mode/encoder/output_rtsp2.h \
    mode/encoder/output_file.h \
    mode/encoder/encode.h \
    mode/encoder/encode_h264_nvenc.h \
    mode/my_container/BoundedQueue.h \
    mode/my_container/ObjectPool2.h \
    mode/my_container/RingBoundedQueue.h \
    mode/my_container/RingBuffer.h \
    mode/my_container/mem_pool.h \
    mode/splice/splice.h \
    mode/buffer/FrameBuffer.h \
    mode/buffer/AudioAnalyzer.h \
    mode/buffer/DecoderThreadProcessor.h \
    mode/buffer/AudioWaveformWidget.h \
    mode/buffer/jsonfilemanager.h \
    mode/buffer/VideoEncoder.h \
    mode/buffer/AudioWaveformWidget.h \
    ui/loginUI.h
FORMS += \
    element/login.ui \
    element/setting.ui \
    mainwindow.ui

INCLUDEPATH += C:/FFMPEG/ffmpeg-n4.3.2-163-g6c414cf8f7-win64-lgpl-shared-4.3/include/
INCLUDEPATH += ./lib/libcurl-vc15-x64-release-dll-ipv6-sspi-schannel/include
INCLUDEPATH += ./lib/ao
INCLUDEPATH += ./lib/Myconver

INCLUDEPATH += D:/opencv_gpu4.7.0/install/include
INCLUDEPATH += ./mode/
INCLUDEPATH += ./element/
INCLUDEPATH += ./ui/
INCLUDEPATH += ./tool/
INCLUDEPATH += ./struct/

CONFIG(debug, debug|release){
#    LIBS += -L./lib/camera_lib/lib/win_x64_vs2017/ -llibsspd
    LIBS += -LD:/opencv_gpu4.7.0/install/x64/vc15/lib -lopencv_img_hash470d -lopencv_world470d
    LIBS += -L./lib/ao/ -llibaocpd
    LIBS += -L./lib/Myconver/ -lConversion
}

CONFIG(release, debug|release){
#    LIBS += -L./lib/camera_lib/lib/win_x64_vs2017/ -llibssp
    LIBS += -LD:/opencv_gpu4.7.0/install/x64/vc15/lib -lopencv_img_hash470 -lopencv_world470
    LIBS += -L./lib/ao/ -llibaocp
    LIBS += -L./lib/Myconver/ -lConversion
}

INCLUDEPATH += C:/FFMPEG/ffmpeg-n4.3.2-163-g6c414cf8f7-win64-lgpl-shared-4.3/include/
#INCLUDEPATH += ../ffmpeg/include
#INCLUDEPATH += ./lib/camera_lib/include
INCLUDEPATH += ./lib/libcurl-vc15-x64-release-dll-ipv6-sspi-schannel/include
#/lib/ffmpeg-n4.3.2-163-g6c414cf8f7-win64-lgpl-shared-4.3/lib/
LIBS += -L./lib/ffmpeg-n4.3.2-163-g6c414cf8f7-win64-lgpl-shared-4.3/lib/ -lavfilter
LIBS += -L./lib/ffmpeg-n4.3.2-163-g6c414cf8f7-win64-lgpl-shared-4.3/lib/ -lavformat
LIBS += -L./lib/ffmpeg-n4.3.2-163-g6c414cf8f7-win64-lgpl-shared-4.3/lib/ -lavcodec
LIBS += -L./lib/ffmpeg-n4.3.2-163-g6c414cf8f7-win64-lgpl-shared-4.3/lib/ -lavutil
LIBS += -L./lib/ffmpeg-n4.3.2-163-g6c414cf8f7-win64-lgpl-shared-4.3/lib/ -lswresample
LIBS += -L./lib/ffmpeg-n4.3.2-163-g6c414cf8f7-win64-lgpl-shared-4.3/lib/ -lswscale
LIBS += -L./lib/ffmpeg-n4.3.2-163-g6c414cf8f7-win64-lgpl-shared-4.3/lib/ -lavdevice

#LIBS += -L../ffmpeg/lib/ -lavfilter
#LIBS += -L../ffmpeg/lib/ -lavformat
#LIBS += -L../ffmpeg/lib/ -lavcodec
#LIBS += -L../ffmpeg/lib/ -lavutil
#LIBS += -L../ffmpeg/lib/ -lswresample
#LIBS += -L../ffmpeg/lib/ -lswscale
#LIBS += -L../ffmpeg/lib/ -lavdevice
LIBS += -L./lib/libcurl-vc15-x64-release-dll-ipv6-sspi-schannel/lib/ -llibcurl

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
