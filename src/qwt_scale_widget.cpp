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
#include <qevent.h>
#include "qwt_painter.h"
#include "qwt_color_map.h"
#include "qwt_scale_widget.h"
#include "qwt_scale_map.h"
#include "qwt_math.h"
#include "qwt_scale_div.h"
#include "qwt_text.h"

class QwtScaleWidget::PrivateData
{
public:
    PrivateData():
        scaleDraw(nullptr)
    {
        colorBar.colorMap = nullptr;
    }

    ~PrivateData()
    {
        delete scaleDraw;
        delete colorBar.colorMap;
    }

    QwtScaleDraw *scaleDraw;

    int borderDist[2];
    int minBorderDist[2];
    int scaleLength;
    int margin;
    int penWidth;

    int titleOffset;
    int spacing;
    QwtText title;

    int layoutFlags;

    struct t_colorBar
    {
        bool isEnabled;
        int width;
        QwtDoubleInterval interval;
        QwtColorMap *colorMap;
    } colorBar;
};

/*!
  \brief Create a scale with the position QwtScaleWidget::Left
  \param parent Parent widget
*/
QwtScaleWidget::QwtScaleWidget(QWidget *parent):
    QWidget(parent)
{
    initScale(QwtScaleDraw::LeftScale);
}

/*!
  \brief Constructor
  \param align Alignment.
  \param parent Parent widget
*/
QwtScaleWidget::QwtScaleWidget(
        QwtScaleDraw::Alignment align, QWidget *parent):
    QWidget(parent)
{
    initScale(align);
}

//! Destructor
QwtScaleWidget::~QwtScaleWidget()
{
    delete d_data;
}

//! Initialize the scale
void QwtScaleWidget::initScale(QwtScaleDraw::Alignment align)
{
    d_data = new PrivateData;

    d_data->layoutFlags = 0;
    if ( align == QwtScaleDraw::RightScale )
        d_data->layoutFlags |= TitleInverted;

    d_data->borderDist[0] = 0;
    d_data->borderDist[1] = 0;
    d_data->minBorderDist[0] = 0;
    d_data->minBorderDist[1] = 0;
    d_data->margin = 4;
    d_data->penWidth = 0;
    d_data->titleOffset = 0;
    d_data->spacing = 2;

    d_data->scaleDraw = new QwtScaleDraw;
    d_data->scaleDraw->setAlignment(align);
    d_data->scaleDraw->setLength(10);

    d_data->colorBar.colorMap = new QwtLinearColorMap();
    d_data->colorBar.isEnabled = false;
    d_data->colorBar.width = 10;

    const int flags = Qt::AlignHCenter | Qt::TextExpandTabs | Qt::TextWordWrap;
    d_data->title.setRenderFlags(flags);
    d_data->title.setFont(font());

    QSizePolicy policy(QSizePolicy::MinimumExpanding,
        QSizePolicy::Fixed);
    if ( d_data->scaleDraw->orientation() == Qt::Vertical )
        policy.transpose();

    setSizePolicy(policy);
    setAttribute(Qt::WA_WState_OwnSizePolicy, false);

}

/*!
   Toggle an layout flag

   \param flag Layout flag
   \param on true/false

   \sa testLayoutFlag(), LayoutFlag
*/
void QwtScaleWidget::setLayoutFlag(LayoutFlag flag, bool on)
{
    if ( ((d_data->layoutFlags & flag) != 0) != on )
    {
        if ( on )
            d_data->layoutFlags |= flag;
        else
            d_data->layoutFlags &= ~flag;
    }
}

/*!
   Test a layout flag

   \param hint Layout flag
   \return true/false
   \sa setLayoutFlag(), LayoutFlag
*/
bool QwtScaleWidget::testLayoutFlag(LayoutFlag flag) const
{
    return (d_data->layoutFlags & flag);
}

/*!
  Give title new text contents

  \param title New title
  \sa title(), setTitle(const QwtText &);
*/
void QwtScaleWidget::setTitle(const QString &title)
{
    if ( d_data->title.text() != title )
    {
        d_data->title.setText(title);
        layoutScale();
    }
}

/*!
  Give title new text contents

  \param title New title
  \sa title()
  \warning The title flags are interpreted in
               direction of the label, AlignTop, AlignBottom can't be set
               as the title will always be aligned to the scale.
*/
void QwtScaleWidget::setTitle(const QwtText &title)
{
    QwtText t = title;
    const int flags = title.renderFlags() & ~(Qt::AlignTop | Qt::AlignBottom);
    t.setRenderFlags(flags);

    if (t != d_data->title)
    {
        d_data->title = t;
        layoutScale();
    }
}

