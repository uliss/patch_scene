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
#include "socket.h"
#include "QtCore/qdebug.h"

#include <map>

using namespace ceam;

namespace {
struct ConnectorModelData {
    QString json_name;
    QString svg_name;
    QString name;
};

using ConnectorModelMap = std::map<ConnectorModel, ConnectorModelData>;

static const ConnectorModelMap& model_map()
{
    static ConnectorModelMap map {
        // clang-format off
        { ConnectorModel::UNKNOWN,                 { "unknown",             "unknown",          "Unknown" } },
        // audio
        { ConnectorModel::XLR,                     { "xlr",                 "xlr",              "XLR" } },
        { ConnectorModel::XLT_5PIN,                { "xlr5",                "xlr5",             "XLR 5-pin" } },
        { ConnectorModel::XLR_JACK_COMBO,          { "xlr_jack",            "xlr_jack",         "XLR/Jack combo" } },
        { ConnectorModel::XLR_MINI,                { "xlr_mini",            "xlr_mini",         "XLR mini" } },
        { ConnectorModel::JACK_TS,                 { "jack_ts",             "jack",             "Jack TS (6.3, Mono)" } },
        { ConnectorModel::JACK_TRS,                { "jack_trs",            "jack",             "Jack TRS (6.3, Stereo)" } },
        { ConnectorModel::JACK_TRRS,               { "jack_trrs",           "jack",             "Jack TRRS" } },
        { ConnectorModel::JACK_TRRRS,              { "jack_trrrs",          "jack",             "Jack TRRRS" } },
        { ConnectorModel::JACK_MINI_TS,            { "jack_mini_ts",        "jack_mini",        "Jack mini TS (3.5, Mono)" } },
        { ConnectorModel::JACK_MINI_TRS,           { "jack_mini_trs",       "jack_mini",        "Jack mini TRS (3.5, Stereo)" } },
        { ConnectorModel::JACK_MINI_TRRS,          { "jack_mini_trrs",      "jack_mini",        "Jack mini TRRS" } },
        { ConnectorModel::JACK_MICRO_TS,           { "jack_micro_ts",       "jack_micro",       "Jack micro TS (2.5, Mono)" } },
        { ConnectorModel::JACK_MICRO_TRS,          { "jack_micro_trs",      "jack_micro",       "Jack micro TRS (2.5, Stereo)" } },
        { ConnectorModel::JACK_MICRO_TRRS,         { "jack_micro_trrs",     "jack_micro",       "Jack micro TRRS" } },
        { ConnectorModel::RCA,                     { "rca",                 "rca",              "RCA" } },
        { ConnectorModel::DIN_41529,               { "din_41529",           "din_41529",        "DIN acoustic" } },
        // digital audio
        { ConnectorModel::TOSLINK,                 { "toslink",             "toslink",          "Toslink" } },
        { ConnectorModel::TOSLINK_MINI,            { "toslink_mini",        "toslink_mini",     "Toslink mini" } },
        { ConnectorModel::SPDIF,                   { "spdif",               "rca_spdif",        "SPDIF" } },
        // computer
        { ConnectorModel::USB_A,                   { "usb_a",               "usb_a",            "USB 2.0 type A" } },
        { ConnectorModel::USB_B,                   { "usb_b",               "usb_b",            "USB 2.0 type B" } },
        { ConnectorModel::USB_MINI_A,              { "usb_mini_a",          "usb_mini_a",       "USB 2.0 mini type A" } },
        { ConnectorModel::USB_MINI_B,              { "usb_mini_b",          "usb_mini_b",       "USB 2.0 mini type B" } },
        { ConnectorModel::USB_MICRO_A,             { "usb_micro_a",         "usb_micro_a",      "USB 2.0 micro type A" } },
        { ConnectorModel::USB_MICRO_B,             { "usb_micro_b",         "usb_micro_b",      "USB 2.0 micro type B" } },
        { ConnectorModel::USB_C,                   { "usb_type_c",          "usb_type_c",       "USB type C" } },
        { ConnectorModel::USB_3_0_A,               { "usb3_a",              "usb3_a",           "USB 3.0 type A" } },
        { ConnectorModel::USB_3_0_B,               { "usb3_b",              "usb3_b",           "USB 3.0 type B" } },
        { ConnectorModel::USB_3_0_MICRO_B,         { "usb3_micro_b",        "usb3_micro_b",     "USB micro super speed" } },
        { ConnectorModel::LIGHTNING,               { "lightning",           "lightning",        "Lightning" } },
        { ConnectorModel::FIREWIRE_400,            { "firewire_400",        "firewire_400",     "Firewire 400" } },
        { ConnectorModel::FIREWIRE_600,            { "firewire_600",        "firewire_600",     "Firewire 600" } },
        { ConnectorModel::FIREWIRE_800,            { "firewire_800",        "firewire_800",     "Firewire 800" } },
        { ConnectorModel::ETHERNET,                { "ethernet",            "ethernet",         "Ethernet RJ45" } },
        { ConnectorModel::PS_2,                    { "ps2",                 "ps2",              "PS/2" } },
        { ConnectorModel::DIN_3,                   { "din_3",               "din_3",            "DIN 3" } },
        { ConnectorModel::DIN_4,                   { "din_4",               "din_4",            "DIN 4" } },
        { ConnectorModel::DIN_MIDI,                { "midi",                "midi",             "MIDI (DIN 5)" } },
        { ConnectorModel::DIN_5,                   { "din_5",               "midi",             "DIN 5 (Stereo)" } },
        { ConnectorModel::DIN_6,                   { "din_6",               "din_6",            "DIN 6" } },
        { ConnectorModel::DIN_7,                   { "din_7",               "din_7",            "DIN 7" } },
        { ConnectorModel::DIN_8,                   { "din_8",               "din_8",            "DIN 8" } },
        { ConnectorModel::DIN_MINI_3,              { "din_mini_3",          "din_mini_3",       "DIN 3 mini" } },
        { ConnectorModel::DIN_MINI_4,              { "din_mini_4",          "din_mini_4",       "DIN 4 mini" } },
        { ConnectorModel::DIN_MINI_5,              { "din_mini_5",          "din_mini_5",       "DIN 5 mini" } },
        { ConnectorModel::DIN_MINI_6,              { "din_mini_6",          "din_mini_6",       "DIN 6 mini" } },
        { ConnectorModel::DIN_MINI_7,              { "din_mini_7",          "din_mini_7",       "DIN 7 mini" } },
        { ConnectorModel::DIN_MINI_8,              { "din_mini_8",          "din_mini_8",       "DIN 8 mini" } },
        // video
        { ConnectorModel::SCART,                   { "scart",               "scart",            "SCART" } },
        { ConnectorModel::HDMI,                    { "hdmi",                "hdmi",             "HDMI" } },
        { ConnectorModel::HDMI_MINI,               { "hdmi_mini",           "hdmi_mini",        "HDMI mini" } },
        { ConnectorModel::HDMI_MICRO,              { "hdmi_micro",          "hdmi_micro",       "HDMI micro" } },
        { ConnectorModel::DVI,                     { "dvi",                 "dvi",              "DVI" } },
        { ConnectorModel::VGA,                     { "vga",                 "vga",              "VGA" } },
        { ConnectorModel::DISPLAY_PORT,            { "display_port",        "display_port",     "Display Port" } },
        { ConnectorModel::DISPLAY_PORT_MINI,       { "display_port_mini",   "display_port_mini","Display Port mini" } },
        { ConnectorModel::THUNDERBOLT,             { "thunderbolt",         "thunderbolt",      "Thunderbolt" } },
        { ConnectorModel::BNC,                     { "bnc",                 "bnc",              "BNC" } },
        // power
        { ConnectorModel::SPEAKON,                 { "speakon",             "speakon",          "Speakon" } },
        { ConnectorModel::POWERCON,                { "powercon",            "powercon",         "Powercon" } },
        { ConnectorModel::POWER_TYPE_SCHUKO,       { "power_schuko",        "power_schuko",     "Power Schuko (type F)" } },
        { ConnectorModel::POWER_TYPE_A,            { "power_type_a",        "power_type_a",     "Power type A" } },
        { ConnectorModel::POWER_TYPE_B,            { "power_type_b",        "power_type_b",     "Power type B" } },
        { ConnectorModel::POWER_TYPE_C,            { "power_type_c",        "power_type_b",     "Power type C" } },
        { ConnectorModel::POWER_IEC_C_1_2,         { "power_iec_c_1_2",     "power_iec_c_1_2",  "Power IEC C1/C2" } },
        { ConnectorModel::POWER_IEC_C_7_8,         { "power_iec_c_7_8",     "power_iec_c_7_8",  "Power IEC C7/C8" } },
        { ConnectorModel::POWER_IEC_C_13_14,       { "power_iec_c_13_14",   "power_iec_c_13_14b","Power IEC C13/C14" } },
        { ConnectorModel::POWER_IEC_C_15_16,       { "power_iec_c_15_16",   "power_iec_c_15_16","Power IEC C15/C16" } },
        { ConnectorModel::POWER_IEC_C_17_18,       { "power_iec_c_17_18",   "power_iec_c_17_18","Power IEC C17/C18" } },
        { ConnectorModel::POWER_DC_3_8x1_3,        { "power_dc_3.8x1.3",    "power_dc",         "Power DC 3.8/1.3mm" } },
        { ConnectorModel::POWER_DC_4_0x1_7,        { "power_dc_4.0x1.7",    "power_dc",         "Power DC 4.0/1.7mm" } },
        { ConnectorModel::POWER_DC_4_75x1_7,       { "power_dc_4.75x1.7",   "power_dc",         "Power DC 4.75/1.7mm" } },
        { ConnectorModel::POWER_DC_5_5x2_1,        { "power_dc_5.5x2.1",    "power_dc",         "Power DC 5.5/2.1mm" } },
        { ConnectorModel::POWER_DC_5_5x2_5,        { "power_dc_5.5x2.5",    "power_dc",         "Power DC 5.5/2.5mm" } },
        { ConnectorModel::POWER_DC_3_5x1_35,       { "power_dc_3.5x1.35",   "power_dc",         "Power DC 3.5/1.35mm" } },

        // clang-format on
    };
    return map;
}

}

