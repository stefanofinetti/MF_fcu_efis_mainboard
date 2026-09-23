#include "FCU_EFIS.h"
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include "Fonts/FreeSans8pt7b.h"
#include "Fonts/DSEG7Classic_Regular15pt7b.h"
#include "Fonts/DSEG7Classic_Regular20pt7b.h" //https://github.com/keshikan/DSEG and https://rop.nl/truetype2gfx/
#include "Fonts/DSEG7Classic_Regular22pt7b.h"

/* **********************************************************************************
    The values arrive from the Connector as strings and are kept in fixed buffers
    rather than String objects: nothing is allocated at runtime, so the heap cannot
    fragment while the displays are being redrawn.

    setValue() zero fills whatever follows the string, which matters: the padding
    code below reads past the end of a value on purpose and expects to find 0
    there. That was what Arduino's String did, and it is reproduced here.
********************************************************************************** */
#define VALUE_LEN 8

static void setValue(char *dst, const char *src)
{
    strncpy(dst, src, VALUE_LEN - 1);
    dst[VALUE_LEN - 1] = 0x00;
}

// Efis left
uint8_t efisLeftBaroMode              = 0x00;
uint8_t efisLeftBaroSelect            = 0x00;
char    efisLeftBaroValueHpa[VALUE_LEN] = "1013";
char    efisLeftBaroValueHg[VALUE_LEN]  = "2992";

// Efis right
uint8_t efisRightBaroMode              = 0x00;
uint8_t efisRightBaroSelect            = 0x00;
char    efisRightBaroValueHpa[VALUE_LEN] = "1013";
char    efisRightBaroValueHg[VALUE_LEN]  = "2992";

// FCU Speed
uint8_t fcuSpeedManagedMode        = 0x00;
char    fcuSpeedValue[VALUE_LEN]   = "000";
uint8_t fcuSpeedMode               = 0x00;

// FCU Hdg
uint8_t fcuHdgManagedMode      = 0x00;
char    fcuHdgValue[VALUE_LEN] = "000";

// FCU Trk Mode
uint8_t fcuTrkMode = 0x00;

// FCU Alt
uint8_t fcuAltManagedMode      = 0x00;
char    fcuAltValue[VALUE_LEN] = "00000";

// FCU VS
uint8_t fcuVsManagedMode          = 0x00;
char    fcuVsValue[VALUE_LEN]     = "00000";
char    fcuVsValueFpa[VALUE_LEN]  = "-0.0";

// light test switch
uint8_t lightTestOn = 0x00;

FCU_EFIS::FCU_EFIS()
{
    _initialised = false;
}

void FCU_EFIS::attach(uint8_t addrI2C)
{
    _addrI2C = addrI2C;
    Wire.begin();
    Wire.setClock(400000);
    void *mem = MF_ALLOC_TYPE(OLEDInterface, 1);
    if (!mem) {
        // Error Message to Connector
        cmdMessenger.sendCmd(kStatus, F("Custom Device does not fit in Memory"));
        return;
    }
    // an odd address selects the SSD1306 driver, an even one the SH1106
    if (_addrI2C & 0x01) {
        oled = new (mem) OLEDInterface(SSD1306);
    } else {
        oled = new (mem) OLEDInterface(SH1106);
    }
    _initialised = true;
}