/*!
  Change the alignment

  \param alignment New alignment
  \sa alignment()
*/
void QwtScaleWidget::setAlignment(QwtScaleDraw::Alignment alignment)
{
    if ( !testAttribute(Qt::WA_WState_OwnSizePolicy) )
    {
        QSizePolicy policy(QSizePolicy::MinimumExpanding,
            QSizePolicy::Fixed);
        if ( d_data->scaleDraw->orientation() == Qt::Vertical )
            policy.transpose();
        setSizePolicy(policy);

        setAttribute(Qt::WA_WState_OwnSizePolicy, false);
    }

    if (d_data->scaleDraw)
        d_data->scaleDraw->setAlignment(alignment);
    layoutScale();
}


/*!
    \return position
    \sa setPosition()
*/
QwtScaleDraw::Alignment QwtScaleWidget::alignment() const
{
    if (!scaleDraw())
        return QwtScaleDraw::LeftScale;

    return scaleDraw()->alignment();
}

/*!
  Specify distances of the scale's endpoints from the
  widget's borders. The actual borders will never be less
  than minimum border distance.
  \param dist1 Left or top Distance
  \param dist2 Right or bottom distance
  \sa borderDist()
*/
void QwtScaleWidget::setBorderDist(int dist1, int dist2)
{
    if ( dist1 != d_data->borderDist[0] || dist2 != d_data->borderDist[1] )
    {
        d_data->borderDist[0] = dist1;
        d_data->borderDist[1] = dist2;
        layoutScale();
    }
}

/*!
  \brief Specify the margin to the colorBar/base line.
  \param margin Margin
  \sa margin()
*/
void QwtScaleWidget::setMargin(int margin)
{
    margin = qwtMax( 0, margin );
    if ( margin != d_data->margin )
    {
        d_data->margin = margin;
        layoutScale();
    }
}

/*!
  \brief Specify the distance between color bar, scale and title
  \param spacing Spacing
  \sa spacing()
*/
void QwtScaleWidget::setSpacing(int spacing)
{
    spacing = qwtMax( 0, spacing );
    if ( spacing != d_data->spacing )
    {
        d_data->spacing = spacing;
        layoutScale();
    }
}

/*!
  \brief Specify the width of the scale pen
  \param width Pen width
  \sa penWidth()
*/
void QwtScaleWidget::setPenWidth(int width)
{
    if ( width < 0 )
        width = 0;

    if ( width != d_data->penWidth )
    {
        d_data->penWidth = width;
        layoutScale();
    }
}

/*!
  \brief Change the alignment for the labels.

  \sa QwtScaleDraw::setLabelAlignment(), setLabelRotation()
*/
void QwtScaleWidget::setLabelAlignment(Qt::Alignment alignment)
{
    d_data->scaleDraw->setLabelAlignment(alignment);
    layoutScale();
}

/*!
  \brief Change the rotation for the labels.
  See QwtScaleDraw::setLabelRotation().

  \para, rotation Rotation
  \sa QwtScaleDraw::setLabelRotation(), setLabelFlags()
*/
void QwtScaleWidget::setLabelRotation(double rotation)
{
    d_data->scaleDraw->setLabelRotation(rotation);
    layoutScale();
}

/*!
  Set a scale draw
  sd has to be created with new and will be deleted in
  ~QwtScaleWidget() or the next call of setScaleDraw().

  \param sd ScaleDraw object
  \sa scaleDraw()
*/
void QwtScaleWidget::setScaleDraw(QwtScaleDraw *sd)
{
    if ( sd == nullptr || sd == d_data->scaleDraw )
        return;

    if ( d_data->scaleDraw )
        sd->setAlignment(d_data->scaleDraw->alignment());

    delete d_data->scaleDraw;
    d_data->scaleDraw = sd;

    layoutScale();
}

/*!
    scaleDraw of this scale
    \sa setScaleDraw(), QwtScaleDraw::setScaleDraw()
*/
const QwtScaleDraw *QwtScaleWidget::scaleDraw() const
{
    return d_data->scaleDraw;
}

/*!
    scaleDraw of this scale
    \sa QwtScaleDraw::setScaleDraw()
*/
QwtScaleDraw *QwtScaleWidget::scaleDraw()
{
    return d_data->scaleDraw;
}

/*!
    \return title
    \sa setTitle()
*/
QwtText QwtScaleWidget::title() const
{
    return d_data->title;
}

/*!
    \return start border distance
    \sa setBorderDist()
*/
int QwtScaleWidget::startBorderDist() const
{
    return d_data->borderDist[0];
}

