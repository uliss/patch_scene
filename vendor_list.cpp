/*****************************************************************************
 * Copyright 2024 Serge Poltavski. All rights reserved.
 *
 * This file may be distributed under the terms of GNU Public License version
 * 3 (GPL v3) as defined by the Free Software Foundation (FSF). A copy of the
 * license should have been included with this file, or the project in which
 * this file belongs to. You may also find the details of GPL v3 at:
 * http://www.gnu.org/licenses/gpl-3.0.txt
 *
 * If you have any questions regarding the use of this file, feel free to
 * contact the author of this file, or the owner of the project in which
 * this file belongs to.
 *****************************************************************************/
#include "vendor_list.h"

#include <QSet>

using namespace ceam;

VendorList::VendorList()
{
    mics_
        << "Akai"
        << "AKG"
        << "Astatic"
        << "Audio-Technica"
        << "Behringer"
        << "Beyerdynamic"
        << "Blue Microphones"
        << "Brauner"
        << "Brüel & Kjær"
        << "CAD Audio"
        << "Core Sound LLC"
        << "DJI"
        << "DPA"
        << "Earthworks"
        << "Electro-Voice"
        << "Fostex"
        << "Gauge Precision Instruments"
        << "Gentex Corp"
        << "Grundig"
        << "Heil Sound"
        << "JZ Microphones"
        << "Klark Teknik"
        << "Lauten Audio"
        << "Line 6"
        << "Manley Laboratories"
        << "M-Audio"
        << "Microtech Gefell"
        << "Milab"
        << "MIPRO"
        << "Nady Systems, Inc."
        << "Georg Neumann GmbH"
        << "Nevaton"
        << "NTi Audio"
        << "Oktava"
        << "PCB Piezotronics"
        << "Peavey Electronics"
        << "Philips"
        << "Røde Microphones"
        << "Royer Labs"
        << "Schoeps"
        << "Sennheiser"
        << "Shure"
        << "Sony"
        << "TASCAM/TEAC Corporation"
        << "TOA Corp."
        << "Zoom Corporation"
        /**/;

    phones_
        << "AKG"
        << "Anker Innovations"
        << "Arctic"
        << "Astell & Kern"
        << "Audio-Technica"
        << "Austrian Audio"
        << "Bang & Olufsen"
        << "Beats Electronics"
        << "Beyerdynamic"
        << "Bose Corporation"
        << "Bush"
        << "Creative Technology"
        << "David Clark Company"
        << "Definitive Technology"
        << "Denon"
        << "Doppler Labs"
        << "Earin"
        << "Edifier"
        << "Fostex"
        << "Fujikon"
        << "Grado Labs"
        << "Happy Plugs"
        << "HiFiMan"
        << "IBall"
        << "Jabra"
        << "Jaybird"
        << "JBL"
        << "JLab Audio"
        << "JVC"
        << "JVCKenwood"
        << "Kenwood Corporation"
        << "Klipsch Audio Technologies"
        << "Koss Corporation"
        << "Livv Headphones"
        << "Master & Dynamic"
        << "Meze Audio"
        << "Monster Cable"
        << "Muzik"
        << "Onkyo"
        << "Oppo"
        << "Panasonic"
        << "Phiaton"
        << "Philips"
        << "Pioneer Corporation"
        << "Plantronics"
        << "Plantronics Gamecom"
        << "RCF audio"
        << "Sennheiser"
        << "Shure"
        << "Skullcandy"
        << "SMS Audio"
        << "Sol Republic"
        << "Sony"
        << "Stax Ltd"
        << "Technics"
        << "Thinksound"
        << "Turtle Beach Corporation"
        << "Ultimate Ears"
        << "Ultrasone"
        << "Urbanears"
        << "V-Moda"
        << "Wyze Labs"
        << "Yamaha Corporation"
        << "Zu Audio"
        /**/;

    mix_ << "Alesis"
         << "Allen & Heath"
         << "AMS Neve"
         << "Audient"
         << "Audiotonix"
         << "Automated Processes, Inc."
         << "Avid Audio"
         << "Behringer"
         << "Cadac Electronics"
         << "Crest Audio"
         << "D&R"
         << "DiGiCo"
         << "Electro-Voice"
         << "Euphonix"
         << "Fairlight (company)"
         << "Focusrite"
         << "Harrison Audio Consoles"
         << "Helios Electronics"
         << "Klotz Digital"
         << "Lawo"
         << "Mackie"
         << "Midas Consoles"
         << "Music Center Incorporated"
         << "Neve Electronics"
         << "Peavey Electronics"
         << "Phonic Corporation"
         << "PreSonus"
         << "Rane Corporation"
         << "Roland Corporation"
         << "Shure"
         << "Solid State Logic"
         << "Soundcraft"
         << "Speck Electronics"
         << "Studer"
         << "Studiomaster"
         << "TASCAM"
         << "Ward-Beck Systems"
         << "Yamaha Pro Audio"
         << "Yorkville Sound"
        /**/;
}

VendorList& VendorList::instance()
{
    static VendorList vendors_;
    return vendors_;
}

QStringList VendorList::all() const
{
    QSet<QString> all_;
    all_.unite(mics_);
    all_.unite(phones_);
    all_.unite(mix_);
    return all_.values();
}