void FCU_EFIS::begin()
{
    if (!_initialised)
        return;

    //**********************************************
    // EFIS left
    //**********************************************
    setTCAChannel(TCA9548A_CHANNEL_EFIS_LEFT);
    oled->begin(SCREEN_ADDRESS, true); // Address 0x3C default
    updateDisplayEfisLeft();

    //**********************************************
    // EFIS right
    //**********************************************
    setTCAChannel(TCA9548A_CHANNEL_EFIS_RIGHT);
    oled->begin(SCREEN_ADDRESS, true); // Address 0x3C default
    updateDisplayEfisRight();

    //**********************************************
    // FCU SPD
    //**********************************************
    setTCAChannel(TCA9548A_CHANNEL_FCU_SPD);
    oled->begin(SCREEN_ADDRESS, true); // Address 0x3C default
    updateDisplayFcuSpd();

    //**********************************************
    // FCU HDG
    //**********************************************
    setTCAChannel(TCA9548A_CHANNEL_FCU_HDG);
    oled->begin(SCREEN_ADDRESS, true); // Address 0x3C default
    updateDisplayFcuHdg();

    //**********************************************
    // FCU HDG and V/S or TRK and FPA.
    //**********************************************
    setTCAChannel(TCA9548A_CHANNEL_FCU_FPA);
    oled->begin(SCREEN_ADDRESS, true); // Address 0x3C default
    updateDisplayFcuFpa();

    //**********************************************
    // FCU ALT
    //**********************************************
    setTCAChannel(TCA9548A_CHANNEL_FCU_ALT);
    oled->begin(SCREEN_ADDRESS, true); // Address 0x3C default
    updateDisplayFcuAlt();

    //**********************************************
    // FCU Vs
    //**********************************************
    setTCAChannel(TCA9548A_CHANNEL_FCU_VS);
    oled->begin(SCREEN_ADDRESS, true); // Address 0x3C default
    updateDisplayFcuVs();
}

void FCU_EFIS::detach()
{
    if (!_initialised)
        return;
    _initialised = false;
}

void FCU_EFIS::set(int16_t messageID, char *message)
{
    /* **********************************************************************************
        Each messageID has it's own value
        check for the messageID and define what to do.
        Important Remark!
        MessageID == -1 will be send from the connector when Mobiflight is closed
        Put in your code to shut down your custom device (e.g. clear a display)
        MessageID == -2 will be send from the connector when PowerSavingMode is entered
        Put in your code to enter this mode (e.g. clear a display)

    ********************************************************************************** */
    // do something according your messageID
    switch (messageID) {
    case 0:
        // Efis Left Baro Select
        // inHg=0, hPa=1
        efisLeftBaroSelect = atoi(message);
        updateDisplayEfisLeft();
        break;

    case 1:
        // Efis Left Baro Value Hpa
        setValue(efisLeftBaroValueHpa, message);
        updateDisplayEfisLeft();
        break;

    case 2:
        // Efis Left Baro Value Hg
        setValue(efisLeftBaroValueHg, message);
        updateDisplayEfisLeft();
        break;

    case 3:
        // Efis Left Baro Mode
        // 0 = QFE; 1 = QNH; 2 = STD
        efisLeftBaroMode = atoi(message);
        updateDisplayEfisLeft();
        break;

    case 4:
        // Efis Right Baro Select
        // inHg=0, hPa=1
        efisRightBaroSelect = atoi(message);
        updateDisplayEfisRight();
        break;

    case 5:
        // Efis Right Baro Value Hpa
        setValue(efisRightBaroValueHpa, message);
        updateDisplayEfisRight();
        break;

    case 6:
        // Efis Right Baro Value Hg
        setValue(efisRightBaroValueHg, message);
        updateDisplayEfisRight();
        break;

    case 7:
        // Efis Right Baro Mode
        // 0 = QFE; 1 = QNH; 2 = STD
        efisRightBaroMode = atoi(message);
        updateDisplayEfisRight();
        break;

    case 8:
        // Fcu Speed Value
        setValue(fcuSpeedValue, message);
        updateDisplayFcuSpd();
        break;

    case 9:
        // Fcu Speed Managed
        // 0 = No; 1 = Yes
        fcuSpeedManagedMode = atoi(message);
        updateDisplayFcuSpd();
        break;

    case 10:
        // Fcu Hdg Value
        setValue(fcuHdgValue, message);
        updateDisplayFcuHdg();
        break;

    case 11:
        // Fcu Hdg Managed
        // 0 = No; 1 = Yes
        fcuHdgManagedMode = atoi(message);
        updateDisplayFcuHdg();
        break;

    case 12:
        // Fcu Trk Mode
        // 0 = No; 1 = Yes
        fcuTrkMode = atoi(message);
        updateDisplayFcuHdg();
        updateDisplayFcuFpa();
        updateDisplayFcuVs();
        break;

    case 13:
        setValue(fcuAltValue, message);
        updateDisplayFcuAlt();
        break;

    case 14:
        fcuAltManagedMode = atoi(message);
        updateDisplayFcuAlt();
        break;

    case 15:
        setValue(fcuVsValue, message);
        updateDisplayFcuVs();
        break;

    case 16:
        setValue(fcuVsValueFpa, message);
        updateDisplayFcuVs();
        break;

    case 17:
        fcuVsManagedMode = atoi(message);
        updateDisplayFcuVs();
        break;

    case 18:
        fcuSpeedMode = atoi(message);
        updateDisplayFcuSpd();
        break;

    case 19:
        lightTestOn = atoi(message);
        updateDisplayEfisLeft();
        updateDisplayEfisRight();
        updateDisplayFcuSpd();
        updateDisplayFcuHdg();
        updateDisplayFcuAlt();
        updateDisplayFcuFpa();
        updateDisplayFcuVs();
        break;

    default:
        break;
    }
}