/*!
    \return end border distance
    \sa setBorderDist()
*/
int QwtScaleWidget::endBorderDist() const
{
    return d_data->borderDist[1];
}

/*!
    \return margin
    \sa setMargin()
*/
int QwtScaleWidget::margin() const
{
    return d_data->margin;
}

/*!
    \return distance between scale and title
    \sa setMargin()
*/
int QwtScaleWidget::spacing() const
{
    return d_data->spacing;
}

/*!
    \return Scale pen width
    \sa setPenWidth()
*/
int QwtScaleWidget::penWidth() const
{
    return d_data->penWidth;
}
/*!
  \brief paintEvent
*/
void QwtScaleWidget::paintEvent(QPaintEvent *e)
{
    const QRect &ur = e->rect();
    if ( ur.isValid() )
    {
        QPainter painter(this);
        draw(&painter);
    }
}

/*!
  \brief draw the scale
*/
void QwtScaleWidget::draw(QPainter *painter) const
{
    painter->save();

    QPen scalePen = painter->pen();
    scalePen.setWidth(d_data->penWidth);
    painter->setPen(scalePen);

    d_data->scaleDraw->draw(painter, palette());
    painter->restore();

    if ( d_data->colorBar.isEnabled && d_data->colorBar.width > 0 &&
        d_data->colorBar.interval.isValid() )
    {
        drawColorBar(painter, colorBarRect(rect()));
    }

    QRect r = rect();
    if ( d_data->scaleDraw->orientation() == Qt::Horizontal )
    {
        r.setLeft(r.left() + d_data->borderDist[0]);
        r.setWidth(r.width() - d_data->borderDist[1]);
    }
    else
    {
        r.setTop(r.top() + d_data->borderDist[0]);
        r.setHeight(r.height() - d_data->borderDist[1]);
    }

    if ( !d_data->title.isEmpty() )
        drawTitle(painter, d_data->scaleDraw->alignment(), r);
}

QRect QwtScaleWidget::colorBarRect(const QRect& rect) const
{
    QRect cr = rect;

    if ( d_data->scaleDraw->orientation() == Qt::Horizontal )
    {
        cr.setLeft(cr.left() + d_data->borderDist[0]);
        cr.setWidth(cr.width() - d_data->borderDist[1] + 1);
    }
    else
    {
        cr.setTop(cr.top() + d_data->borderDist[0]);
        cr.setHeight(cr.height() - d_data->borderDist[1] + 1);
    }

    switch(d_data->scaleDraw->alignment())
    {
        case QwtScaleDraw::LeftScale:
        {
            cr.setLeft( cr.right() - d_data->spacing
                - d_data->colorBar.width + 1 );
            cr.setWidth(d_data->colorBar.width);
            break;
        }

        case QwtScaleDraw::RightScale:
        {
            cr.setLeft( cr.left() + d_data->spacing );
            cr.setWidth(d_data->colorBar.width);
            break;
        }

        case QwtScaleDraw::BottomScale:
        {
            cr.setTop( cr.top() + d_data->spacing );
            cr.setHeight(d_data->colorBar.width);
            break;
        }

        case QwtScaleDraw::TopScale:
        {
            cr.setTop( cr.bottom() - d_data->spacing
                - d_data->colorBar.width + 1 );
            cr.setHeight(d_data->colorBar.width);
            break;
        }
    }

    return cr;
}

/*!
  \brief resizeEvent
*/
void QwtScaleWidget::resizeEvent(QResizeEvent *)
{
    layoutScale(false);
}

//! Recalculate the scale's geometry and layout based on
//  the current rect and fonts.
//  \param update_geometry   notify the layout system and call update
//         to redraw the scale

void QwtScaleWidget::layoutScale( bool update_geometry )
{
    int bd0, bd1;
    getBorderDistHint(bd0, bd1);
    if ( d_data->borderDist[0] > bd0 ) bd0 = d_data->borderDist[0];
    if ( d_data->borderDist[1] > bd1 ) bd1 = d_data->borderDist[1];

    int colorBarWidth = 0;
    if ( d_data->colorBar.isEnabled && d_data->colorBar.interval.isValid() ) colorBarWidth = d_data->colorBar.width + d_data->spacing;

    const QRect r = rect();
    int x, y, length;

    if ( d_data->scaleDraw->orientation() == Qt::Vertical )
    {
        y = r.top() + bd0;
        length = r.height() - (bd0 + bd1);

        if ( d_data->scaleDraw->alignment() == QwtScaleDraw::LeftScale )
            x = r.right() - d_data->margin - colorBarWidth;
        else
            x = r.left() + d_data->margin + colorBarWidth;
    }
    else
    {
        x = r.left() + bd0;
        length = r.width() - (bd0 + bd1);

        if ( d_data->scaleDraw->alignment() == QwtScaleDraw::BottomScale )
            y = r.top() + d_data->margin + colorBarWidth;
        else
            y = r.bottom() - d_data->margin - colorBarWidth;
    }

    d_data->scaleDraw->move(x, y);
    d_data->scaleDraw->setLength(length);

    d_data->titleOffset = d_data->margin + d_data->spacing + colorBarWidth + d_data->scaleDraw->extent(QPen(Qt::black, d_data->penWidth), font());

    if ( update_geometry )
    {
      updateGeometry();
      update();
    }
}

