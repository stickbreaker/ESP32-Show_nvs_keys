# ESP32-Show_nvs_keys
* initial release 04DEC2017 by Ed Smallenburg @Edzelf
* modified 24FEB2018 for additional Error reporting, Data Display by @stickbreaker

Show all keys in ESP32 non volatile storage (NVS).

The NVS library is missing a function to list all keys (or a subset) present in the nvs.
For some programs however, it is vital to know which keys are present.
This demo shows all the keys in the NVS.

## Simple sketch to reinit nvs provided byte @lbernstone
```c++
#include <nvs.h>
#include <nvs_flash.h>
void setup() {
    int err;
    err=nvs_flash_init();
    Serial.println("nvs_flash_init: " + err);
    err=nvs_flash_erase();
    Serial.println("nvs_flash_erase: " + err);
 }
```

# example of Normal Output
```

Partition nvs found, 20480 bytes
page[0].State=0xfffffffc INIT FULL  means FULL  Seqnr=0 
page[1].State=0xfffffffc INIT FULL  means FULL  Seqnr=1 
page[2].State=0xfffffffe INIT  means ACTIVE  Seqnr=2 
page[3].State=0xffffffff  means UNINITIALIZED  Page[3] not active, State = 0xffffffff Seqnr=4294967295 
page[4].State=0xffffffff  means UNINITIALIZED  Page[4] not active, State = 0xffffffff Seqnr=4294967295 

Logical Page to Physical Sector
Page 0 -> 0
Page 1 -> 1
Page 2 -> 2
Page 3 -> 255
Page 4 -> 255

NAME SPACES
           misc = 1
   nvs.net80211 = 2
            phy = 3

NameSpace:            misc = 1

NameSpace:    nvs.net80211 = 2

NameSpace:             phy = 3

NameSpace:            misc 
0-001:            log type:BLOB data=size:0010
02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................

NameSpace:    nvs.net80211 
0-010:   sta.authmode type:U8   data=1
0-015:        sta.pmk type:BLOB data=size:0020
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff  ................
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff  ................
0-018:      auto.conn type:U8   data=1
0-020:      sta.bssid type:BLOB data=size:0006
ff ff ff ff ff ff                                ......          
0-022:       sta.phym type:U8   data=3
0-023:      sta.phybw type:U8   data=2
0-053:        ap.ssid type:BLOB data=size:0024
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff  ................
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff  ................
ff ff ff ff                                      ....            
0-058:      ap.passwd type:BLOB data=size:0041
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff  ................
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff  ................
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff  ................
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff  ................
ff  
0-062:         ap.pmk type:BLOB data=size:0020
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff  ................
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff  ................
0-064:        ap.chan type:U8   data=1
0-065:    ap.authmode type:U8   data=0
0-066:      ap.hidden type:U8   data=0
0-067:    ap.max.conn type:U8   data=4
0-068:   bcn.interval type:U16  data=100
0-069:        ap.phym type:U8   data=3
0-070:       ap.phybw type:U8   data=2
0-071:     ap.sndchan type:U8   data=1
1-061:       sta.ssid type:BLOB data=size:0024
0b 00 00 00 42 65 6c 6b 69 6e 2e 34 36 34 43 00  ....Belkin.464C.
a5 a5 a5 a5 a5 a5 a5 a5 d0 13 08 40 3e 60 08 40  ...........@>`.@
30 05 06 00
```

# Example of Damaged NVS
```
// Show_nvs_keys Output, The ESP32 that generated this output has a damaged NVS partition.
// the page.Seqnr values should be inclusive of the partition size (0..4)
// With this damage, the report only accesses Logical Page Zero, (sector 0)

Partition nvs found, 20480 bytes
page[0].State=0xfffffffc INIT FULL  means FULL  Seqnr=0 
page[1].State=0xfffffffe INIT  means ACTIVE Sequence value InValid: page[1].Seqnr=5663 
page[2].State=0xffffffff  means UNINITIALIZED  Page[2] not active, State = 0xffffffff Seqnr=4294967295 
page[3].State=0xfffffffc INIT FULL  means FULL Sequence value InValid: page[3].Seqnr=183 
page[4].State=0xfffffffc INIT FULL  means FULL Sequence value InValid: page[4].Seqnr=5662 

Logical Page to Physical Sector
Page 0 -> 0
Page 1 -> 255
Page 2 -> 255
Page 3 -> 255
Page 4 -> 255

NAME SPACES
           misc = 1
   nvs.net80211 = 2
            phy = 3

NameSpace:            misc = 1

NameSpace:    nvs.net80211 = 2

NameSpace:             phy = 3

NameSpace:            misc 
0-001:            log type:BLOB data=size:0010
02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................

NameSpace:    nvs.net80211 
0-005:        country type:U8   data=0
0-011:   sta.authmode type:U8   data=1
0-016:        sta.pmk type:BLOB data=size:0020
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff  ................
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff  ................
0-019:      auto.conn type:U8   data=1
0-023:       sta.phym type:U8   data=3
0-024:      sta.phybw type:U8   data=2
0-050:        ap.ssid type:BLOB data=size:0024
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff  ................
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff  ................
ff ff ff ff                                      ....            
0-055:      ap.passwd type:BLOB data=size:0041
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff  ................
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff  ................
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff  ................
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff  ................
ff                                               .               
0-059:         ap.pmk type:BLOB data=size:0020
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff  ................
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff  ................
0-061:        ap.chan type:U8   data=1
0-062:    ap.authmode type:U8   data=0
0-063:      ap.hidden type:U8   data=0
0-064:    ap.max.conn type:U8   data=4
0-065:   bcn.interval type:U16  data=100
0-066:        ap.phym type:U8   data=3
0-067:       ap.phybw type:U8   data=2
0-068:     ap.sndchan type:U8   data=1

NameSpace:             phy 
```
