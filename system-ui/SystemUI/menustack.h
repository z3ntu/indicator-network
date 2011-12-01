#ifndef MENUSTACK_H
#define MENUSTACK_H

#include <QDeclarativeItem>
#include <QDeclarativeComponent>
#include <QStack>
#include <QPropertyAnimation>

class MenuStack : public QDeclarativeItem
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
public:
    MenuStack(QDeclarativeItem *parent = 0);
    ~MenuStack();

    Q_INVOKABLE void pushMenu(QDeclarativeComponent *item);
    Q_INVOKABLE void popMenu();

    int count() const;

signals:
    void countChanged();

private slots:
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