void QwtScaleWidget::drawColorBar(QPainter *painter, const QRect& rect) const
{
    if ( !d_data->colorBar.interval.isValid() )
        return;

    const QwtScaleDraw* sd = d_data->scaleDraw;

    QwtPainter::drawColorBar(painter, *d_data->colorBar.colorMap,
        d_data->colorBar.interval.normalized(), sd->map(),
        sd->orientation(), rect);
}

/*!
  Rotate and paint a title according to its position into a given rectangle.
  \param painter Painter
  \param align Alignment
  \param rect Bounding rectangle
*/

void QwtScaleWidget::drawTitle(QPainter *painter,
    QwtScaleDraw::Alignment align, const QRect &rect) const
{
    QRect r = rect;
    double angle;
    int flags = d_data->title.renderFlags() &
        ~(Qt::AlignTop | Qt::AlignBottom | Qt::AlignVCenter);

    switch(align)
    {
        case QwtScaleDraw::LeftScale:
            angle = -90.0;
            flags |= Qt::AlignTop;
            r.setRect(r.left(), r.bottom(),
                r.height(), r.width() - d_data->titleOffset);
            break;

        case QwtScaleDraw::RightScale:
            angle = -90.0;
            flags |= Qt::AlignTop;
            r.setRect(r.left() + d_data->titleOffset, r.bottom(),
                r.height(), r.width() - d_data->titleOffset);
            break;

        case QwtScaleDraw::BottomScale:
            angle = 0.0;
            flags |= Qt::AlignBottom;
            r.setTop( r.top() + d_data->titleOffset );
            break;

        case QwtScaleDraw::TopScale:
        default:
            angle = 0.0;
            flags |= Qt::AlignTop;
            r.setBottom( r.bottom() - d_data->titleOffset );
            break;
    }

    if ( d_data->layoutFlags & TitleInverted )
    {
        if ( align == QwtScaleDraw::LeftScale
            || align == QwtScaleDraw::RightScale )
        {
            angle = -angle;
            r.setRect(r.x() + r.height(), r.y() - r.width(),
                r.width(), r.height() );
        }
    }

    painter->save();
    painter->setFont(font());
    painter->setPen(palette().color(QPalette::Text));

    const QwtMetricsMap metricsMap = QwtPainter::metricsMap();
    QwtPainter::resetMetricsMap();

    r = metricsMap.layoutToDevice(r);

    painter->translate(r.x(), r.y());
    if (angle != 0.0)
        painter->rotate(angle);

    QwtText title = d_data->title;
    title.setRenderFlags(flags);
    title.draw(painter, QRect(0, 0, r.width(), r.height()));


    QwtPainter::setMetricsMap(metricsMap); // restore metrics map

    painter->restore();
}

/*!
  \brief Notify a change of the scale

  This virtual function can be overloaded by derived
  classes. The default implementation updates the geometry
  and repaints the widget.
*/

void QwtScaleWidget::scaleChange()
{
    layoutScale();
}

/*!
  \return a size hint
*/
QSize QwtScaleWidget::sizeHint() const
{
    return minimumSizeHint();
}

/*!
  \return a minimum size hint
*/
QSize QwtScaleWidget::minimumSizeHint() const
{
    const Qt::Orientation o = d_data->scaleDraw->orientation();

    // Border Distance cannot be less than the scale borderDistHint
    // Note, the borderDistHint is already included in minHeight/minWidth
    int length = 0;
    int mbd1, mbd2;
    getBorderDistHint(mbd1, mbd2);
    length += qwtMax( 0, d_data->borderDist[0] - mbd1 );
    length += qwtMax( 0, d_data->borderDist[1] - mbd2 );
    length += d_data->scaleDraw->minLength(
        QPen(Qt::black, d_data->penWidth), font());

    int dim = dimForLength(length, font());
    if ( length < dim )
    {
        // compensate for long titles
        length = dim;
        dim = dimForLength(length, font());
    }

    QSize size(length + 2, dim);
    if ( o == Qt::Vertical )
        size.transpose();

    return size;
}

