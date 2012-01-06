#include "pageanimation.h"

PageAnimation::PageAnimation( QStack<QDeclarativeItem * > items, QObject * parent )
    : QVariantAnimation(parent),
      m_items(items)
{
    QVariantAnimation::setStartValue(0.0);
    QVariantAnimation::setEndValue(1.0);
}

void PageAnimation::setStartValue(QList<qreal> startValues)
{
    m_startValues = startValues;
}

void PageAnimation::setEndValue(QList<qreal> endValues)
{
    m_endValues = endValues;
}

void PageAnimation::updateCurrentValue ( const QVariant & value )
{
    if ((m_startValues.count() != m_endValues.count()) ||
        (m_startValues.count() != m_items.count()))
        return;

    int index = 0;

    Q_FOREACH(QDeclarativeItem *item, m_items) {
        qreal itemX = m_startValues[index] + ((m_endValues[index] - m_startValues[index]) * value.toReal());
        item->setX(itemX);
        index++;
    }
}
