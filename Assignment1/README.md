```
bash

yosys -p 'synth_ice40 -top multiLedBlink -json multiLedBlink.json' multiLedBlink.v

nextpnr-ice40 --hx8k --json multiLedBlink.json --pcf ico-jiwy.pcf --asc multiLe
dBlink.asc

icepack multiLedBlink.asc multiLedBlink.bin

scp /home/afeef_ahmed/Assignment1/ice40.bin raspberrypi@10.0.15.51:/home/raspberrypi/Blinky/

```

```
# Copy a single file
scp /path/to/local/file.txt pi@raspberry-pi-ip:/path/to/destination/

# Copy a directory recursively
scp -r /path/to/local/directory/ pi@raspberry-pi-ip:/path/to/destination/

```