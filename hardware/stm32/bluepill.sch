EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:linear
LIBS:regul
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:bluepill
LIBS:Connector
LIBS:bluepill-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 4
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Sheet
S 3400 4250 850  550 
U 5A5F2F18
F0 "EEPROM" 60
F1 "eeprom.sch" 60
F2 "SCL" O R 4250 4550 60 
F3 "SDA" B R 4250 4400 60 
$EndSheet
$Sheet
S 4450 4250 700  800 
U 5A5F2F24
F0 "MCU" 60
F1 "mcu.sch" 60
F2 "SDA" B L 4450 4400 60 
F3 "SCL" O L 4450 4550 60 
F4 "IG" O R 5150 4850 60 
F5 "IY" O R 5150 4750 60 
F6 "IR" O R 5150 4650 60 
F7 "OG" O R 5150 4550 60 
F8 "OY" O R 5150 4450 60 
F9 "OR" O R 5150 4350 60 
$EndSheet
Wire Wire Line
	4250 4400 4450 4400
Wire Wire Line
	4250 4550 4450 4550
$Sheet
S 5350 4250 750  800 
U 5A5F710E
F0 "Outputs" 60
F1 "Outputs.sch" 60
$EndSheet
$EndSCHEMATC
