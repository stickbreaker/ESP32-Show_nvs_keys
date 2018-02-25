# ESP32-Show_nvs_keys
* initial release 04DEC2017 by Ed Smallenburg @Edzelf
* modified 24FEB2018 for additional Error reporting, Data Display by @stickbreaker

Show all keys in ESP32 non volatile storage (NVS).

The NVS library is missing a function to list all keys (or a subset) present in the nvs.
For some programs however, it is vital to know which keys are present.
This demo shows all the keys in the NVS.
