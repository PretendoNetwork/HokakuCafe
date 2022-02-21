# HokakuCafe
Applies IOSU patches to write all `PRUDP` network frames to a `.pcap` file. Also dumps NEX account tokens, which are needed for packet decryption.  
All files will be in the `HokakuCafe` folder on the root of your SD Card.  

## Usage
### Homebrew Launcher
1. Copy the `HokakuCafe.rpx` to the `wiiu/apps` folder
2. Open the HBL and launch HokakuCafe
3. Launch a game and the packets will be written to the SD

### Tiramisu
1. Copy the `30_hokaku_cafe.rpx` to the `wiiu/environments/tiramisu/modules/setup` folder
2. Launch a game and the packets will be written to the SD

Make sure to only remove the SD Card after powering off the system, to make sure all data has been written.

## Building
Install devkitPPC, devkitARM and wut.  
Run `make` (add `TIRAMISU=1` to build the tiramisu version).
