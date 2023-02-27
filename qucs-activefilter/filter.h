/***************************************************************************
                                 filter.h
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

#ifndef FILTER_H
#define FILTER_H

#include <QtCore>
#include <cmath>
#include <complex>
#include <limits>

static constexpr double pi = 3.1415926535897932384626433832795029;  /* pi   */

struct RC_elements {
    int     N  = 0;
    double  R1 = ::std::numeric_limits<double>::infinity();
    double  R2 = ::std::numeric_limits<double>::infinity();
    double  R3 = ::std::numeric_limits<double>::infinity();
    double  R4 = ::std::numeric_limits<double>::infinity();
    double  R5 = ::std::numeric_limits<double>::infinity();
    double  R6 = ::std::numeric_limits<double>::infinity();
    double  C1 = 0.0;
    double  C2 = 0.0;
    double  Au = 0.0;
};

struct FilterParam {
    double  Ap; // Band pass atten.
    double  As; // Band stop atten.
    double  Fc; // Cutoff frequency
    double  Fs; // Stopband freq.
    double  Rp; // Passband ripple
    double  Kv; // Passband gain
    double  Fl; // Lower freq.
    double  Fu; // Upper freq.
    double  TW; // Band width
    double  Q; // Quality factor
    int order;
};

class Filter
{

public:
    enum FType {HighPass, LowPass, BandPass, BandStop, NoFilter};
    enum FilterFunc {Butterworth, Chebyshev, Cauer, Bessel, InvChebyshev, NoFunc, User};

private:
    void cauerOrderEstim();
    void reformPolesZeros();

protected:
    QVector< std::complex<float> > Poles;
    QVector< std::complex<float> > Zeros;
    QVector<long double> vec_B; // Transfer function numenator
    QVector<long double> vec_A; // and denominator
    QVector<RC_elements> Sections;

    Filter::FType ftype;
    Filter::FilterFunc ffunc;
    int order;
    double  Fc,Kv,Fs,Ap,As,Rp,Fl,Fu,TW,Q,BW,F0;
    int Nr,Nc,Nopamp; // total number of R,C, opamp

    int Nr1,Nc1,Nop1; // number of R,C, opamp per stage

    void calcButterworth();
    void calcChebyshev();
    void calcInvChebyshev();
    void calcCauer();
    void calcBessel();
    void calcUserTrFunc();
    void checkRCL(); // Checks RCL values. Are one of them NaN or not?

    void createFirstOrderComponentsHPF(QString &s,RC_elements stage, int dx);
    void createFirstOrderComponentsLPF(QString &s,RC_elements stage, int dx);
    void createFirstOrderWires(QString &s, int dx, int y);

    virtual void calcHighPass();
    virtual void calcLowPass();
    virtual void calcBandPass();
    virtual void calcBandStop();
    virtual void createHighPassSchematic(QString &s);
    virtual void createLowPassSchematic(QString &s);
    virtual void createBandPassSchematic(QString &s);
    virtual void createBandStopSchematic(QString &s);

    static double  autoscaleCapacitor(double C, QString &suffix);
    static double  autoscaleResistor(double R, QString &suffix);

public:


    Filter(Filter::FilterFunc ffunc_, Filter::FType type_, FilterParam par);
    virtual ~Filter();

    virtual void calcFirstOrder();

    void createPartList(QString & lst);
    void createPolesZerosList(QString &lst);

    virtual void createSchematic(QString &s);

    virtual void calcFilter();

    void set_TrFunc(QVector<long double> a, QVector<long double> b);

    inline int getOrder(void) const
    {
        return order;
    }
};

#endif // FILTER_H
