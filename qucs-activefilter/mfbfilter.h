/***************************************************************************
                               mfbfilter.h
                              ----------------
    begin                : Wed Apr 10 2014
    copyright            : (C) 2014 by Vadim Kuznetsov
    email                : ra3xdh@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MFBFILTER_H
#define MFBFILTER_H

#include <QtCore>
#include <complex>
#include "filter.h"

class MFBfilter : public Filter
{
protected:
    void calcFirstOrder() override;

    void calcHighPass() override;
    void calcLowPass() override;
    void calcBandPass() override;
    void calcBandStop() override;
    void createHighPassSchematic(QString &s) override;
    void createLowPassSchematic(QString &s) override;
    void createBandPassSchematic(QString &s) override;
    void createBandStopSchematic(QString &s) override;

public:
    MFBfilter(Filter::FilterFunc ffunc_, Filter::FType type_, FilterParam par);

};

#endif // MFBFILTER_H
