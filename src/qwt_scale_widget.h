/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SCALE_WIDGET_H
#define QWT_SCALE_WIDGET_H

#include <qwidget.h>
#include <qfont.h>
#include <qcolor.h>
#include <qstring.h>

#include "qwt_global.h"
#include "qwt_text.h"
#include "qwt_scale_draw.h"

class QPainter;
class QwtScaleTransformation;
class QwtScaleDiv;
class QwtColorMap;

/*!
  \brief A Widget which contains a scale

  This Widget can be used to decorate composite widgets with
  a scale.
*/

class QWT_EXPORT QwtScaleWidget : public QWidget
{
    Q_OBJECT

public:
	/*!
     Layout flags of the title

     - TitleInverted\n
       The title of vertical scales is painted from top to bottom. Otherwise
       it is painted from bottom to top.
     */
    enum LayoutFlag
    {
        TitleInverted = 1
    };

    explicit QwtScaleWidget(QWidget *parent = nullptr);
    explicit QwtScaleWidget(QwtScaleDraw::Alignment, QWidget *parent = nullptr);
    virtual ~QwtScaleWidget();

signals:
    //! Signal emitted, whenever the scale divison changes
    void scaleDivChanged();

public:
    void setTitle(const QString &title);
    void setTitle(const QwtText &title);
    QwtText title() const;

	void setLayoutFlag(LayoutFlag, bool on);
	bool testLayoutFlag(LayoutFlag) const;

    void setBorderDist(int start, int end);
    int startBorderDist() const;
    int endBorderDist() const;

    void getBorderDistHint(int &start, int &end) const;

    void getMinBorderDist(int &start, int &end) const;
    void setMinBorderDist(int start, int end);

    void setMargin(int);
    int margin() const;

    void setSpacing(int td);
    int spacing() const;

    void setPenWidth(int);
    int penWidth() const;

    void setScaleDiv(QwtScaleTransformation *, const QwtScaleDiv &sd);

    void setScaleDraw(QwtScaleDraw *);
    const QwtScaleDraw *scaleDraw() const;
    QwtScaleDraw *scaleDraw();

    void setLabelAlignment(Qt::Alignment);
    void setLabelRotation(double rotation);

    void setColorBarEnabled(bool);
    bool isColorBarEnabled() const;

    void setColorBarWidth(int);
    int colorBarWidth() const;

    void setColorMap(const QwtDoubleInterval &, const QwtColorMap &);

    QwtDoubleInterval colorBarInterval() const;
    const QwtColorMap &colorMap() const;

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

    int titleHeightForWidth(int width) const;
    int dimForLength(int length, const QFont &scaleFont) const;

    void drawColorBar(QPainter *painter, const QRect &rect) const;
    void drawTitle(QPainter *painter, QwtScaleDraw::Alignment,
        const QRect &rect) const;

    void setAlignment(QwtScaleDraw::Alignment);
    QwtScaleDraw::Alignment alignment() const;

    QRect colorBarRect(const QRect&) const;

protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void resizeEvent(QResizeEvent *e);

    void draw(QPainter *p) const;

    void scaleChange();
    void layoutScale( bool update = true );

private:
    void initScale(QwtScaleDraw::Alignment);

    class PrivateData;
    PrivateData *d_data;
};

#endif
