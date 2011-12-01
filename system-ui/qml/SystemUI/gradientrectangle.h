#ifndef GRADIENTRECTANGLE_H
#define GRADIENTRECTANGLE_H

#include <QDeclarativeItem>
#include <QColor>

class GradientRectangle : public QDeclarativeItem
{
    Q_OBJECT
    Q_PROPERTY(QVariantList colors READ colors WRITE setColors NOTIFY colorsChanged)
    Q_PROPERTY(qreal percentage READ percentage WRITE setPercentage NOTIFY percentageChanged)
public:
    GradientRectangle(QDeclarativeItem *parent = 0);
    ~GradientRectangle();

    QVariantList colors();
    void setColors(QVariantList colors);

    qreal percentage() const;
    void setPercentage(qreal value);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

signals:
    void percentageChanged();
    void colorsChanged();

private:
    qreal m_percentage;
    QList<QColor> m_colors;
    QBrush *m_brush;

    void updatePen();
};

#endif
