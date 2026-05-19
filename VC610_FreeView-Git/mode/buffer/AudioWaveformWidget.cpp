#include "AudioWaveformWidget.h"
#include <QPainter>

AudioWaveformWidget::AudioWaveformWidget(QWidget *parent)
    : QWidget(parent)
{
    samples.fill(0, maxSamples);
}

void AudioWaveformWidget::addSample(qint16 sample)
{
    if (samples.size() >= maxSamples)
        samples.pop_front();

    samples.append(sample);
    update();  // 觸發 repaint
}

void AudioWaveformWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);
    painter.setPen(Qt::green);

    int midY = height() / 2;
    double xStep = static_cast<double>(width()) / samples.size();

    for (int i = 1; i < samples.size(); ++i) {
        int x1 = (i - 1) * xStep;
        int y1 = midY - samples[i - 1] * midY / 32768.0;
        int x2 = i * xStep;
        int y2 = midY - samples[i] * midY / 32768.0;
        painter.drawLine(x1, y1, x2, y2);
    }
}