void FCU_EFIS::update()
{
    // Do something which is required regulary
    updateDisplayEfisLeft();
    updateDisplayEfisRight();
    updateDisplayFcuSpd();
    updateDisplayFcuHdg();
    updateDisplayFcuAlt();
    updateDisplayFcuFpa();
    updateDisplayFcuVs();
}

/* ************************************************************************************************
 ************************************************************************************************
 ************************************************************************************************ */

/*
  switch multiplexer channel
*/
void FCU_EFIS::setTCAChannel(byte i)
{
    Wire.beginTransmission(_addrI2C);
    Wire.write(1 << i);
    Wire.endTransmission();
    delay(5); // Pause
}

/* **********************************************************************************
    Both EFIS displays draw the same thing, each from its own set of values, so one
    function serves both: it is told which multiplexer channel to talk to and which
    values to read.
********************************************************************************** */
void FCU_EFIS::updateDisplayEfis(uint8_t channel, uint8_t baroMode,
                                 uint8_t baroSelect, const char *valueHpa,
                                 const char *valueHg)
{
    setTCAChannel(channel);
    // Clear the buffer
    oled->clearDisplay();
    oled->setTextColor(SSD1306_WHITE);
    if (lightTestOn == 1) {
        oled->setFont(&FreeSans9pt7b);
        oled->setTextSize(1);
        oled->setCursor(20, 15);
        oled->println("QFE");
        oled->setCursor(85, 15);
        oled->println("QNH");
        oled->setFont(&DSEG7Classic_Regular20pt7b);
        oled->setCursor(0, 60);
        oled->println("8888");
        oled->fillCircle(64, 60, 2, SSD1306_WHITE);
    } else {
        if (baroMode == 2 || baroMode == 3) {
            oled->setFont(&DSEG7Classic_Regular22pt7b);
            oled->setCursor(10, 60);
            oled->println("5td");
        } else {
            if (baroMode == 0) {
                oled->setFont(&FreeSans9pt7b);
                oled->setTextSize(1);
                oled->setCursor(0, 15);
                oled->println("QFE");
            } else {
                oled->setFont(&FreeSans9pt7b);
                oled->setTextSize(1);
                oled->setCursor(85, 15);
                oled->println("QNH");
            }
            if (baroSelect == 0) {
                oled->setFont(&DSEG7Classic_Regular20pt7b);
                oled->setCursor(0, 60);
                oled->println(valueHg);
                oled->fillCircle(64, 60, 2, SSD1306_WHITE);
            } else {
                oled->setFont(&DSEG7Classic_Regular20pt7b);
                oled->setCursor(0, 60);
                oled->println(valueHpa);
            }
        }
    }
    oled->display();

} // updateDisplayEfis
void FCU_EFIS::updateDisplayEfisLeft(void)
{
    updateDisplayEfis(TCA9548A_CHANNEL_EFIS_LEFT, efisLeftBaroMode, efisLeftBaroSelect,
                      efisLeftBaroValueHpa, efisLeftBaroValueHg);
}

