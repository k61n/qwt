/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_CURVE_FITTER_H
#define QWT_CURVE_FITTER_H

#include "qwt_global.h"
#include "qwt_double_rect.h"

class QwtSpline;

#include <QPolygonF>

/*!
  \brief Abstract base class for a curve fitter
*/
class QWT_EXPORT QwtCurveFitter
{
public:
    virtual ~QwtCurveFitter();

    /*!
        Find a curve which has the best fit to a series of data points

        \param polygon Series of data points
        \return Curve points
     */
    virtual QPolygonF fitCurve(const QPolygonF &polygon) const = 0;

protected:
    QwtCurveFitter();

private:
    QwtCurveFitter( const QwtCurveFitter & );
    QwtCurveFitter &operator=( const QwtCurveFitter & );
};

/*!
  \brief A curve fitter using cubic splines
*/
class QWT_EXPORT QwtSplineCurveFitter: public QwtCurveFitter
{
public:
    enum FitMode
    {
        Auto,
        Spline,
        ParametricSpline
    };

    QwtSplineCurveFitter();
    virtual ~QwtSplineCurveFitter();

    void setFitMode(FitMode);
    FitMode fitMode() const;

    void setSpline(const QwtSpline&);
    const QwtSpline &spline() const;
    QwtSpline &spline();

    void setSplineSize(int size);
    int splineSize() const;

    virtual QPolygonF fitCurve(const QPolygonF &) const;

private:
    QPolygonF fitSpline(const QPolygonF &) const;
    QPolygonF fitParametric(const QPolygonF &) const;

    class PrivateData;
    PrivateData *d_data;
};

/*!
  \brief A curve fitter implementing Douglas and Peucker algorithm

  The purpose of the Douglas and Peucker algorithm is that given a 'curve'
  composed of line segments to find a curve not too dissimilar but that
  has fewer points. The algorithm defines 'too dissimilar' based on the
  maximum distance (tolerance) between the original curve and the
  smoothed curve.

  The smoothed curve consists of a subset of the points that defined the
  original curve.

  In opposite to QwtSplineCurveFitter the Douglas and Peucker algorithm reduces
  the number of points. By adjusting the tolerance parameter according to the
  axis scales QwtSplineCurveFitter can be used to implement different
  level of details to speed up painting of curves of many points.
*/
class QWT_EXPORT QwtWeedingCurveFitter: public QwtCurveFitter
{
public:
    QwtWeedingCurveFitter(double tolerance = 1.0);
    virtual ~QwtWeedingCurveFitter();

    void setTolerance(double);
    double tolerance() const;

    virtual QPolygonF fitCurve(const QPolygonF &) const;

private:
    class Line;

    class PrivateData;
    PrivateData *d_data;
};
#endif
