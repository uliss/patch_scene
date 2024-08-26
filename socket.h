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
#ifndef SOCKET_H
#define SOCKET_H

#include <QObject>

#include <functional>

namespace ceam {

enum class ConnectorModel : std::uint8_t {
    UNKNOWN,
    // audio
    XLR,
    XLT_5PIN,
    XLR_JACK_COMBO,
    XLR_MINI,
    JACK_TS,
    JACK_TRS,
    JACK_TRRS,
    JACK_TRRRS,
    JACK_MINI_TS,
    JACK_MINI_TRS,
    JACK_MINI_TRRS,
    JACK_MICRO_TS,
    JACK_MICRO_TRS,
    JACK_MICRO_TRRS,
    RCA,
    DIN_41529,
    DIN_5,
    // digital audio
    TOSLINK,
    TOSLINK_MINI,
    SPDIF,
    // computer
    USB_A,
    USB_B,
    USB_MINI_A,
    USB_MINI_B,
    USB_MICRO_A,
    USB_MICRO_B,
    USB_C,
    USB_3_0_A,
    USB_3_0_B,
    USB_3_0_MICRO_B,
    LIGHTNING,
    FIREWIRE_400,
    FIREWIRE_600,
    FIREWIRE_800,
    ETHERNET,
    PS_2,
    DIN_3,
    DIN_4,
    DIN_MIDI,
    DIN_6,
    DIN_7,
    DIN_8,
    DIN_MINI_3,
    DIN_MINI_4,
    DIN_MINI_5,
    DIN_MINI_6,
    DIN_MINI_7,
    DIN_MINI_8,
    // video
    SCART,
    HDMI,
    HDMI_MINI,
    HDMI_MICRO,
    DVI,
    DISPLAY_PORT,
    DISPLAY_PORT_MINI,
    VGA,
    THUNDERBOLT,
    BNC,
    // power
    SPEAKON,
    POWERCON,
    POWER_TYPE_SCHUKO,
    POWER_TYPE_A,
    POWER_TYPE_B,
    POWER_TYPE_C,
    POWER_IEC_C_1_2,
    POWER_IEC_C_7_8,
    POWER_IEC_C_13_14,
    POWER_IEC_C_15_16,
    POWER_IEC_C_17_18,
    POWER_DC_3_8x1_3,
    POWER_DC_3_5x1_35,
    POWER_DC_4_0x1_7,
    POWER_DC_4_75x1_7,
    POWER_DC_5_5x2_1,
    POWER_DC_5_5x2_5,
    // DC_5_5x2_1,
    CONNECTOR_MAX,
};

const QString& connectorSvgName(ConnectorModel model);
const QString& connectorJsonName(ConnectorModel model);
const QString& connectorName(ConnectorModel model);
ConnectorModel findConnectorBySvgName(const QString& name);
ConnectorModel findConnectorByJsonName(const QString& name);

enum class ConnectorType : std::uint8_t {
    SocketMale,
    SocketFemale,
    PlugMale,
    PlugFemale,
    MaxConnectorType,
};

enum class XletType : std::uint8_t {
    None,
    In,
    Out,
};

QString connectorTypeJsonName(ConnectorType type);
QString connectorTypeName(ConnectorType type);
bool connectorIsSocket(ConnectorType type);
bool connectorIsPlug(ConnectorType type);
void foreachConnectorType(const std::function<void(ConnectorType, int /*idx*/)>& fn);

enum class PowerType : std::uint8_t {
    None,
    DC_Positive,
    DC_Negative,
    AC,
    AC_DC,
    Phantom,
    //
    MaxPowerType,
};

QString powerTypeToString(PowerType type);
std::optional<PowerType> powerTypeFromString(const QString& str);
void foreachPowerType(const std::function<void(PowerType, int /*idx*/)>& fn);

}

#endif // SOCKET_H