void FCU_EFIS::updateDisplayEfisRight(void)
{
    updateDisplayEfis(TCA9548A_CHANNEL_EFIS_RIGHT, efisRightBaroMode, efisRightBaroSelect,
                      efisRightBaroValueHpa, efisRightBaroValueHg);
}

void FCU_EFIS::updateDisplayFcuSpd(void)
{

    setTCAChannel(TCA9548A_CHANNEL_FCU_SPD);

    // Clear the buffer
    oled->clearDisplay();
    oled->setTextColor(SSD1306_WHITE);
    oled->setFont(&FreeSans8pt7b);
    oled->setTextSize(1);
    if (lightTestOn == 1) {
        oled->setCursor(25, 20);
        oled->println("SPD");
        oled->setCursor(65, 20);
        oled->println("MACH");
        oled->setFont(&DSEG7Classic_Regular15pt7b);
        oled->setCursor(28, 55);
        oled->println("888");
        oled->fillCircle(104, 40, 3, SSD1306_WHITE);
    } else {
        if (fcuSpeedMode == 1) {
            oled->setCursor(65, 20);
            oled->println("MACH");
        } else {
            oled->setCursor(25, 20);
            oled->println("SPD");
        }

        if (fcuSpeedManagedMode == 1) {
            oled->setFont(&DSEG7Classic_Regular15pt7b);
            oled->setCursor(28, 55);
            oled->println("---");
            oled->fillCircle(104, 40, 3, SSD1306_WHITE);
        } else {
            oled->setFont(&DSEG7Classic_Regular15pt7b);
            oled->setCursor(28, 55);
            oled->println(fcuSpeedValue);
        }
    }
    oled->display();
}

void FCU_EFIS::updateDisplayFcuHdg(void)
{
    char strHdgValue[4] = "000";

    // FCU Hdg
    setTCAChannel(TCA9548A_CHANNEL_FCU_HDG);

    // Clear the buffer
    oled->clearDisplay();
    oled->setTextColor(SSD1306_WHITE); // Draw white text

    oled->setFont(&FreeSans8pt7b);
    oled->setTextSize(1);

    if (lightTestOn == 1) {
        oled->setCursor(20, 20);
        oled->println("HDG");
        oled->setCursor(60, 20);
        oled->println("TRK");
        oled->setCursor(95, 20);
        oled->println("LAT");
        oled->setFont(&DSEG7Classic_Regular15pt7b);
        oled->setCursor(28, 55);
        oled->println("888");
        oled->fillCircle(104, 40, 3, SSD1306_WHITE);
    } else {
        if (fcuTrkMode == 0) {
            oled->setCursor(20, 20);
            oled->println("HDG");
        } else {
            oled->setCursor(60, 20);
            oled->println("TRK");
        }

        oled->setCursor(95, 20);
        oled->println("LAT");

        if (fcuHdgManagedMode == 1) {
            oled->setFont(&DSEG7Classic_Regular15pt7b);
            oled->setCursor(28, 55);
            oled->println("---");
            oled->fillCircle(104, 40, 3, SSD1306_WHITE);
        } else {
            oled->setFont(&DSEG7Classic_Regular15pt7b);
            oled->setCursor(28, 55);

            if (fcuHdgValue[1] == 0x00) {
                strHdgValue[0] = '0';
                strHdgValue[1] = '0';
                strHdgValue[2] = fcuHdgValue[0];
            } else {
                if (fcuHdgValue[2] == 0x00) {
                    strHdgValue[0] = '0';
                    strHdgValue[1] = fcuHdgValue[0];
                    strHdgValue[2] = fcuHdgValue[1];
                } else {
                    strHdgValue[0] = fcuHdgValue[0];
                    strHdgValue[1] = fcuHdgValue[1];
                    strHdgValue[2] = fcuHdgValue[2];
                }
            }
            oled->println(strHdgValue);
        }
    }
    oled->display();
}