/*!
  \brief Find the height of the title for a given width.
  \param width Width
  \return height Height
 */

int QwtScaleWidget::titleHeightForWidth(int width) const
{
    return d_data->title.heightForWidth(width, font());
}

/*!
  \brief Find the minimum dimension for a given length.
         dim is the height, length the width seen in
         direction of the title.
  \param length width for horizontal, height for vertical scales
  \param scaleFont Font of the scale
  \return height for horizontal, width for vertical scales
*/

int QwtScaleWidget::dimForLength(int length, const QFont &scaleFont) const
{
    int dim = d_data->margin;
    dim += d_data->scaleDraw->extent(
        QPen(Qt::black, d_data->penWidth), scaleFont);

    if ( !d_data->title.isEmpty() )
        dim += titleHeightForWidth(length) + d_data->spacing;

    if ( d_data->colorBar.isEnabled && d_data->colorBar.interval.isValid() )
        dim += d_data->colorBar.width + d_data->spacing;

    return dim;
}

/*!
  \brief Calculate a hint for the border distances.

  This member function calculates the distance
  of the scale's endpoints from the widget borders which
  is required for the mark labels to fit into the widget.
  The maximum of this distance an the minimum border distance
  is returned.

  \warning
  <ul> <li>The minimum border distance depends on the font.</ul>
  \sa setMinBorderDist(), getMinBorderDist(), setBorderDist()
*/
void QwtScaleWidget::getBorderDistHint(int &start, int &end) const
{
    d_data->scaleDraw->getBorderDistHint(font(), start, end);

    if ( start < d_data->minBorderDist[0] )
        start = d_data->minBorderDist[0];

    if ( end < d_data->minBorderDist[1] )
        end = d_data->minBorderDist[1];
}

/*!
  Set a minimum value for the distances of the scale's endpoints from
  the widget borders. This is useful to avoid that the scales
  are "jumping", when the tick labels or their positions change
  often.

  \param start Minimum for the start border
  \param end Minimum for the end border
  \sa getMinBorderDist(), getBorderDistHint()
*/
void QwtScaleWidget::setMinBorderDist(int start, int end)
{
    d_data->minBorderDist[0] = start;
    d_data->minBorderDist[1] = end;
}

/*!
  Get the minimum value for the distances of the scale's endpoints from
  the widget borders.

  \sa setMinBorderDist(), getBorderDistHint()
*/
void QwtScaleWidget::getMinBorderDist(int &start, int &end) const
{
    start = d_data->minBorderDist[0];
    end = d_data->minBorderDist[1];
}

/*!
  \brief Assign a scale division

  The scale division determines where to set the tick marks.

  \param transformation Transformation, needed to translate between
                        scale and pixal values
  \param scaleDiv Scale Division
  \sa For more information about scale divisions, see QwtScaleDiv.
*/
void QwtScaleWidget::setScaleDiv(
    QwtScaleTransformation *transformation,
    const QwtScaleDiv &scaleDiv)
{
    QwtScaleDraw *sd = d_data->scaleDraw;
    if (sd->scaleDiv() != scaleDiv ||
        sd->map().transformation()->type() != transformation->type() )
    {
        sd->setTransformation(transformation);
        sd->setScaleDiv(scaleDiv);
        layoutScale();

        emit scaleDivChanged();
    }
    else
        delete transformation;
}

void QwtScaleWidget::setColorBarEnabled(bool on)
{
    if ( on != d_data->colorBar.isEnabled )
    {
        d_data->colorBar.isEnabled = on;
        layoutScale();
    }
}

bool QwtScaleWidget::isColorBarEnabled() const
{
    return d_data->colorBar.isEnabled;
}


void QwtScaleWidget::setColorBarWidth(int width)
{
    if ( width != d_data->colorBar.width )
    {
        d_data->colorBar.width = width;
        if ( isColorBarEnabled() )
            layoutScale();
    }
}

int QwtScaleWidget::colorBarWidth() const
{
    return d_data->colorBar.width;
}

QwtDoubleInterval QwtScaleWidget::colorBarInterval() const
{
    return d_data->colorBar.interval;
}

void QwtScaleWidget::setColorMap(const QwtDoubleInterval &interval,
    const QwtColorMap &colorMap)
{
    d_data->colorBar.interval = interval;

    delete d_data->colorBar.colorMap;
    d_data->colorBar.colorMap = colorMap.copy();

    if ( isColorBarEnabled() )
        layoutScale();
}

const QwtColorMap &QwtScaleWidget::colorMap() const
{
    return *d_data->colorBar.colorMap;
}
