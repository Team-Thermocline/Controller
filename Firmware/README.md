# Team Thermocline Controller Firmware

This project is configured to build for the RP2040 with PicoDVI library support, following the [Elecrow CrowPanel tutorial](https://www.elecrow.com/wiki/CrowPanel_Pico_HMI_4.3-inch_Arduino_Tutorial.html).

## PlatformIO Configuration

This project is built with platformio, to get started. Install platformio and invoke:

```shell
pio run
```

Then 

```shell
pio run deploy
```
to flash the firmware using `picotool`

## References

- [Elecrow CrowPanel Tutorial](https://www.elecrow.com/wiki/CrowPanel_Pico_HMI_4.3-inch_Arduino_Tutorial.html)
- [PicoDVI Library](https://github.com/maxgerhardt/pico-dvi)
- [PlatformIO Documentation](https://docs.platformio.org/)
