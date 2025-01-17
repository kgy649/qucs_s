/***************************************************************************
                               ampere_ac.cpp
                              ---------------
    begin                : Sun May 23 2004
    copyright            : (C) 2003 by Michael Margraf
    email                : michael.margraf@alumni.tu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ampere_ac.h"
#include "node.h"
#include "misc.h"
#include "extsimkernels/spicecompat.h"


Ampere_ac::Ampere_ac()
{
  Description = QObject::tr("ideal ac current source");

  Arcs.append(new qucs::Arc(-12,-12, 24, 24,  0, 16*360,QPen(Qt::darkBlue,2)));
  Lines.append(new qucs::Line(-30,  0,-12,  0,QPen(Qt::darkBlue,2)));
  Lines.append(new qucs::Line( 30,  0, 12,  0,QPen(Qt::darkBlue,2)));
  Lines.append(new qucs::Line( -7,  0,  7,  0,QPen(Qt::darkBlue,3)));
  Lines.append(new qucs::Line(  6,  0,  0, -4,QPen(Qt::darkBlue,3)));
  Lines.append(new qucs::Line(  6,  0,  0,  4,QPen(Qt::darkBlue,3)));
  Arcs.append(new qucs::Arc( 12,  5,  6,  6,16*270, 16*180,QPen(Qt::darkBlue,2)));
  Arcs.append(new qucs::Arc( 12, 11,  6,  6, 16*90, 16*180,QPen(Qt::darkBlue,2)));

  Ports.append(new Port( 30,  0));
  Ports.append(new Port(-30,  0));

  x1 = -30; y1 = -14;
  x2 =  30; y2 =  16;

  tx = x1+4;
  ty = y2+4;
  Model = "Iac";
  Name  = "I";
  SpiceModel = "I";

  Props.append(new Property("I", "1 mA", true,
		QObject::tr("peak current in Ampere")));
  Props.append(new Property("f", "1 GHz", false,
		QObject::tr("frequency in Hertz")));
  Props.append(new Property("Phase", "0", false,
		QObject::tr("initial phase in degrees")));
  Props.append(new Property("Theta", "0", false,
		QObject::tr("damping factor (transient simulation only)")));

  rotate();  // fix historical flaw
}

Ampere_ac::~Ampere_ac()
{
}

Component* Ampere_ac::newOne()
{
  return new Ampere_ac();
}

Element* Ampere_ac::info(QString& Name, char* &BitmapFile, bool getNewOne)
{
  Name = QObject::tr("ac Current Source");
  BitmapFile = (char *) "ac_current";

  if(getNewOne)  return new Ampere_ac();
  return 0;
}


QString Ampere_ac::spice_netlist(bool)
{
    QString s = spicecompat::check_refdes(Name,SpiceModel);
    for (Port *p1 : Ports) {
        QString nam = p1->Connection->Name;
        if (nam=="gnd") nam = "0";
        s += " "+ nam;   // node names
    }

    QString amperes = spicecompat::normalize_value(Props.at(0)->Value);
    QString freq = spicecompat::normalize_value(Props.at(1)->Value);

    QString theta = Props.at(3)->Value;
    theta.remove(' ');
    if (theta.isEmpty()) theta="0";
    s += QString(" DC 0 SIN(0 %1 %2 0 %3) AC %4\n").arg(amperes).arg(freq).arg(theta).arg(amperes);
    return s;
}
