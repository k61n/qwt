/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

// vim: expandtab

#include <qpainter.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qpaintengine.h>
#ifdef Q_WS_X11
#include <qx11info_x11.h>
#endif
#include <qevent.h>
#include "qwt_painter.h"
#include "qwt_math.h"
#include "qwt_plot.h"
#include "qwt_plot_canvas.h"

class QwtPlotCanvas::PrivateData
{
public:
    PrivateData():
        focusIndicator(NoFocusIndicator),
        paintAttributes(0),
        cache(nullptr)
    {
    }

    ~PrivateData()
    {
        delete cache;
    }

    FocusIndicator focusIndicator;
    int paintAttributes;
    QPixmap *cache;
};

//! Sets a cross cursor, enables QwtPlotCanvas::PaintCached

QwtPlotCanvas::QwtPlotCanvas(QwtPlot *plot):
    QFrame(plot)
{
    d_data = new PrivateData;

    setAutoFillBackground(true);

#ifndef QT_NO_CURSOR
    setCursor(Qt::CrossCursor);
#endif

    setPaintAttribute(PaintCached, true);
    setPaintAttribute(PaintPacked, true);
}

//! Destructor
QwtPlotCanvas::~QwtPlotCanvas()
{
    delete d_data;
}

//! Return parent plot widget
QwtPlot *QwtPlotCanvas::plot()
{
    QWidget *w = parentWidget();
    if ( w && w->inherits("QwtPlot") )
        return (QwtPlot *)w;

    return nullptr;
}

//! Return parent plot widget
const QwtPlot *QwtPlotCanvas::plot() const
{
    const QWidget *w = parentWidget();
    if ( w && w->inherits("QwtPlot") )
        return (QwtPlot *)w;

    return nullptr;
}

/*!
  \brief Changing the paint attributes

  \param attribute Paint attribute
  \param on On/Off

  The default setting enables PaintCached and PaintPacked

  \sa testPaintAttribute(), drawCanvas(), drawContents(), paintCache()
*/
void QwtPlotCanvas::setPaintAttribute(PaintAttribute attribute, bool on)
{
    if ( bool(d_data->paintAttributes & attribute) == on )
        return;

    if ( on )
        d_data->paintAttributes |= attribute;
    else
        d_data->paintAttributes &= ~attribute;

    switch(attribute)
    {
        case PaintCached:
        {
            if ( on )
            {
                if ( d_data->cache == nullptr )
                    d_data->cache = new QPixmap();

                if ( isVisible() )
                {
                    const QRect cr = contentsRect();
#if QT_VERSION < 0x050f00
                    *d_data->cache = QPixmap::grabWidget(this,
                        cr.x(), cr.y(), cr.width(), cr.height() );
#else
                    *d_data->cache = QWidget::grab(QRect(cr.x(), cr.y(), cr.width(), cr.height()));
#endif
                }
            }
            else
            {
                delete d_data->cache;
                d_data->cache = nullptr;
            }
            break;
        }
        case PaintPacked:
        {
            /*
              If not visible, changing of the background mode
              is delayed until it becomes visible. This tries to avoid 
              looking through the canvas when the canvas is shown the first 
              time.
             */

            if ( on == false || isVisible() )
                QwtPlotCanvas::setSystemBackground(!on);

            break;
        }
    }
}

/*!
  Test wether a paint attribute is enabled

  \param attribute Paint attribute
  \return true if the attribute is enabled
  \sa setPaintAttribute()
*/
bool QwtPlotCanvas::testPaintAttribute(PaintAttribute attribute) const
{
    return (d_data->paintAttributes & attribute) != 0;
}

//! Return the paint cache, might be null
QPixmap *QwtPlotCanvas::paintCache()
{
    return d_data->cache;
}

//! Return the paint cache, might be null
const QPixmap *QwtPlotCanvas::paintCache() const
{
    return d_data->cache;
}

//! Invalidate the internal paint cache
void QwtPlotCanvas::invalidatePaintCache()
{
    if ( d_data->cache )
        *d_data->cache = QPixmap();
}

/*!
  Set the focus indicator

  \sa FocusIndicator, focusIndicator()
*/
void QwtPlotCanvas::setFocusIndicator(FocusIndicator focusIndicator)
{
    d_data->focusIndicator = focusIndicator;
}

/*!
  \return Focus indicator
  
  \sa FocusIndicator, setFocusIndicator()
*/
QwtPlotCanvas::FocusIndicator QwtPlotCanvas::focusIndicator() const
{
    return d_data->focusIndicator;
}

/*!
  Hide event
  \param event Hide event
*/
void QwtPlotCanvas::hideEvent(QHideEvent *event)
{
    QFrame::hideEvent(event);

    if ( d_data->paintAttributes & PaintPacked )
    {
        // enable system background to avoid the "looking through
        // the canvas" effect, for the next show

        setSystemBackground(true);
    }
}

