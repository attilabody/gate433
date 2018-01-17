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
Sheet 2 4
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L 24LC512 U2
U 1 1 5A5F322C
P 5800 3850
F 0 "U2" H 5550 4100 50  0000 C CNN
F 1 "24LC512" H 5850 4100 50  0000 L CNN
F 2 "Housings_DIP:DIP-8_W7.62mm_LongPads" H 5850 3600 50  0001 L CNN
F 3 "" H 5800 3750 50  0001 C CNN
	1    5800 3850
	1    0    0    -1  
$EndComp
Text HLabel 6600 3850 2    60   Output ~ 0
SCL
Text HLabel 6600 3750 2    60   BiDi ~ 0
SDA
Wire Wire Line
	6200 3750 6600 3750
Wire Wire Line
	6200 3850 6600 3850
$Comp
L +5V #PWR01
U 1 1 5A5F3296
P 5800 3300
F 0 "#PWR01" H 5800 3150 50  0001 C CNN
F 1 "+5V" H 5800 3440 50  0000 C CNN
F 2 "" H 5800 3300 50  0001 C CNN
F 3 "" H 5800 3300 50  0001 C CNN
	1    5800 3300
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR02
U 1 1 5A5F32AC
P 5800 4500
F 0 "#PWR02" H 5800 4250 50  0001 C CNN
F 1 "GND" H 5800 4350 50  0000 C CNN
F 2 "" H 5800 4500 50  0001 C CNN
F 3 "" H 5800 4500 50  0001 C CNN
	1    5800 4500
	1    0    0    -1  
$EndComp
Wire Wire Line
	5800 3550 5800 3300
Wire Wire Line
	5800 4150 5800 4500
Wire Wire Line
	5400 3750 5300 3750
Wire Wire Line
	5300 3750 5300 4250
Wire Wire Line
	5300 4250 6200 4250
Connection ~ 5800 4250
Wire Wire Line
	5400 3850 5300 3850
Connection ~ 5300 3850
Wire Wire Line
	5400 3950 5300 3950
Connection ~ 5300 3950
Wire Wire Line
	6200 4250 6200 3950
$EndSCHEMATC
