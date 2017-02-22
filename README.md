elc-tools
=========

Perform memory dump of a ELC-3 traffic controller over RS232

LICENCE

	GPLv2

SYNOPSIS

	parcopy [r|w|t] [port] [baud]

DESCRIPTION


	* only the 'r' function is supported yet. Used to perform memory dumps

	* saves memory dump in 'dump.hex'

	* don't forget to verify checksum! This can be done using the included 'checksum.pl'

CAVEATS

	* Doesn't have a built-in checksum control yet

	* Seems to work with USB-based com-ports, but not with PC-card based ones

HISTORY

	First version appered in July 2006

AUTHOR

	David Otterdahl <david.otterdahl@gmail.com>


Additional comments about PARCOPY
-------------------------

Communication with ELC-2/3: Usually 1200 7-E-1 without hardware flow control

Establising communication
	- Sends ascii ESC (27) & "s" (115) to establish communication
	- If no answer, sends ascii DEL (127) 3 times with 5 seconds interval
	  Trying this in DosBox prints to following error:
Serial port at 3f8: Write to reserved register, value 0x0, register 2
	- If still no answer, prints message nr 02, 03, 04 
	  on each line e.g. No controller found, osv..

Terminating communication
	- Sends 'æ'(230) to terminate (även '©'(169) has been seen)
	2006-04-26: update: DEL (127), "y" 121, DEL 127 or 127, "?" 63, 127

	Sends commands LOCK, LINEFEED, BAUD0

Initiated communications
	parcopy defaults to 9660 baud. This baud rate can't be selected on the
        command line. parcopy saves using the same settings as during
        transmission

	- if answer, parcopy sends BAUD

	- sends ESC (27) och expects answer
	- sends command "PARDUMPOUT" or "PARDUMPIN"

Closing communication
	- when finished, sends ESC (27)
	  - LOCK
	  - LINEFEED
	  - BAUD 0

Additional comments about PARASCII
-------------

Similar to parcopy
- ESC (27)
- CRT40
- STAT
- BAUD4

Command reference
------------

BAUDx: Change baud rate
BAUD0: 1200 baud
BAUD1: 2400 baud
BAUD2: 300 baud
BAUD3: 600 baud
BAUD4: 4800 baud
BAUD5: 9600 baud
LOCK: Locks all lock levels
LINEFEED: Send a linefeed '\n' after each line
CRTxx/TTYxx: Selects between printer type terminal and crt type terminal.
             The line length is given as parameter.
             eg. a 80 char line: CRT80
             With TTY, status display commands xxINT (e.g. GRINT) displays
             line/second
STAT: Prints status info about the intersection, etc.

ELC-2 Configuration - HOWTO
-----------------------------

Fix 'Level locked' error (tested with ELC-3)
LA
0
LB
0

BP21=1
Disable flashing error in 7 segments

BP25=3
Changes what's displayed in the 7 segment (cycle)

BP40=[plan]
force plan

How to prevent that all signal groups gets DIR=1 during simulation
-------------------------------------------------------

BP19=1 (timer mode)
BP84=0 (ed channels)
BP85=0 (elc-mode)


How to get push buttons to work (seems to work anyway on ELC-3?)
----------------------------------------------------------------
Detektor parameters
DP3 Detektor input..
D1P3=2-1-1 ... changed to D1P3=2-1-89

Errors during transmission
==============
Error in: GIP, NCP, WCP, PCP
"Syntax error" "Only in configuration", "Wrong type"
Fix GIP and NCP by hand.
Bugs in ELC-TOOL? Wrong version? (Card 2.47)

Configuration
================
16 char name> FAGELBACKSG.
CDate> 93-9-30 (BDP1)
Conftime> 12-0 (BDP2)
Groups?> 24 (BDP3)
Plans?> 10 (BD4)
Logics?> 4 (BD5)
Sequences?> 4 (BD6)
Privilege time (GNP5) used?(Y/N)> Y (BDP26)
Detectors?>  40 (BDP8)
Predetectors> 8 (BDP7)
Contrl blocks> 30 (BD9)
integreen in groups 1 > 16 (G1BP13)
instructions in control block 1> 50 (C1DP1)
                              2> 30 (C2DP1)
                              18> 20 (C18DP1)
                              21> 10 (C21DP1)
cmd rows in plan nr 1> 32 (N1DP24)
                    7> 10 (N7DP24)
switching times in table1> 16 (W1DP1)
          .
switching times in table4> 8 (W8DP1)
          .
special days in year1> 10 (Y1DP1)
          .
plan change algorithm used (N)> 1 (BDP12)
psu plans?> 6 (BDP17)
Ch1 ranges> 3 (BDP18)
Ch2 ranges> 3 (BDP19)
sets of m points> 2 (BDP21)
traf situations> 6 (BDP20)
controllers? > 20 (BDP22)
emergency routes in use> 1 (BDP23)
No of em routes?> 5 (BDP24)
Controllers in this emergency route? >1> 5 (PBDP1)
           .
ED Channels> 30 (BDP25)

OPTIONS

[r|w|t] read, write, test
only read is supported yet

[baud]
Optional, force usage of specific baud rate

