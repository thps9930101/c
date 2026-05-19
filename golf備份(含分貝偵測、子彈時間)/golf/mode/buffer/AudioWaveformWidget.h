#ifndef AUDIOWAVEFORMWIDGET_H
#define AUDIOWAVEFORMWIDGET_H

#include <QWidget>
#include <QVector>
#include <QTimer>

class AudioWaveformWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AudioWaveformWidget(QWidget *parent = nullptr);

    void addSample(qint16 sample);  // 每次加入一筆音訊樣本

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<qint16> samples;
    const int maxSamples = 1024;
};

#endif // AUDIOWAVEFORMWIDGET_H
