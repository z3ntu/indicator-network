#include "pagestack.h"

#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDeclarativeProperty>

PageStack::PageStack(QDeclarativeItem *parent)
    : QDeclarativeItem(parent),
      m_animation(0)
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


void PageStack::onSizeChanged()
{
    foreach(QDeclarativeItem *menu, m_menus) {
        menu->setWidth(width());
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

    QDeclarativeContext *ctx = QDeclarativeEngine::contextForObject(this);
    QObject *menuObject = component->create(ctx);
    Q_ASSERT(menuObject);

    if (m_menus.size())
        m_oldItem = m_menus.last();
    else
        m_oldItem = 0;

    QDeclarativeItem *menu = qobject_cast<QDeclarativeItem*>(menuObject);
    menu->setParentItem(this);
    menu->setWidth(width());
    menu->setHeight(height());
    m_menus.append(menu);


    // Finalize current Animation
    if (m_animation) {
        m_animation->setLoopCount(m_animation->loopCount());
        delete m_animation;
    }

    // Start a new animation
    m_animation = new QPropertyAnimation(menu, "x");
    m_animation->setDuration(300);
    m_animation->setStartValue(width());
    m_animation->setEndValue(0);
    connect(m_animation, SIGNAL(valueChanged(QVariant)), SLOT(onAnimationValueChanged(QVariant)));
    connect(m_animation, SIGNAL(finished()), SLOT(onAnimationFowardFinished()));

    emit countChanged();
    m_animation->start();
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

    QDeclarativeItem *last = m_menus.pop();
    m_oldItem = m_menus.last();
    m_oldItem->setVisible(true);

    m_animation = new QPropertyAnimation(last, "x");
    m_animation->setDuration(300);
    m_animation->setStartValue(0);
    m_animation->setEndValue(width());
    connect(m_animation, SIGNAL(valueChanged(QVariant)), SLOT(onAnimationValueChanged(QVariant)));
    connect(m_animation, SIGNAL(finished()), SLOT(onAnimationBackFinished()));
    m_oldItem->setVisible(true);

    emit countChanged();
    m_animation->start();
}