void FCU_EFIS::updateDisplayFcuFpa(void)
{
    setTCAChannel(TCA9548A_CHANNEL_FCU_FPA);

    // Clear the buffer
    oled->clearDisplay();
    oled->setTextColor(SSD1306_WHITE);
    oled->setFont(&FreeSans8pt7b);
    oled->setTextSize(1);
    if (lightTestOn == 1) {
        oled->setCursor(30, 30);
        oled->println("HDG");

        oled->setCursor(74, 30);
        oled->println("V/S");
        oled->setCursor(30, 45);
        oled->println("TRK");

        oled->setCursor(74, 45);
        oled->println("FPA");
    } else {
        if (fcuTrkMode == 0) {
            oled->setCursor(30, 30);
            oled->println("HDG");

            oled->setCursor(74, 30);
            oled->println("V/S");
        } else {
            oled->setCursor(30, 45);
            oled->println("TRK");

            oled->setCursor(74, 45);
            oled->println("FPA");
        }
    }
    oled->display();
}

void FCU_EFIS::updateDisplayFcuAlt(void)
{

    char strAltValue[6] = "00000";

    setTCAChannel(TCA9548A_CHANNEL_FCU_ALT);
    // Clear the buffer
    oled->clearDisplay();
    oled->setTextColor(SSD1306_WHITE);
    oled->setFont(&FreeSans8pt7b);
    oled->setTextSize(1);
    oled->setCursor(52, 20);
    oled->print("ALT");

    oled->setCursor(94, 20);
    oled->print("LVL");
    oled->print("/");

    oled->drawFastVLine(82, 15, 5, SSD1306_WHITE);
    oled->drawFastHLine(82, 15, 10, SSD1306_WHITE);

    if (lightTestOn == 1) {
        oled->setFont(&DSEG7Classic_Regular15pt7b);
        oled->setCursor(0, 55);
        oled->print("88888");
        oled->fillCircle(124, 39, 3, SSD1306_WHITE);
    } else {

        if (fcuAltValue[3] == 0x00) {
            strAltValue[0] = '0';
            strAltValue[1] = '0';
            strAltValue[2] = fcuAltValue[0];
            strAltValue[3] = fcuAltValue[1];
            strAltValue[4] = fcuAltValue[2];
        } else {
            if (fcuAltValue[4] == 0x00) {
                strAltValue[0] = '0';
                strAltValue[1] = fcuAltValue[0];
                strAltValue[2] = fcuAltValue[1];
                strAltValue[3] = fcuAltValue[2];
                strAltValue[4] = fcuAltValue[3];
            } else {
                strAltValue[0] = fcuAltValue[0];
                strAltValue[1] = fcuAltValue[1];
                strAltValue[2] = fcuAltValue[2];
                strAltValue[3] = fcuAltValue[3];
                strAltValue[4] = fcuAltValue[4];
            }
        }

        oled->setFont(&DSEG7Classic_Regular15pt7b);
        oled->setCursor(0, 55);
        oled->print(strAltValue);

        if (fcuAltManagedMode == 1) {
            oled->fillCircle(124, 39, 3, SSD1306_WHITE);
        }
    }
    oled->display();

} // updateDisplayFcuAlt

