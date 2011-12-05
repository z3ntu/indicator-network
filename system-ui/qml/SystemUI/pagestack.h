#ifndef PAGESTACK_H
#define PAGESTACK_H

#include <QDeclarativeItem>
#include <QDeclarativeComponent>
#include <QStack>
#include <QPropertyAnimation>

class PageStack : public QDeclarativeItem
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QDeclarativeItem* currentPage READ currentPage NOTIFY countChanged)
public:
    PageStack(QDeclarativeItem *parent = 0);
    ~PageStack();

    Q_INVOKABLE void push(QDeclarativeComponent *item);
    Q_INVOKABLE void pop();

    int count() const;
    QDeclarativeItem* currentPage() const;

Q_SIGNALS:
    void countChanged();

private Q_SLOTS:
    void onAnimationValueChanged(const QVariant &value);
    void onAnimationBackFinished();
    void onAnimationFowardFinished();
    void onSizeChanged();

private:
    QStack<QDeclarativeItem *> m_menus;
    QDeclarativeItem *m_oldItem;
    QPropertyAnimation *m_animation;
};

#endif
