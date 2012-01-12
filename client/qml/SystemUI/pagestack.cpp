#include "pagestack.h"
#include "pageanimation.h"

#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#include <QDeclarativeComponent>
#include <QDeclarativeProperty>

PageStack::PageStack(QDeclarativeItem *parent)
    : QDeclarativeItem(parent),
      m_currentIndex(0),
      m_visiblePages(1),
      m_pageWidth(-1),
      m_animation(0)
{
    QObject::connect(this, SIGNAL(countChanged()), this, SIGNAL(currentIndexChanged()));
    QObject::connect(this, SIGNAL(countChanged()), this, SIGNAL(currentPageChanged()));
    QObject::connect(this, SIGNAL(visiblePagesChanged()), this, SLOT(updateWidth()));
    QObject::connect(this, SIGNAL(pageWidthChanged()), this, SLOT(updateWidth()));
    QObject::connect(this, SIGNAL(spacingChanged()), this, SLOT(updateWidth()));
}

PageStack::~PageStack()
{
    clear();
}

void PageStack::clear()
{
    finishAnimation();
    Q_FOREACH(Page page, m_pages) {
        page.first->setVisible(false);
        page.first->setParent(0);
        page.first->deleteLater();

        page.second->setVisible(false);
        page.second->setParent(0);
        page.second->deleteLater();
    }
    m_pages.clear();
}

int PageStack::count() const
{
    return m_pages.count();
}

int PageStack::currentIndex() const
{
    return m_currentIndex;
}

int PageStack::visiblePages() const
{
    return m_visiblePages;
}

int PageStack::pageWidth() const
{
    return m_pageWidth;
}

int PageStack::spacing() const
{
    return m_spacing;
}

QDeclarativeItem* PageStack::currentPage() const
{
    return m_pages[m_currentIndex].first;
}

void PageStack::setVisiblePages(int value)
{
    if (m_visiblePages != value) {
        m_visiblePages = value;
        Q_EMIT visiblePagesChanged();
    }
}

void PageStack::setPageWidth(int value)
{
    if (m_pageWidth != value) {
        m_pageWidth = value;
        Q_EMIT pageWidthChanged();
    }
}

void PageStack::setSpacing(int value)
{
    if (m_spacing != value) {
        m_spacing = value;
        Q_EMIT spacingChanged();
    }
}

QDeclarativeItem* PageStack::push(QDeclarativeComponent * pageComponent)
{
    QDeclarativeContext *ctx = QDeclarativeEngine::contextForObject(this);

    QObject *pageObject = pageComponent->create(ctx);
    if (!pageObject) {
        qWarning(); << "Fail to load Page:" << pageComponent->errors();
        Q_ASSERT(pageObject);
    }

    QDeclarativeItem *header = 0;

    if (pageObject->property("header").isValid()) {
        QDeclarativeComponent *headerComponent = pageObject->property("header").value<QDeclarativeComponent*>();
        if (headerComponent) {
            ctx = QDeclarativeEngine::contextForObject(pageObject);
            QObject *headerObject = headerComponent->create(ctx);
            if (!headerObject) {
                qWarning(); << "Fail to load Header:" <<  headerComponent->errors();
                Q_ASSERT(headerObject);
            }
            header = qobject_cast<QDeclarativeItem*>(headerObject);
            Q_ASSERT(header);
        }
    }

    if (!header) {
        header = new QDeclarativeItem(this);
    }

    QDeclarativeItem *page = qobject_cast<QDeclarativeItem*>(pageObject);
    addPage(header, page);

    Q_EMIT countChanged();

    startAnimation(QAbstractAnimation::Forward, true);
    return page;
}

void PageStack::pop()
{
    if (m_pages.count() > 0) {
        finishAnimation();

        m_currentIndex = m_pages.size() - 2;

        Q_EMIT countChanged();
        startAnimation(QAbstractAnimation::Backward, true);
    }
}

void PageStack::updateWidth()
{
    setImplicitWidth(m_visiblePages * (m_pageWidth + m_spacing));
    if (m_pages.count() > 0) {
        startAnimation(QAbstractAnimation::Forward, false);
    }
}

void PageStack::geometryChanged ( const QRectF & newGeometry, const QRectF & oldGeometry )
{
    if (newGeometry.height()  != oldGeometry.height()) {
        Q_FOREACH(Page page, m_pages) {
            page.second->setHeight(newGeometry.height() - page.second->y());
        }
    }
    QDeclarativeItem::geometryChanged(newGeometry, oldGeometry);
}

