#!/bin/bash

sudo mount LABEL=TOASTIUM mount &&
make all BOARD=VF2L &&
sudo rm -f mount/boot/kernel.bin &&
sudo cp build/kernel.bin mount/boot/kernel.bin &&
sync &&
sleep 2 &&
sudo umount mount &&
sudo eject LABEL=TOASTIUM 