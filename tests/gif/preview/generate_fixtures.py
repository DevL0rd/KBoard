import base64
import io
import json
import math
import os
import sys

from PIL import Image, ImageDraw, ImageFont

WORDS = ["hello", "lol", "yes!", "omg", "nope", "wow", "thanks", "yay", "hmm", "ok", "love", "bye",
         "facepalm", "cheers", "hug", "what", "deal", "party", "sleepy", "go", "nice", "ugh", "clap", "dance"]
SIZES = [(220, 220), (220, 124), (220, 300), (220, 165), (220, 260), (220, 146),
         (220, 220), (220, 330), (220, 180), (220, 124), (220, 240), (220, 200)]
COLORS = [(233, 84, 32), (52, 152, 219), (155, 89, 182), (46, 204, 113), (241, 196, 15), (231, 76, 60),
          (26, 188, 156), (230, 126, 34), (52, 73, 94), (236, 64, 122), (0, 150, 136), (121, 85, 72)]
CATEGORIES = ["smile", "aww", "high five", "good morning", "good night", "thank you", "birthday", "dance"]
SUGGESTIONS = ["hello", "hello there", "hello kitty", "help", "hell yeah", "hello friend"]
FRAMES = 12


def load_font():
    try:
        return ImageFont.truetype("/usr/share/fonts/noto/NotoSans-Bold.ttf", 34)
    except OSError:
        return ImageFont.load_default()


def draw_frame(size, color, phase, word, font):
    width, height = size
    image = Image.new("RGBA", size)
    draw = ImageDraw.Draw(image)
    for y in range(height):
        shade = 1 - 0.45 * y / height
        draw.line([(0, y), (width, y)], fill=(int(color[0] * shade), int(color[1] * shade), int(color[2] * shade), 255))
    cx = width / 2 + math.cos(phase) * width * 0.22
    cy = height / 2 + math.sin(phase * 2) * height * 0.12
    radius = min(width, height) * 0.28
    draw.ellipse([cx - radius, cy - radius, cx + radius, cy + radius], fill=(255, 255, 255, 70), outline=(255, 255, 255, 200), width=4)
    text_width = draw.textlength(word, font=font)
    draw.text((width / 2 - text_width / 2 + 2, height - 52), word, font=font, fill=(0, 0, 0, 120))
    draw.text((width / 2 - text_width / 2, height - 54), word, font=font, fill=(255, 255, 255, 255))
    return image


def blur_preview(frame, size):
    buffer = io.BytesIO()
    frame.convert("RGB").resize((max(1, size[0] // 16), max(1, size[1] // 16))).save(buffer, "JPEG")
    return "data:image/jpeg;base64," + base64.b64encode(buffer.getvalue()).decode()


def rendition(path, size):
    return {"url": "file://" + path, "width": size[0], "height": size[1], "size": os.path.getsize(path)}


def make_item(index, media, font):
    size = SIZES[index % len(SIZES)]
    word = WORDS[index]
    frames = [draw_frame(size, COLORS[index % len(COLORS)], 2 * math.pi * f / FRAMES, word, font) for f in range(FRAMES)]
    base = os.path.join(media, f"g{index:02d}")
    frames[0].save(base + ".webp", save_all=True, append_images=frames[1:], duration=80, loop=0)
    rgb = [frame.convert("RGB") for frame in frames]
    rgb[0].save(base + ".gif", save_all=True, append_images=rgb[1:], duration=80, loop=0)
    gif = rendition(base + ".gif", size)
    return {
        "id": 8041071659142000 + index,
        "slug": f"{word}-{index}",
        "title": word.title(),
        "file": {"hd": {"gif": gif}, "md": {"gif": gif}, "sm": {"gif": gif, "webp": rendition(base + ".webp", size)}},
        "tags": [word],
        "type": "gif",
        "blur_preview": blur_preview(frames[0], size),
    }


def page(items):
    return {"result": True, "data": {"data": items, "current_page": 1, "per_page": 24, "has_next": True}}


def write_json(directory, name, payload):
    with open(os.path.join(directory, name), "w", encoding="utf-8") as handle:
        json.dump(payload, handle)


def main():
    output = os.path.abspath(sys.argv[1])
    media = os.path.join(output, "media")
    os.makedirs(media, exist_ok=True)
    font = load_font()
    items = [make_item(index, media, font) for index in range(len(WORDS))]
    write_json(output, "trending.json", page(items))
    write_json(output, "search.json", page(list(reversed(items[:10]))))
    write_json(output, "autocomplete.json", {"result": True, "data": SUGGESTIONS})
    categories = [{"category": name, "query": name, "preview_url": ""} for name in CATEGORIES]
    write_json(output, "categories.json", {"result": True, "data": {"locale": "en_US", "categories": categories}})


if __name__ == "__main__":
    main()
