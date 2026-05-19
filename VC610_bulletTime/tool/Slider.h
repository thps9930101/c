#include <QSlider>
#include <QMouseEvent>
#include <QStyle>

class ClickableSlider : public QSlider {
    Q_OBJECT
public:
    using QSlider::QSlider;

protected:
    void mousePressEvent(QMouseEvent* e) override {
        if (orientation() == Qt::Horizontal) {
            int sliderMin = minimum();
            int sliderMax = maximum();
            int w = width();
            int x = e->pos().x();
            double ratio = double(x) / double(w);
            ratio = qBound(0.0, ratio, 1.0);
            int newVal = sliderMin + qRound(ratio * (sliderMax - sliderMin));
            setValue(newVal); // 這會發 emit valueChanged / sliderMoved 視你的連線方式
            // 你可以選擇在這裡手動 emit sliderReleased 之後播放邏輯（通常 setValue 夠了）
        }
        QSlider::mousePressEvent(e); // 保留原行為（例如 handle 拖曳）
    }
};
