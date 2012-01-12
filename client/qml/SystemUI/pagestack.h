#ifndef PAGESTACK_H
#define PAGESTACK_H

#include <QDeclarativeItem>
#include <QVariant>
#include <QParallelAnimationGroup>
#include <QStack>

typedef QPair<QDeclarativeItem *, QDeclarativeItem *> Page;

class PageStack : public QDeclarativeItem
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(int visiblePages READ visiblePages WRITE setVisiblePages NOTIFY visiblePagesChanged)
    Q_PROPERTY(int pageWidth READ pageWidth WRITE setPageWidth NOTIFY pageWidthChanged)
    Q_PROPERTY(int spacing READ spacing WRITE setSpacing NOTIFY spacingChanged)
    Q_PROPERTY(QDeclarativeItem* currentPage READ currentPage NOTIFY currentPageChanged)

public:
    PageStack(QDeclarativeItem *parent = 0);
    ~PageStack();

    int count() const;
    int currentIndex() const;
    int visiblePages() const;
    int pageWidth() const;
    int spacing() const;
    QDeclarativeItem* currentPage() const;

    void setVisiblePages(int value);
    void setPageWidth(int value);
    void setSpacing(int value);

    Q_INVOKABLE QDeclarativeItem* push( QDeclarativeComponent * page );
    Q_INVOKABLE void pop();
    Q_INVOKABLE void clear();

Q_SIGNALS:
    void countChanged();
    void currentIndexChanged();
    void currentPageChanged();
    void visiblePagesChanged();
    void pageWidthChanged();
    void spacingChanged();
    void pageLoaded( QDeclarativeItem * page );
    void pageUnloaded( QDeclarativeItem * page );

private Q_SLOTS:
    void onAnimationFinished();
    void onHeaderHeightChanged();
    void updateWidth();

protected:
    void geometryChanged ( const QRectF & newGeometry, const QRectF & oldGeometry );

private:
    QStack<Page > m_pages;
    int m_currentIndex;
    int m_visiblePages;
    int m_pageWidth;
    int m_spacing;
    QParallelAnimationGroup *m_animation;

    void finishAnimation();
    void startAnimation( QAbstractAnimation::Direction direction, bool move );
    void addPage( QDeclarativeItem * header, QDeclarativeItem * page );
    void prepareMoveAnimation( int distance, QStack<QDeclarativeItem * > & headers, QStack<QDeclarativeItem * > & pages, QList<qreal> & startValues, QList<qreal> & endValues );
    void prepareArrangeAnimation( QStack<QDeclarativeItem * > & headers, QStack<QDeclarativeItem * > & pages, QList<qreal> & startValues, QList<qreal> & endValues );
};

#endif
