#pragma once

#include "Arduino.h"
#include "OLEDInterface.h"

// Channels of the PCA9548A multiplexer, as wired on the FCU mainboard.
// Each one is the connector the display cable plugs into:
//   0 J1 R_OLED   1 J2 L_OLED   2 J3 SPD_OLED   3 J4 HDG_OLED
//   4 J5 H_T_OLED 5 J14 ALT_OLED 6 J15 VSPEED_OLED
// J1 sits in the R_ connector cluster, J2 in the L_ one, so the left
// EFIS is on channel 1 and the right one on channel 0.
#define TCA9548A_I2C_ADDRESS  0x70
#define TCA9548A_CHANNEL_EFIS_LEFT  1
#define TCA9548A_CHANNEL_EFIS_RIGHT 0
#define TCA9548A_CHANNEL_FCU_SPD    2
#define TCA9548A_CHANNEL_FCU_HDG    3
#define TCA9548A_CHANNEL_FCU_FPA    4
#define TCA9548A_CHANNEL_FCU_ALT    5
#define TCA9548A_CHANNEL_FCU_VS     6
#define TCA9548A_CHANNEL_UNUSED     7


class FCU_EFIS
{
public:
    FCU_EFIS();
    void begin();
    void attach(uint8_t addrI2C);
    void detach();
    void set(int16_t messageID, char *message);
    void update();

private:
    bool          _initialised;
    uint8_t       _addrI2C;
    OLEDInterface *oled;

    void setTCAChannel(byte i);
    void updateDisplayEfisLeft(void);
    void updateDisplayEfisRight(void);
    void updateDisplayFcuSpd(void);
    void updateDisplayFcuHdg(void);
    void updateDisplayFcuFpa(void);
    void updateDisplayFcuAlt(void);
    void updateDisplayFcuVs(void);

};
