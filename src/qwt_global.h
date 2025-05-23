/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

// vim: expandtab

#ifndef QWT_GLOBAL_H
#define QWT_GLOBAL_H

#include <qglobal.h>

// QWT_VERSION is (major << 16) + (minor << 8) + patch.

#define QWT_VERSION       0x050200
#define QWT_VERSION_STR   "5.2.0"

#if defined(_MSC_VER) && defined(QWT_MAKEDLL)
#define QWT_EXPORT  __declspec(dllexport)
#elif defined(_MSC_VER)
#define QWT_EXPORT  __declspec(dllimport)
#else
#define QWT_EXPORT
#endif

#endif // QWT_GLOBAL_H
