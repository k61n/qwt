/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SPLINE_H
#define QWT_SPLINE_H

#include "qwt_global.h"
#include "qwt_double_rect.h"

#include <QPolygonF>

/*!
  \brief A class for spline interpolation

  The QwtSpline class is used for cubical spline interpolation.
  Two types of splines, natural and periodic, are supported.
  
  \par Usage:
  <ol>
  <li>First call setPoints() to determine the spline coefficients 
      for a tabulated function y(x).
  <li>After the coefficients have been set up, the interpolated
      function value for an argument x can be determined by calling 
      QwtSpline::value().
  </ol>

  \par Example:
  \code
#include <qwt_spline.h>

QPolygonF interpolate(const QPolygonF& points, int numValues)
{
    QwtSpline spline;
    if ( !spline.setPoints(points) ) 
        return points;

    QPolygonF interpolatedPoints(numValues);

    const double delta = 
        (points[numPoints - 1].x() - points[0].x()) / (points.size() - 1);
    for(i = 0; i < points.size(); i++)  / interpolate
    {
        const double x = points[0].x() + i * delta;
        interpolatedPoints[i].setX(x);
        interpolatedPoints[i].setY(spline.value(x));
    }
    return interpolatedPoints;
}
  \endcode
*/

class QWT_EXPORT QwtSpline
{
public:
    //! Spline type
    enum SplineType
    {
        Natural,
        Periodic
    };

    QwtSpline();
    QwtSpline( const QwtSpline & );

    ~QwtSpline();

    QwtSpline &operator=( const QwtSpline & );

    void setSplineType(SplineType);
    SplineType splineType() const;

    bool setPoints(const QPolygonF& points);
    QPolygonF points() const;

    void reset();

    bool isValid() const;
    double value(double x) const;

    const QwtArray<double> &coefficientsA() const;
    const QwtArray<double> &coefficientsB() const;
    const QwtArray<double> &coefficientsC() const;

protected:
    bool buildNaturalSpline(const QPolygonF &);
    bool buildPeriodicSpline(const QPolygonF &);

    class PrivateData;
    PrivateData *d_data;
};

#endif