/*!
  Paint event
  \param event Paint event
*/
void QwtPlotCanvas::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    
    if ( !contentsRect().contains( event->rect() ) ) 
    {
        painter.save();
        painter.setClipRegion( event->region() & frameRect() );
        drawFrame( &painter );
        painter.restore(); 
    }

    painter.setClipRegion(event->region() & contentsRect());

    drawContents( &painter );

    if ( d_data->paintAttributes & PaintPacked )
        setSystemBackground(false);
}

/*! 
  Redraw the canvas, and focus rect
  \param painter Painter
*/
void QwtPlotCanvas::drawContents(QPainter *painter)
{
    if ( d_data->paintAttributes & PaintCached && d_data->cache 
        && d_data->cache->size() == contentsRect().size() )
    {
        painter->drawPixmap(contentsRect().topLeft(), *d_data->cache);
    }
    else
    {
        QwtPlot *plot = ((QwtPlot *)parent());
        const bool doAutoReplot = plot->autoReplot();
        plot->setAutoReplot(false);

        drawCanvas(painter);

        plot->setAutoReplot(doAutoReplot);
    }

    if ( hasFocus() && focusIndicator() == CanvasFocusIndicator )
        drawFocusIndicator(painter);
}

/*!
  Draw the the canvas

  Paints all plot items to the contentsRect(), using QwtPlot::drawCanvas
  and updates the paint cache.

  \param painter Painter

  \sa QwtPlot::drawCanvas(), setPaintAttributes(), testPaintAttributes()
*/
void QwtPlotCanvas::drawCanvas(QPainter *painter)
{
    if ( !contentsRect().isValid() )
        return;

    QBrush bgBrush;
    bgBrush = palette().brush(backgroundRole());

    if ( d_data->paintAttributes & PaintCached && d_data->cache )
    {
        *d_data->cache = QPixmap(contentsRect().size());

#ifdef Q_WS_X11
        if ( d_data->cache->x11Info().screen() != x11Info().screen() )
            d_data->cache->x11SetScreen(x11Info().screen());
#endif

        if ( d_data->paintAttributes & PaintPacked )
        {
            QPainter bgPainter(d_data->cache);
            bgPainter.setPen(Qt::NoPen);

            bgPainter.setBrush(bgBrush);
            bgPainter.drawRect(d_data->cache->rect());
        }
        else
#if QT_VERSION < 0x050f00
            d_data->cache->fill(this, d_data->cache->rect().topLeft());
#else
            {
            QPainter painter(d_data->cache);
            painter.fillRect(d_data->cache->rect(), QColor(Qt::white));
        }
#endif

        QPainter cachePainter(d_data->cache);
        cachePainter.translate(-contentsRect().x(),
            -contentsRect().y());

        ((QwtPlot *)parent())->drawCanvas(&cachePainter);

        cachePainter.end();

        painter->drawPixmap(contentsRect(), *d_data->cache);
    }
    else
    {
        if ( d_data->paintAttributes & PaintPacked )
        {
            painter->save();

            painter->setPen(Qt::NoPen);
            painter->setBrush(bgBrush);
            painter->drawRect(contentsRect());

            painter->restore();
        }

        ((QwtPlot *)parent())->drawCanvas(painter);
    }
}

/*! 
  Draw the focus indication
  \param painter Painter
*/
void QwtPlotCanvas::drawFocusIndicator(QPainter *painter)
{
    const int margin = 1;

    QRect focusRect = contentsRect();
    focusRect.setRect(focusRect.x() + margin, focusRect.y() + margin,
        focusRect.width() - 2 * margin, focusRect.height() - 2 * margin);

    QwtPainter::drawFocusRect(painter, this, focusRect);
}

void QwtPlotCanvas::setSystemBackground(bool on)
{
    if ( testAttribute(Qt::WA_NoSystemBackground) == on ) setAttribute(Qt::WA_NoSystemBackground, !on);
}

/*!
   Invalidate the paint cache and repaint the canvas
   \sa invalidatePaintCache()
*/
void QwtPlotCanvas::replot()
{
    invalidatePaintCache();

    /*
      In case of cached or packed painting the canvas
      is repainted completely and doesn't need to be erased.
     */
    const bool erase =
        !testPaintAttribute(QwtPlotCanvas::PaintPacked)
        && !testPaintAttribute(QwtPlotCanvas::PaintCached);

    const bool noBackgroundMode = testAttribute(Qt::WA_OpaquePaintEvent);
    if ( !erase && !noBackgroundMode )
        setAttribute(Qt::WA_OpaquePaintEvent, true);

    repaint(contentsRect());

    if ( !erase && !noBackgroundMode )
        setAttribute(Qt::WA_OpaquePaintEvent, false);
}