const QString& ceam::connectorSvgName(ConnectorModel model)
{
    static const QString unknown("???");
    auto& data = model_map();
    auto it = model_map().find(model);

    return it == data.end() ? unknown : it->second.svg_name;
}

const QString& ceam::connectorJsonName(ConnectorModel model)
{
    static const QString unknown("???");
    auto& data = model_map();
    auto it = model_map().find(model);

    return it == data.end() ? unknown : it->second.json_name;
}

const QString& ceam::connectorName(ConnectorModel model)
{
    static const QString unknown("???");
    auto& data = model_map();
    auto it = model_map().find(model);

    return it == data.end() ? unknown : it->second.name;
}

ConnectorModel ceam::findConnectorBySvgName(const QString& name)
{
    auto& data = model_map();
    for (auto& kv : data) {
        if (kv.second.svg_name == name)
            return kv.first;
    }

    return ConnectorModel::UNKNOWN;
}

ConnectorModel ceam::findConnectorByJsonName(const QString& name)
{
    auto& data = model_map();
    for (auto& kv : data) {
        if (kv.second.json_name == name)
            return kv.first;
    }

    return ConnectorModel::UNKNOWN;
}

QString ceam::connectorTypeName(ConnectorType type)
{
    switch (type) {
    case ConnectorType::Socket_Male:
        return "socket_male";
    case ConnectorType::Socket_Female:
        return "socket_female";
    case ConnectorType::Plug_Male:
        return "plug_male";
    case ConnectorType::Plug_Female:
        return "plug_female";
    default:
        return "???";
    }
}

