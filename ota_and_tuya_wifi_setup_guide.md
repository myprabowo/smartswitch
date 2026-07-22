# OTA Update & Tuya Cloud Setup Walkthrough

This step-by-step walkthrough guides you through configuring the Tuya Developer Console, provisioning WiFi, and performing Over-The-Air (OTA) firmware updates.

---

## Part 1: Tuya Developer Console Step-by-Step Setup

1. **Create Account**: Go to [Tuya IoT Platform](https://iot.tuya.com/) and register a free developer account.
2. **Create Cloud Project**:
   - Navigate to **Cloud -> Development**.
   - Click **Create Cloud Project**.
   - Project Name: `Pump Remote Controller`
   - Industry: `Smart Home`
   - Data Center: Select your local data center (e.g., *Western America Data Center*).
3. **Authorize API Services**:
   - Under **API Services**, ensure **IoT Core**, **Authorization Token Management**, and **Device Control** are subscribed.
4. **Link Smart Life / Tuya App Account**:
   - Go to **Cloud -> Development -> Project -> Devices -> Link Tuya App Account**.
   - Click **Add App Account** and scan the QR code using your **Smart Life** app on your smartphone.
   - All switches paired to your Smart Life app will appear under the project devices list.
5. **Copy Credentials**:
   - Navigate to project **Overview** tab.
   - Copy **Access ID / Client ID**.
   - Copy **Access Secret / Client Secret**.
   - Click **Devices** tab and copy your smart switch's **Device ID**.

---

## Part 2: WiFi Provisioning Walkthrough

1. Apply 5V power to the ESP32-C3 Super Mini via USB-C.
2. On your smartphone, open WiFi settings and connect to **`PumpRemoteSetup`**.
3. A captive portal page titled **`PumpRemoteSetup`** will open automatically.
4. Tap **Configure WiFi**.
5. Select your home 2.4GHz WiFi network, enter the password, and tap **Save**.
6. The ESP32-C3 Super Mini will connect to your router and display its assigned IP address.

---

## Part 3: Over-The-Air (OTA) Firmware Flashing

The firmware includes built-in `ArduinoOTA` support for wireless updates over WiFi without requiring a USB cable.

### Flashing OTA from Arduino IDE
1. Ensure your computer is connected to the same WiFi network as the ESP32-C3 Super Mini.
2. Open `main.ino` in Arduino IDE 2.x.
3. Under **Tools -> Port**, select the network port: `PumpRemote at 192.168.x.x`.
4. Click **Upload**.
5. When prompted for password, enter default OTA password: `admin123` (or custom password set in `config.h`).
6. The onboard LED will rapid blink during transfer, and the board will automatically reboot with new code upon completion.
