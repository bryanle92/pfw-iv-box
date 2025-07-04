#!/usr/bin/env python3
import sys
from PIL import Image
import subprocess

# --- Cấu hình ---
I2C_BUS = "1"
I2C_ADDR = "0x3c"

# --- Đọc tên file .pbm ---
if len(sys.argv) != 2:
    print("Usage: oled_send_pbm.py <128x64.pbm>")
    sys.exit(1)

pbm_file = sys.argv[1]

# --- Mở ảnh & xác minh ---
try:
    img = Image.open(pbm_file).convert("1")
except Exception as e:
    print(f"❌ Lỗi khi mở ảnh: {e}")
    sys.exit(1)

if img.size != (128, 64):
    print(f"❌ Ảnh phải đúng kích thước 128x64. Đang là: {img.size}")
    sys.exit(1)

# --- Load pixel và chuyển về GDDRAM format ---
pixels = img.load()
buffer = bytearray()

for page in range(8):  # mỗi page = 8 dòng
    for col in range(128):
        byte = 0
        for bit in range(8):
            y = page * 8 + bit
            pixel = pixels[col, y]
            if pixel:  # pixel sáng (trắng)
                byte |= (1 << bit)
        buffer.append(byte)

# --- Gửi từng page (128 byte) ---
for page in range(8):
    offset = page * 128
    page_cmd = 0xB0 + page  # set page address

    print(f"Page {page} → cmd: 0x{page_cmd:02X}, offset: {offset}")

    # Gửi command: set page + column address
    subprocess.run([
        "i2ctransfer", "-y", I2C_BUS,
        f"w5@{I2C_ADDR}",
        "0x00",  # command prefix
        f"0x{page_cmd:02X}",  # set page
        "0x21",  # set column start
        "0x00",  # col start
        "0x7F"   # col end
    ], check=True)

    # Gửi 128 byte data (bắt đầu bằng 0x40 prefix)
    chunk = buffer[offset:offset + 128]
    cmd = ["i2ctransfer", "-y", I2C_BUS, f"w129@{I2C_ADDR}", "0x40"]
    cmd += [f"0x{b:02X}" for b in chunk]
    subprocess.run(cmd, check=True)

print("✅ Gửi ảnh hoàn tất.")
