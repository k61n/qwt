/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_SVGITEM_H
#define QWT_PLOT_SVGITEM_H

#include <qglobal.h>

#include <qstring.h>
#include "qwt_double_rect.h" 
#include "qwt_plot_item.h" 

class QSvgRenderer;
class QByteArray;

/*!
  \brief A plot item, which displays 
         data in Scalable Vector Graphics (SVG) format.

  SVG images are often used to display maps
*/

class QWT_EXPORT QwtPlotSvgItem: public QwtPlotItem
{
public:
    explicit QwtPlotSvgItem(const QString& title = QString() );
    explicit QwtPlotSvgItem(const QwtText& title );
    virtual ~QwtPlotSvgItem();

    bool loadFile(const QwtDoubleRect&, const QString &fileName);
    bool loadData(const QwtDoubleRect&, const QByteArray &);

    virtual QwtDoubleRect boundingRect() const;

    virtual void draw(QPainter *p,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRect &rect) const;

    virtual int rtti() const;

protected:
    const QSvgRenderer &renderer() const;
    QSvgRenderer &renderer();

    void render(QPainter *painter,
        const QwtDoubleRect &viewBox, const QRect &rect) const;
    QwtDoubleRect viewBox(const QwtDoubleRect &area) const;

private:
    void init();

    class PrivateData;
    PrivateData *d_data;
};

#endif