bool ceam::connectorIsSocket(ConnectorType type)
{
    return type == ConnectorType::Socket_Female || type == ConnectorType::Socket_Male;
}

bool ceam::connectorIsPlug(ConnectorType type)
{
    return type == ConnectorType::Plug_Female || type == ConnectorType::Plug_Male;
}

QString ceam::powerTypeToString(PowerType type)
{
    switch (type) {

    case PowerType::DC_Positive:
        return "+";
    case PowerType::DC_Negative:
        return "-";
    case PowerType::AC:
        return "~";
    case PowerType::AC_DC:
        return "ac/dc";
    case PowerType::Phantom:
        return "phantom";
    case PowerType::None:
    default:
        return {};
    }
}

std::optional<PowerType> ceam::powerTypeFromString(const QString& str)
{
    if (str.isEmpty())
        return PowerType::None;

    const auto lstr = str.toLower();
    if (lstr == "+")
        return PowerType::DC_Positive;
    else if (lstr == "-")
        return PowerType::DC_Negative;
    else if (lstr == "~")
        return PowerType::AC;
    else if (lstr == "ac/dc")
        return PowerType::AC_DC;
    else if (lstr == "phantom")
        return PowerType::Phantom;
    else
        return {};
}

void ceam::foreachPowerType(const std::function<void(PowerType, int)>& fn)
{
    for (int i = static_cast<int>(PowerType::None);
         i < static_cast<int>(PowerType::MaxPowerType);
         i++) {
        fn(static_cast<PowerType>(i), i);
    }
}
