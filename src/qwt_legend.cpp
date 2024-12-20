/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

// vim: expandtab

#include <qapplication.h>
#include <qmap.h>
#include <qscrollbar.h>
#include "qwt_math.h"
#include "qwt_dyngrid_layout.h"
#include "qwt_legend_itemmanager.h"
#include "qwt_legend_item.h"
#include "qwt_legend.h"

class QwtLegend::PrivateData
{
public:
    class LegendMap
    {
    public:
        void insert(const QwtLegendItemManager *, QWidget *);

        void remove(const QwtLegendItemManager *);
        void remove(QWidget *);

        void clear();

        uint count() const;

        inline const QWidget *find(const QwtLegendItemManager *) const;
        inline QWidget *find(const QwtLegendItemManager *);

        inline const QwtLegendItemManager *find(const QWidget *) const;
        inline QwtLegendItemManager *find(const QWidget *);

        const QMap<QWidget *, const QwtLegendItemManager *> &widgetMap() const;
        QMap<QWidget *, const QwtLegendItemManager *> &widgetMap();

    private:
        QMap<QWidget *, const QwtLegendItemManager *> d_widgetMap;
        QMap<const QwtLegendItemManager *, QWidget *> d_itemMap;
    };

    QwtLegend::LegendItemMode itemMode;
    QwtLegend::LegendDisplayPolicy displayPolicy;
    int identifierMode;

    LegendMap map;

    class LegendView;
    LegendView *view;
};

#include <qscrollarea.h>

class QwtLegend::PrivateData::LegendView: public QScrollArea
{
public:
    LegendView(QWidget *parent):
        QScrollArea(parent)
    {
        contentsWidget = new QWidget(this);

        setWidget(contentsWidget);
        setWidgetResizable(false);
        setFocusPolicy(Qt::NoFocus);
    }

    virtual bool viewportEvent(QEvent *e) 
    {
        bool ok = QScrollArea::viewportEvent(e);

        if ( e->type() == QEvent::Resize )
        {
            QEvent event(QEvent::LayoutRequest);
            QApplication::sendEvent(contentsWidget, &event);
        }
        return ok;
    }

    QSize viewportSize(int w, int h) const
    {
        const int sbHeight = horizontalScrollBar()->sizeHint().height();
        const int sbWidth = verticalScrollBar()->sizeHint().width();
    
        const int cw = contentsRect().width();
        const int ch = contentsRect().height();

        int vw = cw;
        int vh = ch;

        if ( w > vw )
            vh -= sbHeight;

        if ( h > vh )
        {
            vw -= sbWidth;
            if ( w > vw && vh == ch )
                vh -= sbHeight;
        }
        return QSize(vw, vh);
    }

    QWidget *contentsWidget;
};

void QwtLegend::PrivateData::LegendMap::insert(
    const QwtLegendItemManager *item, QWidget *widget)
{
    d_itemMap.insert(item, widget);
    d_widgetMap.insert(widget, item);
}

void QwtLegend::PrivateData::LegendMap::remove(const QwtLegendItemManager *item)
{
    QWidget *widget = d_itemMap[item];
    d_itemMap.remove(item);
    d_widgetMap.remove(widget);
}

void QwtLegend::PrivateData::LegendMap::remove(QWidget *widget)
{
    const QwtLegendItemManager *item = d_widgetMap[widget];
    d_itemMap.remove(item);
    d_widgetMap.remove(widget);
}

void QwtLegend::PrivateData::LegendMap::clear()
{
    
    /*
       We can't delete the widgets in the following loop, because
       we would get ChildRemoved events, changing d_itemMap, while
       we are iterating.
     */

    QList<QWidget *> widgets;

    QMap<const QwtLegendItemManager *, QWidget *>::const_iterator it;
    for ( it = d_itemMap.begin(); it != d_itemMap.end(); ++it ) 
        widgets.append(it.value());

    d_itemMap.clear();
    d_widgetMap.clear();

    for ( int i = 0; i < (int)widgets.size(); i++ )
        delete widgets[i];
}

uint QwtLegend::PrivateData::LegendMap::count() const
{
    return d_itemMap.count();
}

inline const QWidget *QwtLegend::PrivateData::LegendMap::find(const QwtLegendItemManager *item) const
{
    if ( !d_itemMap.contains((QwtLegendItemManager *)item) )
        return nullptr;

    return d_itemMap[(QwtLegendItemManager *)item];
}

inline QWidget *QwtLegend::PrivateData::LegendMap::find(const QwtLegendItemManager *item)
{
    if ( !d_itemMap.contains((QwtLegendItemManager *)item) )
        return nullptr;

    return d_itemMap[(QwtLegendItemManager *)item];
}

inline const QwtLegendItemManager *QwtLegend::PrivateData::LegendMap::find(
    const QWidget *widget) const
{
    if ( !d_widgetMap.contains((QWidget *)widget) )
        return nullptr;

    return d_widgetMap[(QWidget *)widget];
}

