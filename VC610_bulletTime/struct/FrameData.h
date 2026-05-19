// FrameData.h
#pragma once

#include <QImage>

struct FrameData {
    QImage FrameImage;
    int fps = 60;
    int FrameCount = 0;
};
