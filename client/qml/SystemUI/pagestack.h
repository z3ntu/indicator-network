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
    Q_PROPERTY(int pageWidth READ pageWidth WRITE setPageWidth NOTIFY pageWidthChanged)
    Q_PROPERTY(int spacing READ spacing WRITE setSpacing NOTIFY spacingChanged)
    Q_PROPERTY(QDeclarativeItem* currentPage READ currentPage NOTIFY countChanged)
    Q_PROPERTY(Layout layout READ layout WRITE setLayout NOTIFY layoutChanged)
public:
    PageStack(QDeclarativeItem *parent = 0);
    ~PageStack();

    Q_INVOKABLE void push(QDeclarativeComponent *item);
    Q_INVOKABLE void pop();

    Q_ENUMS(Layout)
    enum Layout {
        Stage,
        Slider
    };

    int count() const;
    QDeclarativeItem* currentPage() const;
    Layout layout() const;
    void setLayout(Layout value);
    int pageWidth() const;
    void setPageWidth(int width);
    int spacing() const;
    void setSpacing(int value);


Q_SIGNALS:
    void countChanged();
    void layoutChanged();
    void pageWidthChanged();
    void spacingChanged();
    void aboutToInsertPage();
    void aboutToRemovePage();

private Q_SLOTS:
    void onAnimationValueChanged(const QVariant &value);
    void onAnimationBackFinished();
    void onAnimationFowardFinished();
    void onSizeChanged();

private:
    QStack<QDeclarativeItem *> m_menus;
    QDeclarativeItem *m_oldItem;
    QPropertyAnimation *m_animation;
    Layout m_layout;
    int m_pageWidth;
    int m_spacing;
};

#endif