inline QwtLegendItemManager *QwtLegend::PrivateData::LegendMap::find(
    const QWidget *widget)
{
    if ( !d_widgetMap.contains((QWidget *)widget) )
        return nullptr;

    return (QwtLegendItemManager *)d_widgetMap[(QWidget *)widget];
}

inline const QMap<QWidget *, const QwtLegendItemManager *> &
    QwtLegend::PrivateData::LegendMap::widgetMap() const
{
    return d_widgetMap;
} 

inline QMap<QWidget *, const QwtLegendItemManager *> &
    QwtLegend::PrivateData::LegendMap::widgetMap() 
{
    return d_widgetMap;
} 

/*!
  Constructor

  \param parent Parent widget
*/
QwtLegend::QwtLegend(QWidget *parent): 
    QFrame(parent)
{
    setFrameStyle(NoFrame);

    d_data = new QwtLegend::PrivateData;
    d_data->itemMode = QwtLegend::ReadOnlyItem;
    d_data->displayPolicy = QwtLegend::AutoIdentifier;
    d_data->identifierMode = QwtLegendItem::ShowLine | 
        QwtLegendItem::ShowSymbol | QwtLegendItem::ShowText;

    d_data->view = new QwtLegend::PrivateData::LegendView(this);
    d_data->view->setFrameStyle(NoFrame);

    QwtDynGridLayout *layout = new QwtDynGridLayout(
        d_data->view->contentsWidget);
    layout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    d_data->view->contentsWidget->installEventFilter(this);
}

//! Destructor
QwtLegend::~QwtLegend()
{
    delete d_data;
}

/*!
  Set the legend display policy to:

  \param policy Legend display policy
  \param mode Identifier mode (or'd ShowLine, ShowSymbol, ShowText)

  \sa displayPolicy(), LegendDisplayPolicy
*/
void QwtLegend::setDisplayPolicy(LegendDisplayPolicy policy, int mode)
{
    d_data->displayPolicy = policy;
    if (-1 != mode)
       d_data->identifierMode = mode;

    QMap<QWidget *, const QwtLegendItemManager *> &map = 
        d_data->map.widgetMap();

    QMap<QWidget *, const QwtLegendItemManager *>::iterator it;
    for ( it = map.begin(); it != map.end(); ++it ) 
    {
        QwtLegendItemManager *item = (QwtLegendItemManager *)it.value();
        if ( item )
            item->updateLegend(this);
    }
}

/*! 
  \return the legend display policy.
  Default is LegendDisplayPolicy::Auto.
  \sa setDisplayPolicy(), LegendDisplayPolicy
*/ 

QwtLegend::LegendDisplayPolicy QwtLegend::displayPolicy() const 
{ 
    return d_data->displayPolicy; 
}

//! \sa LegendItemMode
void QwtLegend::setItemMode(LegendItemMode mode)
{
    d_data->itemMode = mode;
}

//! \sa LegendItemMode
QwtLegend::LegendItemMode QwtLegend::itemMode() const
{
    return d_data->itemMode;
}

/*!
  \return the IdentifierMode to be used in combination with
  LegendDisplayPolicy::Fixed.

  Default is ShowLine | ShowSymbol | ShowText.
*/
int QwtLegend::identifierMode() const
{
    return d_data->identifierMode;
}

/*! 
  The contents widget is the only child of the viewport() and
  the parent widget of all legend items.
*/
QWidget *QwtLegend::contentsWidget() 
{ 
    return d_data->view->contentsWidget; 
}

/*! 
  \return Horizontal scrollbar
  \sa verticalScrollBar()
*/
QScrollBar *QwtLegend::horizontalScrollBar() const
{
    return d_data->view->horizontalScrollBar();
}

/*! 
  \return Vertical scrollbar
  \sa horizontalScrollBar()
*/
QScrollBar *QwtLegend::verticalScrollBar() const
{
    return d_data->view->verticalScrollBar();
}

/*!  
  The contents widget is the only child of the viewport() and
  the parent widget of all legend items.
*/
const QWidget *QwtLegend::contentsWidget() const 
{ 
    return d_data->view->contentsWidget; 
}

/*!
  Insert a new item for a plot item
  \param plotItem Plot item
  \param legendItem New legend item
  \note The parent of item will be changed to QwtLegend::contentsWidget()
*/
void QwtLegend::insert(const QwtLegendItemManager *plotItem, QWidget *legendItem)
{
    if ( legendItem == nullptr || plotItem == nullptr )
        return;

    QWidget *contentsWidget = d_data->view->contentsWidget;

    if ( legendItem->parent() != contentsWidget )
    {
        legendItem->setParent(contentsWidget);
    }

    legendItem->show();

    d_data->map.insert(plotItem, legendItem);

    layoutContents();

    if ( contentsWidget->layout() )
    {
        contentsWidget->layout()->addWidget(legendItem);

        // set tab focus chain

        QWidget *w = nullptr;

        for (int i = 0; i < contentsWidget->layout()->count(); i++)
        {
            QLayoutItem *item = contentsWidget->layout()->itemAt(i);
            if ( w && item->widget() )
            {
                QWidget::setTabOrder(w, item->widget());
                w = item->widget();
            }
        }
    }
    if ( parentWidget() && parentWidget()->layout() == nullptr )
    {
       /*
          updateGeometry() doesn't post LayoutRequest in certain
          situations, like when we are hidden. But we want the
          parent widget notified, so it can show/hide the legend
          depending on its items.
        */
        QApplication::postEvent(parentWidget(),
            new QEvent(QEvent::LayoutRequest));
    }
}

