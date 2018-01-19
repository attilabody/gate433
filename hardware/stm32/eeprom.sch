EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:switches
LIBS:relays
LIBS:motors
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
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
LIBS:valves
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
Text HLabel 7250 3850 2    60   Output ~ 0
SCL
Text HLabel 7250 3750 2    60   BiDi ~ 0
SDA
$Comp
L +5V #PWR3
U 1 1 5A5F3296
P 5800 3450
F 0 "#PWR3" H 5800 3300 50  0001 C CNN
F 1 "+5V" H 5800 3590 50  0000 C CNN
F 2 "" H 5800 3450 50  0001 C CNN
F 3 "" H 5800 3450 50  0001 C CNN
	1    5800 3450
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR4
U 1 1 5A5F32AC
P 5800 4500
F 0 "#PWR4" H 5800 4250 50  0001 C CNN
F 1 "GND" H 5800 4350 50  0000 C CNN
F 2 "" H 5800 4500 50  0001 C CNN
F 3 "" H 5800 4500 50  0001 C CNN
	1    5800 4500
	1    0    0    -1  
$EndComp
Wire Wire Line
	5800 3550 5800 3450
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
$Comp
L Conn_01x06 J14
U 1 1 5A604F2C
P 6700 2950
F 0 "J14" H 6700 3250 50  0000 C CNN
F 1 "RTC" H 6700 2550 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x06_Pitch2.54mm" H 6700 2950 50  0001 C CNN
F 3 "" H 6700 2950 50  0001 C CNN
	1    6700 2950
	1    0    0    1   
$EndComp
$Comp
L GND #PWR6
U 1 1 5A604F8F
P 6450 3250
F 0 "#PWR6" H 6450 3000 50  0001 C CNN
F 1 "GND" H 6450 3100 50  0000 C CNN
F 2 "" H 6450 3250 50  0001 C CNN
F 3 "" H 6450 3250 50  0001 C CNN
	1    6450 3250
	1    0    0    -1  
$EndComp
$Comp
L +5V #PWR5
U 1 1 5A6050C2
P 6450 2550
F 0 "#PWR5" H 6450 2400 50  0001 C CNN
F 1 "+5V" H 6450 2690 50  0000 C CNN
F 2 "" H 6450 2550 50  0001 C CNN
F 3 "" H 6450 2550 50  0001 C CNN
	1    6450 2550
	1    0    0    -1  
$EndComp
Wire Wire Line
	6500 3150 6450 3150
Wire Wire Line
	6450 3150 6450 3250
Wire Wire Line
	6500 3050 6450 3050
Wire Wire Line
	6450 3050 6450 2550
Wire Wire Line
	6500 2950 6350 2950
Wire Wire Line
	6500 2850 6350 2850
Wire Wire Line
	6200 3750 6350 3750
Wire Wire Line
	6200 3850 6350 3850
Wire Wire Line
	7250 3750 7100 3750
Wire Wire Line
	7250 3850 7100 3850
Text Label 7100 3750 2    60   ~ 0
SDA
Text Label 7100 3850 2    60   ~ 0
SCL
Text Label 6350 3750 0    60   ~ 0
SDA
Text Label 6350 3850 0    60   ~ 0
SCL
Text Label 6350 2950 2    60   ~ 0
SDA
Text Label 6350 2850 2    60   ~ 0
SCL
$Comp
L C_Small C3
U 1 1 5A605271
P 5550 3000
F 0 "C3" H 5560 3070 50  0000 L CNN
F 1 "100n" H 5560 2920 50  0000 L CNN
F 2 "Capacitors_SMD:C_0805" H 5550 3000 50  0001 C CNN
F 3 "" H 5550 3000 50  0001 C CNN
	1    5550 3000
	1    0    0    -1  
$EndComp
$Comp
L +5V #PWR1
U 1 1 5A605277
P 5550 2800
F 0 "#PWR1" H 5550 2650 50  0001 C CNN
F 1 "+5V" H 5550 2940 50  0000 C CNN
F 2 "" H 5550 2800 50  0001 C CNN
F 3 "" H 5550 2800 50  0001 C CNN
	1    5550 2800
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR2
U 1 1 5A60527D
P 5550 3200
F 0 "#PWR2" H 5550 2950 50  0001 C CNN
F 1 "GND" H 5550 3050 50  0000 C CNN
F 2 "" H 5550 3200 50  0001 C CNN
F 3 "" H 5550 3200 50  0001 C CNN
	1    5550 3200
	1    0    0    -1  
$EndComp
Wire Wire Line
	5550 2800 5550 2900
Wire Wire Line
	5550 3100 5550 3200
$EndSCHEMATC
