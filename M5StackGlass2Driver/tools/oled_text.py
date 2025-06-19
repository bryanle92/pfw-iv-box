from PIL import Image, ImageDraw, ImageFont
import sys

# --- Cấu hình ---
WIDTH, HEIGHT = 128, 64
FONT_PATH = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
FONT_SIZE = 12
OUTPUT_FILE = "ssd1309_text.raw"

# --- Đọc file văn bản ---
if len(sys.argv) != 2:
    print("Usage: python3 oled_text_from_file.py <textfile.txt>")
    sys.exit(1)

with open(sys.argv[1], "r", encoding="utf-8") as f:
    lines = [line.strip() for line in f if line.strip()]

# --- Tạo ảnh ---
img = Image.new("1", (WIDTH, HEIGHT), color=0)
draw = ImageDraw.Draw(img)

# --- Load font ---
try:
    font = ImageFont.truetype(FONT_PATH, FONT_SIZE)
except IOError:
    print("❌ Font không tồn tại. Dùng: sudo apt install fonts-dejavu")
    sys.exit(1)

# --- Tính toán line spacing ---
line_spacing = FONT_SIZE + 2
max_lines = HEIGHT // line_spacing
visible_lines = lines[:max_lines]

# --- Vẽ văn bản ---
for i, line in enumerate(visible_lines):
    draw.text((0, i * line_spacing), line, font=font, fill=1)

img.save("preview.png")

# --- Chuyển sang GDDRAM format ---
pixels = img.load()
buffer = bytearray()

for page in range(8):
    for col in range(128):
        byte = 0
        for bit in range(8):
            y = page * 8 + bit
            if y < HEIGHT and pixels[col, y]:
                byte |= (1 << bit)
        buffer.append(byte)

# --- Ghi ra file .raw ---
with open(OUTPUT_FILE, "wb") as f:
    f.write(buffer)

print(f"✅ Xuất ra: {OUTPUT_FILE}, preview: preview.png")
