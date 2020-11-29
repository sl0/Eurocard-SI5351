/*
 * mysi5351.ino - Simply configure Si5351 synthesizer for 25 + 40 MHz
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * This arduino sketch configures an i2c connected Si5351 Synthesizer chip
 * to have stable and exact base frequencies:
 *    25 MHz for a LNB directed to QO-100.
 *    40 MHz for an Adalm-Pluto to replace its internal XO
 *
 * These are prerequisites to have reliable and exact frequency numbers
 * within the Pluto and connected Software.
 *
 * For less distrubing noise the arduino is going to sleep mode after
 + having configured the synthesizers.
 *
 * The sketch is independent of the Si5351 onboard XO. To get exact results,
 * this needs to be replaced by a separate OXCO or a GPSDO. Nevertheless
 * the sketch works as well to produce less reliable signals with the
 * soldered oscillater on the breakout board.
 *
 * Si5351 Synthesizers are available from etherkit.com:
 *          https://www.etherkit.com/rf-modules/si5351a-breakout-board.html
 *   or from Adafruit Industries:
 *          https://learn.adafruit.com/adafruit-si5351-clock-generator-breakout
 *
 *
 * Thanks to Jason Milldrum <milldrum@gmail.com> for his library:
 *          https://github.com/etherkit/Si5351Arduino
 *
 * Thanks to Wolf DF7KB and Klaus DC6CM for their helpful hints.
 *
 * Have fun!
 * 73 de Johannes, DO1SLO
 */

#define DEBUG

#include <avr/sleep.h>
#include "si5351.h"
#include "Wire.h"

#define Led_Pin 13
#define Led_On          {digitalWrite(Led_Pin, HIGH);}
#define Led_Off         {digitalWrite(Led_Pin, LOW);}

#define DFLT_FREQ     25000000      // use onboard xtal
#define XTAL_FREQ     10000000      // external oscillator
//                                     needs desoldered xtal and wire

// decide here if you have desoldered xtal
#define BASIC_FREQ    XTAL_FREQ

Si5351 si5351;

/*  enter_sleep() was taken from
    http://www.netzmafia.de/skripten/hardware/Arduino/Sleep/index.html
    (c) by Hochschule Muenchen, FK04, Prof. Jürgen Plate
*/

#define SLEEP_MODE SLEEP_MODE_PWR_DOWN

void enter_sleep(void)
{
    set_sleep_mode(SLEEP_MODE);
    sleep_enable();
    sleep_mode();
    /** Das Programm läuft ab hier nach dem Aufwachen weiter. **/
    /** Es wird immer zuerst der Schlafmodus disabled.        **/
    sleep_disable();
}


void setup()
{
    bool i2c_found;

    pinMode(Led_Pin, OUTPUT);
    Led_On;

#ifdef DEBUG
    // Start serial and initialize the Si5351
    Serial.begin(115200);
    Serial.println("Arduino init starts");
#endif
    i2c_found = si5351.init(SI5351_CRYSTAL_LOAD_8PF, BASIC_FREQ, 0);
#ifdef DEBUG
    if(!i2c_found) {
      Serial.println("Device not found on I2C bus!");
    } else {
      Serial.println("Si5351 device found on I2C bus!");
      Serial.println("Initializing the synthesizer ...");
    }
#endif

    // CLK0 is not used now
    si5351.output_enable(SI5351_CLK0, 0);

    // CLK1 and CLK2 are used
    si5351.output_enable(SI5351_CLK1, 1);
    si5351.output_enable(SI5351_CLK2, 1);

    si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_4MA);
    si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_4MA);

    // Set CLK1 to output 25 MHz with PLL A ( x 32)
    si5351.set_ms_source(SI5351_CLK1, SI5351_PLLA);
    si5351.set_freq_manual(2500000000ULL, 80000000000ULL, SI5351_CLK1);

    // Set CLK2 to output 40 MHz with PLL B (x 16)
    si5351.set_ms_source(SI5351_CLK2, SI5351_PLLB);
    si5351.set_freq_manual(4000000000ULL, 64000000000ULL, SI5351_CLK2);

    // Query a status update and wait a bit to let the Si5351 populate the
    // status flags correctly.
    si5351.update_status();
    delay(1500);
#ifdef DEBUG
      Serial.println("Init ready, starting normal operation ...");
#endif
}

void loop()
{
    // Read the Status Register and print it once
    si5351.update_status();
#ifdef DEBUG
    Serial.println("Synthesizer status:");
    Serial.print("SYS_INIT: ");
    Serial.print(si5351.dev_status.SYS_INIT);
    Serial.print("  LOL_A: ");
    Serial.print(si5351.dev_status.LOL_A);
    Serial.print("  LOL_B: ");
    Serial.print(si5351.dev_status.LOL_B);
    Serial.print("  LOS: ");
    Serial.print(si5351.dev_status.LOS);
    Serial.print("  REVID: ");
    Serial.println(si5351.dev_status.REVID);
#endif
    delay(1500);
#ifdef DEBUG
    Serial.println("Entering sleep mode, operation continues ...");
    delay(20);
#endif
    Led_Off;
    // power down to sleep mode, don't do anything more!
    enter_sleep();
    // usually the following will never run
    delay(10000);
}
// EoF