/*!
  Find the widget that represents a plot item

  \param plotItem Plot item
  \return Widget on the legend, or nullptr
*/
QWidget *QwtLegend::find(const QwtLegendItemManager *plotItem) const
{
    return d_data->map.find(plotItem);
}

/*!
  Find the widget that represents a plot item

  \param legendItem Legend item
  \return Widget on the legend, or nullptr
*/
QwtLegendItemManager *QwtLegend::find(const QWidget *legendItem) const
{
    return d_data->map.find(legendItem);
}

/*! 
   Find the corresponding item for a plotItem and remove it 
   from the item list.

   \param plotItem Plot item
*/
void QwtLegend::remove(const QwtLegendItemManager *plotItem)
{ 
    QWidget *legendItem = d_data->map.find(plotItem);
    d_data->map.remove(legendItem); 
    delete legendItem;
}

//! Remove all items.
void QwtLegend::clear()
{
    bool doUpdate = updatesEnabled();
    setUpdatesEnabled(false);

    d_data->map.clear();

    setUpdatesEnabled(doUpdate);
    update();
}

//! Return a size hint.
QSize QwtLegend::sizeHint() const
{
    QSize hint = d_data->view->contentsWidget->sizeHint();
    hint += QSize(2 * frameWidth(), 2 * frameWidth());

    return hint;
}

/*!
  \return The preferred height, for the width w.
  \param width Width
*/
int QwtLegend::heightForWidth(int width) const
{
    width -= 2 * frameWidth();

    int h = d_data->view->contentsWidget->heightForWidth(width);
    if ( h >= 0 )
        h += 2 * frameWidth();

    return h;
}

/*!
  Adjust contents widget and item layout to the size of the viewport().
*/
void QwtLegend::layoutContents()
{
    const QSize visibleSize = d_data->view->viewport()->size();

    const QLayout *l = d_data->view->contentsWidget->layout();
    if ( l && l->inherits("QwtDynGridLayout") )
    {
        const QwtDynGridLayout *tl = (const QwtDynGridLayout *)l;

        auto margin = std::max({tl->contentsMargins().left(), tl->contentsMargins().right(),
                                tl->contentsMargins().top(), tl->contentsMargins().bottom()});
        const int minW = int(tl->maxItemWidth()) + 2 * margin;

        int w = qwtMax(visibleSize.width(), minW);
        int h = qwtMax(tl->heightForWidth(w), visibleSize.height());

        const int vpWidth = d_data->view->viewportSize(w, h).width();
        if ( w > vpWidth )
        {
            w = qwtMax(vpWidth, minW);
            h = qwtMax(tl->heightForWidth(w), visibleSize.height());
        }

        d_data->view->contentsWidget->resize(w, h);
    }
}

/*!
  Filter layout related events of QwtLegend::contentsWidget().

  \param o Object to be filtered
  \param e Event
*/
bool QwtLegend::eventFilter(QObject *o, QEvent *e)
{
    if ( o == d_data->view->contentsWidget )
    {
        switch(e->type())
        {
            case QEvent::ChildRemoved:
            {   
                const QChildEvent *ce = (const QChildEvent *)e;
                if ( ce->child()->isWidgetType() )
                    d_data->map.remove((QWidget *)ce->child());
                break;
            }
            case QEvent::LayoutRequest:
            {
                layoutContents();
                break;
            }
            default:
                break;
        }
    }
    
    return QFrame::eventFilter(o, e);
}


//! Return true, if there are no legend items.
bool QwtLegend::isEmpty() const
{
    return d_data->map.count() == 0;
}

//! Return the number of legend items.
uint QwtLegend::itemCount() const
{
    return d_data->map.count();
}

//! Return a list of all legend items
QList<QWidget *> QwtLegend::legendItems() const
{
    const QMap<QWidget *, const QwtLegendItemManager *> &map = 
        d_data->map.widgetMap();

    QList<QWidget *> list;

    QMap<QWidget *, const QwtLegendItemManager *>::const_iterator it;
    for ( it = map.begin(); it != map.end(); ++it ) 
        list += it.key();

    return list;
}

/*!
   Resize event
   \param e Resize event
*/
void QwtLegend::resizeEvent(QResizeEvent *e)
{
    QFrame::resizeEvent(e);
    d_data->view->setGeometry(contentsRect());
}
