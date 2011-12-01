#include "gradientrectangle.h"

#include <QPainter>

GradientRectangle::GradientRectangle(QDeclarativeItem *parent)
    : QDeclarativeItem(parent),
      m_percentage(0.0),
      m_brush(0)
{
    setFlag(QGraphicsItem::ItemHasNoContents, false);
    setCacheMode(QGraphicsItem::NoCache);
}

GradientRectangle::~GradientRectangle()
{
    if (m_brush)
        delete m_brush;
}

QVariantList GradientRectangle::colors()
{
    QVariantList colors;
    foreach(QColor color, m_colors) {
        colors << color;
    }
    return colors;
}

void GradientRectangle::setColors(QVariantList colors)
{
    m_colors.clear();
    foreach(QVariant var, colors) {
        QColor color = var.value<QColor>();
        if (color.isValid())
            m_colors << color;
        else
            qWarning() << "Color invalid:" << var;
    }
    delete m_brush;
    m_brush = 0;
    update(boundingRect());
    emit colorsChanged();
}

qreal GradientRectangle::percentage() const
{
    return m_percentage;
}

void GradientRectangle::setPercentage(qreal value)
{
    if ((value >= 0.0) &&
        (value <= 1.0) &&
        (m_percentage != value)) {
        m_percentage = value;
        delete m_brush;
        m_brush = 0;
        update(boundingRect());
        emit percentageChanged();
    }
}

void GradientRectangle::updatePen()
{
    if (m_brush)
        delete m_brush;

    if ((m_percentage  == 0.0) || (m_colors.size() == 0)) {
        m_brush = new QBrush();
    } else {
        QLinearGradient linearGrad(QPointF(0, 0), QPointF(width(), 0));

        qreal offset = 1.0 / m_colors.size();
        qreal point = 0;
        foreach(QColor color, m_colors) {
            linearGrad.setColorAt(point, color);
            point += offset;
        }

        m_brush = new QBrush(linearGrad);
    }
}

void GradientRectangle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QDeclarativeItem::paint(painter, option, widget);
    if (m_percentage <= 0.0) {
        painter->fillRect(boundingRect(), QColor("white"));
    } else {
        if (!m_brush)
            updatePen();

        painter->fillRect(boundingRect(), *m_brush);

        // Clip the right from the indicator position
        if (m_percentage < 1.0) {
            QRectF blank = boundingRect();
            qreal usedWith = blank.width() * m_percentage;
            blank.setWidth(blank.width() - usedWith);
            blank.moveLeft(usedWith);
            painter->eraseRect(blank);
        }
    }
}
