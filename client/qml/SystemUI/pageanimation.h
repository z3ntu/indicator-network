#ifndef PAGEANIMATION_H
#define PAGEANIMATION_H

#include <QVariantAnimation>
#include <QDeclarativeItem>
#include <QStack>

class PageAnimation : public QVariantAnimation
{
    Q_OBJECT
public:
    PageAnimation( QStack<QDeclarativeItem*> items, QObject * parent = 0 );

    void setStartValue( QList< qreal > startValue );
    void setEndValue( QList< qreal > endValue );

protected:
    void updateCurrentValue ( const QVariant & value );

private:
    QStack<QDeclarativeItem*> m_items;
    QList<qreal> m_startValues;
    QList<qreal> m_endValues;
};

#endif
