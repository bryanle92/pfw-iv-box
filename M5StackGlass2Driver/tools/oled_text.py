from PIL import Image, ImageDraw, ImageFont
import sys

# --- Configuration ---
WIDTH, HEIGHT = 128, 64
FONT_PATH = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
FONT_SIZE = 12
OUTPUT_FILE = "ssd1309_text.raw"

# --- Read text file ---
if len(sys.argv) != 2:
    print("Usage: python3 oled_text_from_file.py <textfile.txt>")
    sys.exit(1)

with open(sys.argv[1], "r", encoding="utf-8") as f:
    lines = [line.strip() for line in f if line.strip()]

# --- Create image ---
img = Image.new("1", (WIDTH, HEIGHT), color=0)
draw = ImageDraw.Draw(img)

# --- Load font ---
try:
    font = ImageFont.truetype(FONT_PATH, FONT_SIZE)
except IOError:
    print("❌ Font not found. Install with: sudo apt install fonts-dejavu")
    sys.exit(1)

# --- Calculate line spacing ---
line_spacing = FONT_SIZE + 2
max_lines = HEIGHT // line_spacing
visible_lines = lines[:max_lines]

# --- Draw text ---
for i, line in enumerate(visible_lines):
    draw.text((0, i * line_spacing), line, font=font, fill=1)

img.save("preview.png")

# --- Convert to GDDRAM format ---
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

# --- Write to .raw file ---
with open(OUTPUT_FILE, "wb") as f:
    f.write(buffer)

print(f"✅ Output: {OUTPUT_FILE}, preview: preview.png")
