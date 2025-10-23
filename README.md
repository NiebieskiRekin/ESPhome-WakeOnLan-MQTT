# Wake On Lan MQTT

This project aims to allow booting other systems using Wake on LAN coming from a tiny embedded ESP32-C3 board connected to a MQTT broker hosted in the cloud. The primary goal of this project was to limit the power draw of my homelab, while still allowing remote access to home resources.

![Architecture Diagram](diagram.svg)

This repository contains my setup using an excellent embedded development framework - ESPhome and some other configuration details that I didn't want to be lost.

## Environment setup

1. Check if Wake On Lan is supported. Enable it in the BIOS on target PC. You can test it with the provided C++ code in `wol.cpp` (tested on Linux) or other tools.
   - `g++ wol.cpp -o wol`
   - `wol 00:AA:BB:11:22:33`
2. Set up an MQTT broker, the processing requirements are very small for this project.
3. Test the TLS connectivity using provided Python code in `mqtt-test`.
   - `uv sync`
   - Fill missing values in `main.py`
   - `uv run python main.py`
4. Create a new file `secrets.yaml` and fill it with correct values for your setup using the provided template `secrets.example.yaml`
5. Download the TLS certificate chain of your MQTT broker and copy the root CA certificate in PEM format to paste into the `mqtt_certificate_authority` key in `secrets.yaml`. Ready to use command:
    ```bash
    broker="$(yq -r .mqtt_broker secrets.yaml)"
    port="$(yq -r .mqtt_port secrets.yaml)"
    # Show fullchain cert
    openssl s_client -showcerts \
      -servername "$broker" \
      -connect "$broker:$port" < /dev/null 2>/dev/null |
      awk '/^---$/ { n++ } n==1, n==2' | sed '/^---$/d'
    ```
    It's sufficient to only copy the last part e.g.
    ```
      1 s:C=US, O=Let's Encrypt, CN=R12
     ...
     -----BEGIN CERTIFICATE-----
     ...
     -----END CERTIFICATE-----
    ```

    The `mqtt_certificate_authority` key should be updated when the broker certificate's root CA changes. This happens every few years, but you are probably in the middle of the cycle.
    
    You can optionally add multiple (future) root CA certificates to smooth the transition. Here's some more information from Let's Encrypt <https://letsencrypt.org/certificates/> 

    Alternatively set `skip_cert_cn_check: true` in `wakeonlanmqtt.yaml` to skip skip host verification entirely (insecure).
6. The main file is `wakeonlanmqtt.yaml` which is the base firmware for the ESP, written in [ESPhome](https://esphome.io/guides/getting_started_command_line.html). It can be compiled and uploaded using
   ```bash
   docker run --rm --privileged -v "${PWD}":/config --device=/dev/ttyUSB0 -it ghcr.io/esphome/esphome run wakeonlanmqtt.yaml
   ```
7. Download [DroidPad](https://github.com/umer0586/DroidPad) and import the template (also add connection details) or use any other MQTT client software to send boot messages from your device. It's recommended to set QOS=2 (deliver exactly once) if your broker supports it.

## References

- [ESPhome docs](https://esphome.io/components/mqtt.html)
- MQTT:
  - Self-hosted - [Mosquitto](https://hub.docker.com/_/eclipse-mosquitto)
  - Managed Cloud offerring with totally sufficient free tier [HiveMQ](https://www.hivemq.com/products/mqtt-cloud-broker/)
- ESP32 board I chose (avoid if possible, but it does work)
  - <https://pl.aliexpress.com/item/1005007446928015.html>
  - <https://sigmdel.ca/michel/ha/esp8266/super_mini_esp32c3_en.html>
  - <https://github.com/sigmdel/supermini_esp32c3_sketches>
  - <https://github.com/sidharthmohannair/Tutorial-ESP32-C3-Super-Mini>
  - <https://dl.artronshop.co.th/ESP32-C3%20SuperMini%20datasheet.pdf>
- Wireguard on ESP32 (unused in this project):
  - <https://github.com/pirate/wireguard-docs>
  - <https://github.com/ciniml/WireGuard-ESP32-Arduino>
  - <https://github.com/trombik/esp_wireguard>

## Common issues

### TLS verification issues

If you copy all of the certificates in the chain then the ESP will not be able to connect to the broker in the future when any of the certificates changes. They are usually changed before the expiry date. Here is a sample log:
```
esp-tls-mbedtls: mbedtls_ssl_handshake returned -0x2700
esp-tls-mbedtls: Failed to verify peer certificate!
esp-tls: Failed to open new connection
transport_base: Failed to open a new connection
mqtt_client: Error transport connect
MQTT_EVENT_ERROR
Last error code reported from esp-tls: 0x801a
Last tls stack error number: 0x2700
Last captured errno : 0 (Success)
```

### WiFi Connection issues

In case the microcontroller cannot connect to the specified WiFi network then it will start its own one with the password you provided in the `secrets.yaml` in `wifi_ap_password`. It's usually fine to just reupload the binary with corrected credentials to resolve the issue.

In case the WiFi signal is too weak you can try to increase the power supplied. Add this build flag to your `wakeonlanmqtt.yaml`
```yaml
esphome:
  name: pcbooter
  platformio_options:
    build_flags:
      - -DTX_POWER=WIFI_POWER_11dBm
```
How to specify build flags to ESPHome yaml: <https://github.com/crankyoldgit/IRremoteESP8266/discussions/1611>

## Other ideas

Boot over VPN - allow booting a desktop PC using Wake on LAN coming from a tiny embedded ESP32-C3 board connected to a Wireguard VPN. Scrapped due to connectivity problems on my esp
