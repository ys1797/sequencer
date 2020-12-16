# sequencer with CW

##Sequencer with CW keyer and USB intervace

###Features:

* Dual COM port via USB interface.
* CW speed adjustable from 1 to 45 WPM (can be changed in code).
* Up to 4 TX/RX Sequencer keying lines with configurable lead, tail, and hang times.
* Sequencer has one optotransistor output, one small relay, one "latch relay" and "BIG" relay output.
* On tx state this board provide bias voltage for external needs.
* 3 sequencer line led indicator.
* CW output line has optotransistor output.
* CW output can be reprogrammed as 5 sequencer output.
* PTT line have protected circuits.
* Tx led indicator.
* Programming and interfacing via USB port (Console).
* Logging and Contest Program Interfacing via K1EL Winkey version 1 or 2 interface emulation. (Via second COM port).
* Speed potentiometer.
* Iambic A and B.
* Straight key support.
* Ultimatic mode.
* Bug mode.
* CMOS Super Keyer Iambic B Timing.
* Paddle reverse.
* Keying Compensation.
* Dash to Dot Ratio adjustment.
* Weighting.
* Autospace.
* Wordspace Adjustment.
* Flash non-volatile storage of most settings.
* Non-English (Russian) Character Support.

![Image alt](images/seq.jpg "Sequencer assembled board")

Schematic and PCB board can be found on easyeda project:

	https://easyeda.com/ys1797/pa_sequencer

 

The board schematic and part of PCB design based on the development of the following authors:

	"Sequencer with CW memory keyer 2018 UR3IQO:
	https://vhfdesign.com/ru/other/cw-key-sequencer.html"

	"RA3KBO 4 TX-RX unit PTT board:
	https://eb104.ru/internet-magazin/vhf-power-amplifier/vhf-tx-rx-unit-ptt-board"


This code based on previous work by the following authors:
	"K3NG Arduino CW Keyer Copyright 2010 - 2020 Anthony Good, K3NG"
	"morse generator module COPYRIGHT(c) 2016 S54MTB"
	"Examples from Copyright (c) 2020 STMicroelectronics."


    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License 
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
