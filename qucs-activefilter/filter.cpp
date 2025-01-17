/***************************************************************************
                                filter.cpp
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "filter.h"
#include "qf_poly.h"
#include "bessel.h"
#include "filter-exceptions.h"
#include <string>
#include <utility>

using ::QUCS::Exception::FilterError;

#include <limits>

static const int MaxOrder = 50;

Filter::Filter(Filter::FilterFunc ffunc_, Filter::FType type_, FilterParam par)
{
    ffunc = ffunc_;
    ftype = type_;

    switch (ftype) {
        case Filter::HighPass: [[fallthrough]];
        case Filter::LowPass:
            Fc = par.Fc;
            Fs = par.Fs;
            Ap = par.Ap;
        break;
        default:
            Fl = par.Fl;
            Fu = par.Fu;
            TW = par.TW;
            BW = fabs(Fu -Fl);
            F0 = sqrt(Fu*Fl);
            Fc=BW;          // cutoff freq. of LPF prototype
            double  Fs1 = Fu + TW;
            double  Fs1lp = fabsf(Fs1 - (F0*F0)/Fs1);    // stopband freq. of LPF prototype
            double  Fs2 = Fl - TW;
            double  Fs2lp = fabsf(Fs2 - (F0*F0)/Fs2);
            Fs = std::min(Fs1lp,Fs2lp);
            Ap = 3.0;
            qDebug()<<Fc<<Fs;
            Q = F0/fabs(Fu-Fl);
        break;
    }

    Rp = par.Rp;
    As = par.As;
    Kv = par.Kv;

    order = par.order;  // for Bessel
}

Filter::~Filter()
{

}

void Filter::createSchematic(QString &s)
{
    switch (ftype) {
    case Filter::HighPass : createHighPassSchematic(s);
        break;
    case Filter::LowPass : createLowPassSchematic(s);
        break;
    case Filter::BandPass : createBandPassSchematic(s);
        break;
    case Filter::BandStop : createBandStopSchematic(s);
        break;
    default: break;
    }

}

void Filter::createHighPassSchematic(QString &s)
{
    s = "<Qucs Schematic ";
    s += PACKAGE_VERSION;
    s += ">\n";
}

void Filter::createLowPassSchematic(QString &s)
{
    s = "<Qucs Schematic ";
    s += PACKAGE_VERSION;
    s += ">\n";
}

void Filter::createBandPassSchematic(QString &s)
{
    s = "<Qucs Schematic " PACKAGE_VERSION ">\n";
}

void Filter::createBandStopSchematic(QString &s)
{
    s = "<Qucs Schematic " PACKAGE_VERSION ">\n";
}

void Filter::calcFilter()
{
    Sections.clear();
    Poles.clear();
    Zeros.clear();

    switch (ffunc) {
        case Filter::Chebyshev:
            calcChebyshev();
        break;
        case Filter::Butterworth:
            calcButterworth();
        break;
        case Filter::Cauer:
            calcCauer();
        break;
        case Filter::InvChebyshev:
            calcInvChebyshev();
        break;
        case Filter::Bessel:
            calcBessel();
        break;
        case Filter::User:
            calcUserTrFunc();
        break;
        default:
            throw FilterError("Internal error: unknown filter function");
        break;
    }

    if (Poles.isEmpty()) {
        throw FilterError("No Poles generated");
    }

    if (Zeros.isEmpty()) {
        if (ffunc==Filter::Cauer) {
            throw FilterError("No Zeroes generated for Cauer filter");
        }
        if (ffunc==Filter::InvChebyshev) {
            throw FilterError("No Zeroes generated for Inverse Chebyshev filter");
        }
    }

    switch (ftype) {
        case Filter::LowPass:
            calcLowPass();
        break;
        case Filter::HighPass:
            calcHighPass();
        break;
        case Filter::BandPass:
            calcBandPass();
        break;
        case Filter::BandStop:
            calcBandStop();
        break;
        default:
            throw FilterError("Internal error: unknown filter type");
        break;
    }

    checkRCL();

    Nr = Nr1*(order/2);
    Nc = Nc1*(order/2);
    Nopamp = Nop1*order/2;
}


void Filter::calcHighPass()
{
}

void Filter::calcLowPass()
{
}

void Filter::calcBandPass()
{
}

void Filter::calcBandStop()
{
}

void Filter::calcFirstOrder()
{
    if (order%2 == 0) {
        return;
    }

    double  R1,R3,R4;

    int k = order/2 + 1;
    double  Nst = order/2 + order%2;
    double  Kv1 = pow(Kv,1.0/Nst);
    double  Wc = 2*pi*Fc;
    double  re = Poles.at(k-1).real();
    double  C = -re;
    double  C1 = 10/Fc;

    if (ftype==Filter::HighPass) {
        R1 = 1.0*C/(Wc*C1);
    } else {
        R1 = 1.0/(Wc*C*C1);
    }

    if (Kv != 1.0) {
        R3 = Kv1 * R1/(Kv1 - 1.0);
        R4 = Kv1 * R1;
    } else {
        R3 = ::std::numeric_limits<double>::infinity();
        R4 = 0;
    }

    RC_elements curr_stage;
    curr_stage.N  = k;
    curr_stage.R1 = 1000*R1;
    curr_stage.R2 = 0.0;
    curr_stage.R3 = 1000*R3;
    curr_stage.R4 = 1000*R4;
    curr_stage.C1 = C1;
    curr_stage.Au = Kv1;
    Sections.append(curr_stage);
}

void Filter::createPartList(QString & lst)
{
    RC_elements stage;
    unsigned int maxR = 0U;
    bool hasAu = false;

    foreach (stage, Sections) {
        double Rv[] { stage.R1, stage.R2, stage.R3, stage.R4, stage.R5, stage.R6 };
        for (unsigned int i = 0; i < 6; ++i) {
            if (Rv[i] != ::std::numeric_limits<double>::infinity()) {
                maxR = i;
            }
        }
        if (stage.Au != 0.0) {
            hasAu = true;
        }
    }

    lst += QString(
        "<table cellpadding=\"6\" cellspacing=\"1\" bgcolor=\"#000000\">"
        "<caption align=\"top\" bgcolor=\"#ffffff\">%1</caption>"
        "<tr bgcolor=\"#ffffff\">"
            "<th style=\"text-align: center\"> %2 </th>"
            "<th style=\"text-align: center\"> C1 </th>"
            "<th style=\"text-align: center\"> C2 </th>"
    ).
        arg(QObject::tr("Part List:")).
        arg(QObject::tr("Stage"));

    for (unsigned int i = 0; i <= maxR; ++i) {
        lst += QString("<th style=\"text-align: center\"> R%1 </th>").arg(i+1);
    }

    if (hasAu) {
        lst += QString("<th style=\"text-align: center\"> Au </th>");
    }

    lst += QString("</tr>");

    foreach (stage, Sections) {
        double Rv[] { stage.R1, stage.R2, stage.R3, stage.R4, stage.R5, stage.R6 };
        auto C = [](double c) {
            if (c == 0.0) {
                return QString(QObject::tr("not used"));
            }
            QString suffix;
            double result = autoscaleCapacitor(c, suffix);
            return QString("%1%2").arg(result, 8, 'f', 3).arg(suffix);
        };
        auto R = [](double r) {
            if (r == ::std::numeric_limits<double>::infinity()) {
                return QString(QObject::tr("not used"));
            }
            if (r == 0.0) {
                return QString(QObject::tr("short"));
            }
            QString suffix;
            double result = autoscaleResistor(r, suffix);
            return QString("%1%2").arg(result, 8, 'f', 3).arg(suffix);
        };
        auto A = [](double au) {
            if (au == 0.0) {
                return QString("unknown");
            }
            return QString("%1").arg(au, 5, 'f', 3);
        };

        lst += QString(
            "<tr bgcolor=\"#ffffff\">"
                "<td style=\"text-align: center\"> %1 </td>"
                "<td style=\"text-align: center\"> %2 </td>"
                "<td style=\"text-align: center\"> %3 </td>"
        ).
            arg(stage.N).
            arg(C(stage.C1)).
            arg(C(stage.C2));


        for (unsigned int i = 0; i <= maxR; ++i) {
            lst += QString("<td style=\"text-align: center\"> %1 </td>").arg(R(Rv[i]));
        }

        if (hasAu) {
            lst += QString("<td style=\"text-align: center\"> %1 </td>").arg(A(stage.Au));
        }

        lst += QString("</tr>");
    }

    lst += "</table>\r\n";
}

void Filter::createPolesZerosList(QString &lst) {
    lst += QString(QObject::tr("Filter order = %1")).arg(order);
    if (!Zeros.isEmpty()) {
        lst += "\r\n\r\n";
        lst += QObject::tr("Zeros list Pk=Re+j*Im");
        for (std::complex<float> zero: Zeros) {
            lst += "\r\n";
            lst += QString::number(zero.real()) + " + j*" + QString::number(zero.imag());
        }
    }

    if ((ftype == Filter::BandPass) || (ftype == Filter::BandStop)) {
        lst += "\r\n";
        lst += QObject::tr("LPF prototype poles list Pk=Re+j*Im");
    } else {
        lst += "\r\n";
        lst += QObject::tr("Poles list Pk=Re+j*Im");
    }
    for (std::complex<float> pole: Poles) {
        lst += "\r\n";
        lst += QString::number(pole.real()) + " + j*" + QString::number(pole.imag());
    }
    lst += "\r\n";
}

void Filter::createFirstOrderComponentsHPF(QString &s,RC_elements stage,int dx)
{
    QString suf;
    double C = autoscaleCapacitor(stage.C2,suf);
    s += QString("<OpAmp OP%1 1 %2 160 -26 42 0 0 \"1e6\" 1 \"15 V\" 0>\n").arg(Nopamp+1).arg(270+dx);
    s += QString("<GND * 1 %1 270 0 0 0 0>\n").arg(170+dx);
    s += QString("<GND * 1 %1 370 0 0 0 0>\n").arg(220+dx);
    s += QString("<R R%1 1 %2 340 15 -26 0 1 \"%3k\" 1 \"26.85\" 0 \"0.0\" 0 \"0.0\" 0 \"26.85\" 0 \"european\" 0>\n").arg(Nr+2).arg(220+dx).arg(stage.R2,0,'f',3);
    s += QString("<R R%1 1 %2 240 -75 -26 1 1 \"%3k\" 1 \"26.85\" 0 \"0.0\" 0 \"0.0\" 0 \"26.85\" 0 \"european\" 0>\n").arg(Nr+1).arg(170+dx).arg(stage.R1,0,'f',3);
    s += QString("<R R%1 1 %2 260 -26 15 1 2 \"%3k\" 1 \"26.85\" 0 \"0.0\" 0 \"0.0\" 0 \"26.85\" 0 \"european\" 0>\n").arg(Nr+3).arg(310+dx).arg(stage.R3,0,'f',3);
    s += QString("<C C%1 1 %2 190 -26 -45 1 0 \"%3%4\" 1 \"\" 0 \"neutral\" 0>\n").arg(Nc+1).arg(100+dx).arg(C,0,'f',3).arg(suf);
}


void Filter::createFirstOrderComponentsLPF(QString &s,RC_elements stage,int dx)
{
    QString suf;
    double C = autoscaleCapacitor(stage.C2,suf);
    s += QString("<OpAmp OP%1 1 %2 160 -26 42 0 0 \"1e6\" 1 \"15 V\" 0>\n").arg(Nopamp+1).arg(270+dx);
    s += QString("<GND * 1 %1 270 0 0 0 0>\n").arg(170+dx);
    if (stage.R3 != 0) {
        s += QString("<R R%1 1 %2 340 15 -26 0 1 \"%3k\" 1 \"26.85\" 0 \"0.0\" 0 \"0.0\" 0 \"26.85\" 0 \"european\" 0>\n").arg(Nr+2).arg(220+dx).arg(stage.R2,0,'f',3);
        s += QString("<GND * 1 %1 370 0 0 0 0>\n").arg(220+dx);
    }
    s += QString("<R R%1 1 %2 190 -75 -52 1 0 \"%3k\" 1 \"26.85\" 0 \"0.0\" 0 \"0.0\" 0 \"26.85\" 0 \"european\" 0>\n").arg(Nr+1).arg(100+dx).arg(stage.R1,0,'f',3);
    if (stage.R3 == 0) {
    } else if (stage.R3 == ::std::numeric_limits<double>::infinity()) {
    } else {
        s += QString("<R R%1 1 %2 260 -26 15 1 2 \"%3k\" 1 \"26.85\" 0 \"0.0\" 0 \"0.0\" 0 \"26.85\" 0 \"european\" 0>\n").arg(Nr+3).arg(310+dx).arg(stage.R3,0,'f',3);
    }
    s += QString("<C C%1 1 %2 240 26 -45 1 1 \"%3%4\" 1 \"\" 0 \"neutral\" 0>\n").arg(Nc+1).arg(170+dx).arg(C,0,'f',3).arg(suf);
}

void Filter::createFirstOrderWires(QString &s, const RC_elements & stage, int dx, int y)
{
    s += QString("<%1 190 %2 190 \"\" 0 0 0 \"\">\n").arg(dx-20).arg(70+dx);
    s += QString("<%1 190 %2 %3 \"\" 0 0 0 \"\">\n").arg(dx-20).arg(dx-20).arg(y);
    s += QString("<%1 %2 %3 %4 \"\" 0 0 0 \"\">\n").arg(dx-20).arg(y).arg(dx-50).arg(y);

    s += QString("<%1 190 %2 190 \"\" 0 0 0 \"\">\n").arg(130+dx).arg(170+dx);
    s += QString("<%1 160 %2 160 \"out\" %3 130 39 \"\">\n").arg(310+dx).arg(360+dx).arg(380+dx);
    s += QString("<%1 260 %2 260 \"\" 0 0 0 \"\">\n").arg(340+dx).arg(360+dx);
    s += QString("<%1 160 %2 260 \"\" 0 0 0 \"\">\n").arg(360+dx).arg(360+dx);
    s += QString("<%1 190 %2 210 \"\" 0 0 0 \"\">\n").arg(170+dx).arg(170+dx);
    s += QString("<%1 140 %2 190 \"\" 0 0 0 \"\">\n").arg(170+dx).arg(170+dx);
    s += QString("<%1 140 %2 140 \"\" 0 0 0 \"\">\n").arg(170+dx).arg(240+dx);
    s += QString("<%1 180 %2 260 \"\" 0 0 0 \"\">\n").arg(220+dx).arg(220+dx);
    s += QString("<%1 180 %2 180 \"\" 0 0 0 \"\">\n").arg(220+dx).arg(240+dx);
    s += QString("<%1 260 %2 260 \"\" 0 0 0 \"\">\n").arg(220+dx).arg(280+dx);
    if (stage.R3 != 0) {
        s += QString("<%1 260 %2 310 \"\" 0 0 0 \"\">\n").arg(220+dx).arg(220+dx);
    } else {
        s += QString("<%1 260 %2 260 \"\" 0 0 0 \"\">\n").arg(280+dx).arg(340+dx);
    }
}


double Filter::autoscaleResistor(double R, QString & suffix)
{
    if (R == ::std::numeric_limits<double>::infinity()) {
        suffix = "";
        return R;
    }

    double R1 = R * 1e3;

    if (R1 >= 1e6) {
        suffix = "M";
        R1 *= 1e-6;
    } else if (R1 >= 1e3) {
        suffix = "k";
        R1 *= 1e-3;
    } else {
        suffix = "";
    }

    return R1;
}

double Filter::autoscaleCapacitor(double C, QString &suffix)
{
    double C1 = C * 1e-6;

    if (C1 >= 1e-6) {
        suffix = "u";
        C1 *= 1e6;
    } else if (C1 >= 1e-9) {
        suffix = "n";
        C1 *= 1e9;
    } else {
        suffix = "p";
        C1 *= 1e12;
    }

    return C1;
}

void Filter::checkRCL()
{
    for (auto &sec : Sections) {
        if (std::isnan(sec.R1)) {
            throw FilterError("R1 value is invalid");
        }
        if (std::isnan(sec.R2)) {
            throw FilterError("R2 value is invalid");
        }
        if (std::isnan(sec.R3)) {
            throw FilterError("R3 value is invalid");
        }
        if (std::isnan(sec.R4)) {
            throw FilterError("R4 value is invalid");
        }
        if (std::isnan(sec.R5)) {
            throw FilterError("R5 value is invalid");
        }
        if (std::isnan(sec.R6)) {
            throw FilterError("R6 value is invalid");
        }
        if (std::isnan(sec.C1)) {
            throw FilterError("C1 value is invalid");
        }
        if (std::isnan(sec.C2)) {
            throw FilterError("C2 value is invalid");
        }
    }
}

void Filter::calcChebyshev()
{
    double  kf = std::max(Fs/Fc,Fc/Fs);
    double  eps=sqrt(pow(10,0.1*Rp)-1);

    double  N1 = acosh(sqrt((pow(10,0.1*As)-1)/(eps*eps)))/acosh(kf);
    int N = ceil(N1);

    if (N < 1) {
        ::std::string msg = "Chebyshev Filter: too low order (" + ::std::to_string(N) + ")\n";
        msg += ::std::string(" o As=") + ::std::to_string(As)
                         + "\n o Rp="  + ::std::to_string(Rp)
                         + "\n o Fs="  + ::std::to_string(Fs)
                         + "\n o Fc="  + ::std::to_string(Fc)
                         + "\n o kf="  + ::std::to_string(kf) + ")";
        throw FilterError(msg.c_str());
    }

    if (N > MaxOrder) {
        throw FilterError(QObject::tr("Too high Filter Order: %1").arg(N));
    }

    if (ftype==Filter::BandStop) {
        if (N%2!=0) N++; // only even order Cauer.
    }

    double  a = sinh((asinh(1/eps))/N);
    double  b = cosh((asinh(1/eps))/N);

    Poles.clear();
    Zeros.clear();

    for (int k=1;k<=N;k++) {
            double  re = -1*a*sin(pi*(2*k-1)/(2*N));
            double  im = b*cos(pi*(2*k-1)/(2*N));
            std::complex<float> pol(re,im);
            Poles.append(pol);
    }

    order = Poles.count();
}

void Filter::calcButterworth()
{
    double  kf = std::min(Fc/Fs,Fs/Fc);

    double  C1=(pow(10,(0.1*Ap))-1)/(pow(10,(0.1*As))-1);
    double  J2=log10(C1)/(2*log10(kf));
    int N2 = round(J2+1);

    if (ftype==Filter::BandStop) {
        if (N2%2!=0) N2++; // only even order Cauer.
    }

    if (N2 > MaxOrder) {
        throw FilterError(QObject::tr("Too high Filter Order: %1").arg(N2));
    }

    Poles.clear();
    Zeros.clear();

    for (int k=1;k<=N2;k++) {
        double  re =-1*sin(pi*(2*k-1)/(2*N2));
        double  im =cos(pi*(2*k-1)/(2*N2));
        std::complex<float> pol(re,im);
        Poles.append(pol);
    }

    order = Poles.count();
}

void Filter::calcInvChebyshev() // Chebyshev Type-II filter
{
    double  kf = std::max(Fs/Fc,Fc/Fs);

    order = ceil(acosh(sqrt(pow(10.0,0.1*As)-1.0))/acosh(kf));

    if ((ftype==Filter::BandPass)||(ftype==Filter::BandStop)) {
        if (order%2!=0) order++; // only even order Cauer.
    }

    if (order>MaxOrder) {
        throw FilterError(QObject::tr("Too high Filter Order: %1").arg(order));
    }

    double  eps = 1.0/(sqrt(pow(10.0,0.1*As)-1.0));
    double  a = sinh((asinh(1.0/eps))/(order));
    double  b = cosh((asinh(1.0/eps))/(order));

    Poles.clear();
    Zeros.clear();

    for (int k=1;k<=order;k++) {
        double  im = 1.0/(cos(((2*k-1)*pi)/(2*order)));
        Zeros.append(std::complex<float>(0,im));
    }

    for (int k=1;k<=order;k++) {
        double  re = -1*a*sin(pi*(2*k-1)/(2*order));
        double  im = b*cos(pi*(2*k-1)/(2*order));
        std::complex<float> invpol(re,im); // inverse pole
        std::complex<float> pol;
        pol = std::complex<float>(1.0,0) / invpol; // pole
        Poles.append(pol);
    }
}

void Filter::cauerOrderEstim() // from Digital Filter Design Handbook page 102
{
    double  k = std::min(Fc/Fs,Fs/Fc);
    double  kk = sqrt(sqrt(1.0-k*k));
    double  u = 0.5*(1.0-kk)/(1.0+kk);
    double  q = 150.0*pow(u,13) + 2.0*pow(u,9) + 2.0*pow(u,5) + u;
    double  dd = (pow(10.0,As/10.0)-1.0)/(pow(10.0,Rp/10.0)-1.0);
    order = ceil(log10(16.0*dd)/log10(1.0/q));

    if ((ftype==Filter::BandPass)||(ftype==Filter::BandStop)) {
        if (order%2!=0) order++; // only even order Cauer.
    }
}

void Filter::calcCauer() // from Digital Filter Designer's handbook p.103
{
    double  P0;
    //float H0;
    double  mu;
    double  aa[50],bb[50],cc[50];

    cauerOrderEstim();

    if (order > MaxOrder) {
        throw FilterError(QObject::tr("Too high Filter Order: %1").arg(order));
    }

    double  k = Fc/Fs;
    double  kk = sqrt(sqrt(1.0-k*k));
    double  u = 0.5*(1.0-kk)/(1.0+kk);
    double  q = 150.0*pow(u,13) + 2.0*pow(u,9) + 2.0*pow(u,5) + u;
    double  numer = pow(10.0,Rp/20.0)+1.0;
    double  vv = log(numer/(pow(10.0,Rp/20.0)-1.0))/(2.0*order);
    double  sum = 0.0;
    for (int m=0;m<5;m++) {
        double  term = pow(-1.0,m);
        term = term*pow(q,m*(m+1));
        term = term*sinh((2*m+1)*vv);
        sum = sum +term;
    }
    numer = 2.0*sum*sqrt(sqrt(q));

    sum=0.0;
    for (int m=1;m<5;m++) {
        double  term = pow(-1.0,m);
        term = term*pow(q,m*m);
        term = term*cosh(2.0*m*vv);
        sum += term;
    }
    double  denom = 1.0+2.0*sum;
    P0 = fabs(numer/denom);
    double  ww = 1.0+k*P0*P0;
    ww = sqrt(ww*(1.0+P0*P0/k));
    int r = (order-(order%2))/2;
    //float numSecs = r;

    for (int i=1;i<=r;i++) {
        if ((order%2)!=0) {
            mu = i;
        } else {
            mu = i-0.5;
        }
        sum = 0.0;
        for(int m=0;m<5;m++) {
            double  term = pow(-1.0,m)*pow(q,m*(m+1));
            term = term*sin((2*m+1)*pi*mu/order);
            sum += term;
        }
        numer = 2.0*sum*sqrt(sqrt(q));

        sum = 0.0;
        for(int m=1;m<5;m++) {
            double  term = pow(-1.0,m)*pow(q,m*m);
            term = term*cos(2.0*m*pi*mu/order);
            sum += term;
        }
        denom = 1.0+2.0*sum;
        double  xx = numer/denom;
        double  yy = 1.0 - k*xx*xx;
        yy = sqrt(yy*(1.0-(xx*xx/k)));
        aa[i-1] = 1.0/(xx*xx);
        denom = 1.0 + pow(P0*xx,2);
        bb[i-1] = 2.0*P0*yy/denom;
        denom = pow(denom,2);
        numer = pow(P0*yy,2)+pow(xx*ww,2);
        cc[i-1] = numer/denom;
    }

    if (order%2!=0) {
        cc[order-1] = P0; // first order section
        bb[order-1] = 0;
    }

    Zeros.clear();
    Poles.clear();

    for (int i=0;i<r;i++) {
        double  im = sqrt(aa[i]);
        Zeros.append(std::complex<float>(0,im));
        double  re = -0.5*bb[i];
        im = 0.5*sqrt(-1.0*bb[i]*bb[i]+4*cc[i]);
        Poles.append(std::complex<float>(re,im));
    }

    if (order%2!=0) {
        Poles.append(std::complex<float>(-cc[order-1],0.0));
    }

    for (int i=r-1;i>=0;i--) {
        double  im = sqrt(aa[i]);
        Zeros.append(std::complex<float>(0,-im));
        double  re = -0.5*bb[i];
        im = 0.5*sqrt(-1.0*bb[i]*bb[i]+4*cc[i]);
        Poles.append(std::complex<float>(re,-im));
    }
}

void Filter::calcBessel()
{
    if (order <= 0 || order >= static_cast<int>(BesselPolesSize)) {
        throw FilterError(QObject::tr("Invalid Filter Order: %1").arg(order));
    }

    Poles.clear();
    Zeros.clear();

    for (int i=0;i<order;i++) {
        Poles.append(std::complex<float>(BesselPoles[order-1][2*i],BesselPoles[order-1][2*i+1]));
    }

    reformPolesZeros();
}

void Filter::calcUserTrFunc()
{
    if (vec_A.isEmpty() || vec_B.isEmpty()) {
        throw FilterError(QObject::tr("Filter Coefficients are missing for User Function!"));
    }

    long double *a = vec_A.data();
    long double *b = vec_B.data();

    int a_order = vec_A.count() - 1;
    int b_order = vec_B.count() - 1;

    order = std::max(a_order,b_order);
    if (order > MaxOrder) {
        throw FilterError(QObject::tr("Too high Filter Order: %1").arg(order));
    }

    qf_poly Numenator(b_order,b);
    qf_poly Denomenator(a_order,a);

    Numenator.to_roots();
    Denomenator.to_roots();

    Numenator.disp_c();
    Denomenator.disp_c();

    Numenator.roots_to_complex(Zeros);
    Denomenator.roots_to_complex(Poles);

    reformPolesZeros();
}

void Filter::reformPolesZeros()
{
    auto reform = [this](QVector< std::complex<float> > & data)
    {
        int Np = data.count();
        for (int i=0; i < Np/2; ++i) {
            if ((i%2) == 0) {
                continue;
            }
            ::std::swap(data[i], data[Np-1-i]);
        }
    };

    reform(Poles);
    reform(Zeros);
}

void Filter::set_TrFunc(QVector<long double> a, QVector<long double> b)
{
    vec_A = a;
    vec_B = b;
}