void FCU_EFIS::updateDisplayFcuVs(void)
{
    char strVrValue[12] = "0000";

    setTCAChannel(TCA9548A_CHANNEL_FCU_VS);

    // Clear the buffer
    oled->clearDisplay();
    oled->setTextColor(SSD1306_WHITE);
    oled->setFont(&FreeSans8pt7b);
    oled->setTextSize(1);
    if (lightTestOn == 1) {
        oled->setCursor(40, 20);
        oled->print("V/S");
        oled->setCursor(86, 20);
        oled->print("FPA");
        oled->drawFastHLine(26, 15, 10, SSD1306_WHITE);
        oled->drawFastVLine(36, 15, 5, SSD1306_WHITE);
        oled->setFont(&FreeSans18pt7b);
        oled->setCursor(0, 50);
        oled->print("+");

        strVrValue[0] = '8';
        strVrValue[1] = '8';
        strVrValue[2] = '8';
        strVrValue[3] = '8';

        oled->setFont(&DSEG7Classic_Regular15pt7b);
        oled->setCursor(24, 55);
        oled->print(strVrValue);
    } else {
        oled->setCursor(2, 20);
        oled->print("CH");

        if (fcuTrkMode == 0) {
            oled->setCursor(40, 20);
            oled->print("V/S");
        } else {
            oled->setCursor(86, 20);
            oled->print("FPA");
        }

        oled->drawFastHLine(26, 15, 10, SSD1306_WHITE);
        oled->drawFastVLine(36, 15, 5, SSD1306_WHITE);

        if (fcuVsManagedMode == 1) {
            oled->setFont(&DSEG7Classic_Regular15pt7b);
            oled->setCursor(0, 55);
            oled->print("-----");
        } else {
            if (fcuTrkMode == 0) {
                if (fcuVsValue[1] == 0x00) {
                    oled->setFont(&FreeSans18pt7b);
                    oled->setCursor(0, 50);
                    oled->print("+");

                    strVrValue[0] = '0';
                    strVrValue[1] = '0';
                    strVrValue[2] = '0';
                    strVrValue[3] = '0';

                    oled->setFont(&DSEG7Classic_Regular15pt7b);
                    oled->setCursor(24, 55);
                    oled->print(strVrValue);
                } else {
                    if (fcuVsValue[0] == '-') {
                        oled->setFont(&DSEG7Classic_Regular15pt7b);
                        oled->setCursor(0, 55);
                        oled->print("-");

                        if (fcuVsValue[4] == 0x00) {
                            strVrValue[0] = '0';
                            strVrValue[1] = fcuVsValue[1];
                            strVrValue[2] = fcuVsValue[2];
                            strVrValue[3] = fcuVsValue[3];
                        } else {
                            strVrValue[0] = fcuVsValue[1];
                            strVrValue[1] = fcuVsValue[2];
                            strVrValue[2] = fcuVsValue[3];
                            strVrValue[3] = fcuVsValue[4];
                        }
                        oled->print(strVrValue);
                    } else {
                        oled->setFont(&FreeSans18pt7b);
                        oled->setCursor(0, 50);
                        oled->print("+");

                        if (fcuVsValue[3] == 0x00) {
                            strVrValue[0] = '0';
                            strVrValue[1] = fcuVsValue[0];
                            strVrValue[2] = fcuVsValue[1];
                            strVrValue[3] = fcuVsValue[2];
                        } else {
                            strVrValue[0] = fcuVsValue[0];
                            strVrValue[1] = fcuVsValue[1];
                            strVrValue[2] = fcuVsValue[2];
                            strVrValue[3] = fcuVsValue[3];
                        }

                        oled->setFont(&DSEG7Classic_Regular15pt7b);
                        oled->setCursor(24, 55);
                        oled->print(strVrValue);
                    }
                }
            } else {
                if (fcuVsValueFpa[0] == '-') {
                    oled->setFont(&DSEG7Classic_Regular15pt7b);
                    oled->setCursor(0, 55);

                    strcpy(strVrValue, fcuVsValueFpa);
                    if (fcuVsValueFpa[2] == 0x00) {
                        strcat(strVrValue, ".0");
                    }
                    oled->print(strVrValue);
                } else {
                    oled->setFont(&FreeSans18pt7b);
                    oled->setCursor(0, 50);
                    oled->print("+");

                    if (strcmp(fcuVsValueFpa, "0") == 0) {
                        strcpy(strVrValue, "0.0");
                    } else {
                        if (fcuVsValueFpa[1] == 0x00) {
                            strcpy(strVrValue, fcuVsValueFpa);
                            strcat(strVrValue, ".0");
                        } else {
                            strVrValue[0] = fcuVsValueFpa[0];
                            strVrValue[1] = fcuVsValueFpa[1];
                            strVrValue[2] = fcuVsValueFpa[2];
                            strVrValue[3] = 0x00;
                        }
                    }

                    oled->setFont(&DSEG7Classic_Regular15pt7b);
                    oled->setCursor(24, 55);
                    oled->print(strVrValue);
                }
            }
        }
    }
    oled->display();
}
