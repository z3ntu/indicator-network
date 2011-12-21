#include "pagestack.h"

#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDeclarativeProperty>

PageStack::PageStack(QDeclarativeItem *parent)
    : QDeclarativeItem(parent),
      m_animation(0),
      m_layout(Stage),
      m_pageWidth(300),
      m_spacing(6)
{
    QObject::connect(this, SIGNAL(widthChanged()), this, SLOT(onSizeChanged()));
    QObject::connect(this, SIGNAL(heightChanged()), this, SLOT(onSizeChanged()));
}

PageStack::~PageStack()
{
    delete m_animation;
    m_menus.clear();
}

int PageStack::count() const
{
    return m_menus.count();
}

QDeclarativeItem* PageStack::currentPage() const
{
    return m_menus.last();
}

PageStack::Layout PageStack::layout() const
{
    return m_layout;
}

void PageStack::setLayout(Layout value)
{
    if (m_layout != value) {
        m_layout = value;
        Q_EMIT layoutChanged();
    }
}

int PageStack::pageWidth() const
{
    return m_pageWidth;
}

void PageStack::setPageWidth(int width)
{
    if (width != m_pageWidth) {
        m_pageWidth = width;
        Q_EMIT pageWidthChanged();
    }
}

int PageStack::spacing() const
{
    return m_spacing;
}

void PageStack::setSpacing(int value)
{
    if (value != m_spacing) {
        m_spacing = value;
        Q_EMIT spacingChanged();
    }
}

void PageStack::onSizeChanged()
{
    Q_FOREACH(QDeclarativeItem *menu, m_menus) {
        if (m_layout == Slider) {
            menu->setWidth(width());
        }
        menu->setHeight(height());
    }
}

void PageStack::onAnimationValueChanged(const QVariant &value)
{
    if (m_oldItem)
        m_oldItem->setX(value.toReal() - width());
}

void PageStack::onAnimationFowardFinished()
{
    if(m_oldItem)
        m_oldItem->setVisible(false);
}

void PageStack::onAnimationBackFinished()
{
    m_animation->targetObject()->deleteLater();
    m_oldItem = m_menus.last();
}

void PageStack::push(QDeclarativeComponent *component)
{
    if (!component)
        return;

    Q_EMIT aboutToInsertPage();

    QDeclarativeContext *ctx = QDeclarativeEngine::contextForObject(this);
    QObject *menuObject = component->create(ctx);
    if (!menuObject) {
        qDebug() << "Fail to load component:" << component->errors();
        Q_ASSERT(menuObject);
    }

    if (m_menus.size())
        m_oldItem = m_menus.last();
    else
        m_oldItem = 0;

    QDeclarativeItem *menu = qobject_cast<QDeclarativeItem*>(menuObject);
    menu->setParentItem(this);
    menu->setWidth(m_pageWidth);
    menu->setHeight(height());
    m_menus.append(menu);

    // Finalize current Animation
    if (m_animation) {
        m_animation->setLoopCount(m_animation->loopCount());
        delete m_animation;
    }

    if (m_layout == Slider) {
        setWidth(m_pageWidth);

        // Start a new animation
        m_animation = new QPropertyAnimation(menu, "x");
        m_animation->setDuration(300);
        m_animation->setStartValue(width());
        m_animation->setEndValue(0);
        connect(m_animation, SIGNAL(valueChanged(QVariant)), SLOT(onAnimationValueChanged(QVariant)));
        connect(m_animation, SIGNAL(finished()), SLOT(onAnimationFowardFinished()));
        m_animation->start();
    } else if (m_layout == Stage) {
        int widthx = (m_menus.size() * (m_pageWidth + m_spacing));
        menu->setX(widthx - menu->width());
        setWidth(widthx);
    }

    Q_EMIT countChanged();
}

void PageStack::pop()
{
    if ((m_menus.size() <= 0) || !m_oldItem)
        return;

    // Finalize current Animation
    if (m_animation) {
        m_animation->setLoopCount(m_animation->loopCount());
        delete m_animation;
    }

    Q_EMIT aboutToRemovePage();

    QDeclarativeItem *last = m_menus.pop();
    m_oldItem = m_menus.last();

    if (m_layout == Slider) {
        m_oldItem->setVisible(true);
        m_animation = new QPropertyAnimation(last, "x");
        m_animation->setDuration(300);
        m_animation->setStartValue(0);
        m_animation->setEndValue(width());
        connect(m_animation, SIGNAL(valueChanged(QVariant)), SLOT(onAnimationValueChanged(QVariant)));
        connect(m_animation, SIGNAL(finished()), SLOT(onAnimationBackFinished()));
        m_oldItem->setVisible(true);
        m_animation->start();
    } else if (m_layout == Stage) {
        int width = (m_menus.size() * (m_pageWidth + m_spacing));
        setWidth(width);
        last->deleteLater();
        last = 0;
    }

    Q_EMIT countChanged();
}
