# ESP32-C3-OLED with AHT20+BMP280
ESP32 C3 OLED with AHT20+BMP280 to read and output current temperature, humidity and barrometric pressure.

### Overview

This project will create a tiny box with  temperature,  humidity and atmospheric pressure readings. Atmospheric pressure will follow trends to determine (and display) if weather is getting worse or better (or is stable).

Chinese AliExpress ESP32-C3 OLED has non standard pinout. SDA/SCL are on pins 5/6 instead of 8/9
These same pins are used by t he 0.43 inch OLED display
<div align="center">
<img width="305" height="536" alt="image" src="https://github.com/user-attachments/assets/e799c86a-9a00-4573-95f9-d5127a382f48" />
</div>

### Wiring

Use 30AWG flexible wire, length ~1 inch. Solder the 4 pins from ATH20+BMP280 as follows.

| AHT20+BMP280 Pin | ESP32 Pin |
|------------|----------|
| VCC        | 3.3V     |
| GND        | GND      |
| SDA        | GPIO 5   |
| SCL        | GPIO 6   |

### Case

STL files provided:

Main Case - Bottom cavity is for the AHT20+BMP280 board. Print with Tree supports.
Divider - Is inserted to divide ESP32 from the AHT20+BMP280 board in order to reduce heat from ESP32 reaching the sensors.
Lid - Top lid (snaps into  place).

### Assembly and the Final Product

During assembly utilize the corner channels to feed the wires through (2 on each side). I found it easier by using a tool to push the wires to the side. You want flexible, thin (30awg) and short (sub 1 inch) wires for this.
<div align="center">

<img width="675" height="446" src="https://github.com/user-attachments/assets/5f6d13bd-5d03-49ef-aea7-5357739e525e" />
<br><br>
<img width="474" height="439" src="https://github.com/user-attachments/assets/cfa5c7fd-a472-4d3a-b67a-b529a029297e" />
<br><br>
<img width="661" height="574" src="https://github.com/user-attachments/assets/478165d2-517a-4573-a4c5-86afd61d7f75" />
<br><br>
<img width="645" height="536" src="https://github.com/user-attachments/assets/06f9e2ad-8d8c-4e09-a399-07af7370e55e" />
<br><br>
<img width="718" height="439" src="https://github.com/user-attachments/assets/11f34ae9-0393-4d33-9c9e-a5932072037c" />

</div>