void PageStack::finishAnimation()
{
    if (m_animation) {
        m_animation->setCurrentTime(m_animation->totalDuration());
    }
}

void PageStack::onAnimationFinished()
{
    int count = m_pages.size() - (m_currentIndex + 1);
    while(count) {
        Page page = m_pages.pop();
        pageUnloaded(page.second);
        page.first->setVisible(false);
        page.first->setParentItem(0);
        page.first->deleteLater();

        page.second->setVisible(false);
        page.second->setParentItem(0);
        page.second->deleteLater();
        count--;
    }

    if (m_animation) {
        m_animation->deleteLater();
        m_animation = 0;
    }
    setEnabled(true);
}

void PageStack::startAnimation(QAbstractAnimation::Direction direction, bool move)
{
    finishAnimation();

    setEnabled(false);
    if (m_pages.count() > m_visiblePages) {

        QStack<QDeclarativeItem * > headers;
        QStack<QDeclarativeItem * > pages;
        QList<qreal> startValues;
        QList<qreal> endValues;

        if (move) {
            int offset = (m_pageWidth + m_spacing) * (direction == QAbstractAnimation::Backward ? 1 : -1);
            prepareMoveAnimation(offset, headers, pages, startValues, endValues);
        } else {
            prepareArrangeAnimation(headers, pages, startValues, endValues);
        }

        m_animation = new QParallelAnimationGroup(this);
        PageAnimation *headerAnimation = new PageAnimation(headers, m_animation);
        headerAnimation->setEasingCurve(QEasingCurve::InOutExpo);
        headerAnimation->setStartValue(startValues);
        headerAnimation->setEndValue(endValues);
        headerAnimation->setDuration(600);

        PageAnimation *pageAnimation = new PageAnimation(pages, m_animation);
        pageAnimation->setEasingCurve(QEasingCurve::InOutExpo);
        pageAnimation->setStartValue(startValues);
        pageAnimation->setEndValue(endValues);
        pageAnimation->setDuration(900);

        QObject::connect(m_animation, SIGNAL(finished()), this, SLOT(onAnimationFinished()));
        m_animation->start();
    } else {
        onAnimationFinished();
    }
}

void PageStack::prepareMoveAnimation(int distance, QStack<QDeclarativeItem * > & headers, QStack<QDeclarativeItem * > & pages, QList<qreal> & startValues, QList<qreal> & endValues)
{
    Q_FOREACH(Page page, m_pages) {
        headers << page.first;
        pages << page.second;

        startValues << page.second->x();
        endValues << (page.second->x() + distance);
    }
}

void PageStack::prepareArrangeAnimation(QStack<QDeclarativeItem * > & headers, QStack<QDeclarativeItem * > & pages, QList<qreal> & startValues, QList<qreal> & endValues)
{
    int pageIndex = 0;
    Q_FOREACH(Page page, m_pages) {
        headers << page.first;
        pages << page.second;

        startValues << page.second->x();

        int indexOffset = m_currentIndex < m_visiblePages ? pageIndex : (pageIndex - (m_currentIndex - (m_visiblePages - 1)));
        int xOffset = indexOffset * (m_pageWidth + m_spacing);
        endValues << xOffset;
        pageIndex++;
    }
}

void PageStack::addPage(QDeclarativeItem * header, QDeclarativeItem * page )
{
    finishAnimation();
    m_pages << qMakePair(header, page);
    int pageIndex = m_pages.count() - 1;

    int indexOffset = m_currentIndex < m_visiblePages ? pageIndex : (pageIndex - (m_currentIndex - (m_visiblePages - 1)));
    int xOffset =  indexOffset * (m_pageWidth + m_spacing);

    header->setParentItem(this);
    header->setY(0);
    header->setWidth(m_pageWidth);
    header->setX(xOffset);

    page->setParentItem(this);
    page->setY(header->height() + m_spacing);
    page->setWidth(m_pageWidth);
    page->setHeight(height() - page->y());
    page->setX(xOffset);
    QDeclarativeProperty::write(page, "index", pageIndex);


    m_currentIndex = pageIndex;
    Q_EMIT pageLoaded(page);
}
