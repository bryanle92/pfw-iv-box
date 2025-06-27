SSD1309 OLED Display Project

This project provides a complete solution for displaying content on a 128x64 SSD1309 OLED screen over I²C. It includes:

A Linux kernel framebuffer driver using regmap

Python tools for generating and sending content

Sample assets (text and images)

📁 Project Structure

.
├── assets/                 # Sample input/output files
│   ├── image.png           # Original image input
│   ├── output.pbm          # Converted PBM for SSD1309
│   ├── preview.png         # Render preview of text
│   └── text.txt            # Sample text input
│
├── driver/                 # Linux kernel framebuffer driver
│   ├── Makefile            # Build system
│   └── ssd1309fb.c         # Minimal regmap-based SSD1309 driver
│
└── tools/                  # User-space tools
    ├── oled_full_write_pbm.py  # Send PBM image to SSD1309 using i2ctransfer
    └── oled_text.py            # Render text (UTF-8 supported) to raw framebuffer format

🧱 1. Kernel Driver (driver/)

This driver exposes the SSD1309 as a framebuffer device /dev/fb0.

🔧 Build and Load

cd driver
make
sudo modprobe regmap-i2c
sudo insmod ssd1309fb.ko
echo ssd1309fb 0x3c | sudo tee /sys/bus/i2c/devices/i2c-1/new_device

🖼 Test framebuffer

sudo dd if=../assets/ssd1309_text.raw of=/dev/fb0 bs=1024 count=1

🛠️ 2. Tools (tools/)

oled_text.py

Render multilingual text to a 1-bit framebuffer buffer (GDDRAM-compatible .raw).

python3 tools/oled_text.py assets/text.txt

Output: ssd1309_text.raw (in current folder)

Preview: preview.png

oled_full_write_pbm.py

Send a 128×64 PBM image (P4 format) directly to the OLED via I²C.

python3 tools/oled_full_write_pbm.py assets/output.pbm

Requires: i2c-tools, Pillow, and fonts-dejavu

🧪 3. Assets (assets/)

Sample inputs:

image.png → input for PBM conversion

output.pbm → binary PBM image

text.txt → test text

preview.png → rendered text for visual check

To generate PBM:

convert assets/image.png -resize 128x64\! -monochrome pbm:assets/output.pbm

⚙️ Dependencies

Python 3

Pillow (pip install pillow)

ImageMagick (convert command)

fonts-dejavu (sudo apt install fonts-dejavu)

i2c-tools (sudo apt install i2c-tools)

📝 Notes

OLED should be at I²C address 0x3c

Sysfs interface:

echo 0 | sudo tee /sys/devices/platform/soc/3f804000.i2c/i2c-1/1-003c/display_on  # turn off
echo 1 | sudo tee /sys/devices/platform/soc/3f804000.i2c/i2c-1/1-003c/display_on  # turn on

👤 Author

This project was developed for direct I²C control of SSD1309 OLEDs, combining low-level kernel access and flexible user-space rendering.

Feel free to adapt it for embedded systems, dashboards, or hobby electronics.

