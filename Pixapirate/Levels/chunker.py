import xml.etree.ElementTree as ET
import SpriteSplitter as SpriteSplitter
import csv
import os
import pygame

pygame.init()
pygame.display.set_mode((1, 1))

# ---------------------------------------------------------
# TILE CLASSIFICATION ARRAYS
# ---------------------------------------------------------
WATER_TILE_INDEXES = [
    86, 4, 5, 6, 7, 8, 9,
    20, 21, 22, 23, 24, 25,
    36, 37, 38, 39, 40, 41
]

DANGER_TILE_INDEXES = [
    100, 101, 102, 103  # example
]

BLOCK_TILE_INDEXES = [
    200, 201, 202, 203  # example
]

# Collision keys
WATER = 1
DANGER = 2
BLOCKED = 3
WALKABLE = 4


# ---------------------------------------------------------
# LOAD TILESET
# ---------------------------------------------------------
def LoadTileset(path):
    tileset_image = pygame.image.load(path)
    tiles = SpriteSplitter.SpriteSplitter.split_by_grid(tileset_image, 16, 16)

    resizedtiles = []
    for tile in tiles:
        resizedtile = pygame.transform.scale(tile, (48, 48))
        resizedtiles.append(resizedtile)

    return resizedtiles


# ---------------------------------------------------------
# PARSE TMX INTO CHUNKS
# ---------------------------------------------------------
def parse_tmx_chunks(tmx_file):
    tree = ET.parse(tmx_file)
    root = tree.getroot()
    chunks_data = []

    chunk_size = 16
    tile_width = int(root.attrib['tilewidth'])
    tile_height = int(root.attrib['tileheight'])

    for layer in root.findall('.//layer'):
        for chunk in layer.findall('.//chunk'):
            x = int(chunk.attrib['x'])
            y = int(chunk.attrib['y'])
            chunk_text = chunk.text.strip()
            rows = chunk_text.split('\n')
            chunk_data = []

            for row in rows:
                if row.strip():
                    chunk_data.append([int(val.strip()) for val in row.strip().strip(',').split(',') if val.strip()])

            # Pad to 16x16
            while len(chunk_data) < chunk_size:
                chunk_data.append([0] * chunk_size)
            for row in chunk_data:
                while len(row) < chunk_size:
                    row.append(0)

            pixel_x = x * tile_width
            pixel_y = y * tile_height
            chunks_data.append((pixel_x, pixel_y, chunk_data))

    return chunks_data


# ---------------------------------------------------------
# WRITE CHUNKS TO CSV (unchanged)
# ---------------------------------------------------------
def write_chunks_to_csv(chunks_data, output_file):
    with open(output_file, 'w', newline='') as f:
        writer = csv.writer(f)
        for x, y, chunk in chunks_data:
            writer.writerow([f"CHUNK {x},{y}"])
            for row in chunk:
                writer.writerow(row)
            writer.writerow([])


# ---------------------------------------------------------
# CREATE CHUNK PNGs + ONE GIANT COLLISION FILE
# ---------------------------------------------------------
def Create_Chunk_Surface(path_to_csv, tiles):
    tile_size = 48
    chunk_width = 16
    chunk_height = 16
    chunkobjects = []

    # Giant collision map (list of rows)
    giant_collision_map = []

    with open(path_to_csv, 'r') as csvfile:
        chunkdata = []
        chunk_x = 0
        chunk_y = 0

        for row in csvfile:
            stripped = row.strip().replace('"', '')

            if stripped.startswith("CHUNK"):
                coords_str = stripped.split()[1]
                x_str, y_str = coords_str.split(",")
                chunk_x = int(x_str) * tile_size
                chunk_y = int(y_str) * tile_size
                chunkdata = []

            elif stripped == "":
                if chunkdata:
                    # Create visual chunk
                    chunk_surface = pygame.Surface((chunk_width * tile_size, chunk_height * tile_size), pygame.SRCALPHA)
                    chunk_surface.fill((0, 0, 0, 0))

                    # Build collision chunk
                    collision_chunk = []

                    for y, row in enumerate(chunkdata):
                        collision_row = []
                        for x, tile_index in enumerate(row):

                            # Draw tile
                            if tile_index != 0:
                                tile_image = tiles[tile_index - 1]
                                chunk_surface.blit(tile_image, (x * tile_size, y * tile_size))

                            # Classify tile
                            if tile_index in WATER_TILE_INDEXES:
                                collision_row.append(WATER)
                            elif tile_index in DANGER_TILE_INDEXES:
                                collision_row.append(DANGER)
                            elif tile_index in BLOCK_TILE_INDEXES:
                                collision_row.append(BLOCKED)
                            else:
                                collision_row.append(WALKABLE)

                        collision_chunk.append(collision_row)

                    chunkobjects.append((chunk_x, chunk_y, chunk_surface))
                    giant_collision_map.append(collision_chunk)
                    chunkdata = []

            else:
                introw = [int(i) for i in stripped.split(",") if i != ""]
                chunkdata.append(introw)

    # ---------------------------------------------------------
    # SAVE OUTPUT
    # ---------------------------------------------------------
    chunkobjects.sort(key=lambda tup: (tup[1], tup[0]))

    visual_output_folder = os.path.join(os.path.dirname(__file__), "startlvl", "chunks")
    collision_output_folder = os.path.join(os.path.dirname(__file__), "startlvl")
    os.makedirs(visual_output_folder, exist_ok=True)

    # Save chunk PNGs
    for i, (x, y, img) in enumerate(chunkobjects):
        chunk_filename = f"Chunk_{i}.png"
        pygame.image.save(img, os.path.join(visual_output_folder, chunk_filename))

    # Save giant collision map
    with open(os.path.join(collision_output_folder, "world_collision.txt"), "w") as f:
        for chunk in giant_collision_map:
            for row in chunk:
                f.write(" ".join(str(v) for v in row) + "\n")
            


# ---------------------------------------------------------
# MAIN
# ---------------------------------------------------------
def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    input_file = os.path.join(script_dir, 'start.tmx')
    output_file = os.path.join(script_dir, 'start.csv')

    chunks_data = parse_tmx_chunks(input_file)
    write_chunks_to_csv(chunks_data, output_file)
    print(f"Successfully extracted chunks to {output_file}")

    tiles = LoadTileset("Art/Tileset/WorldTileset.png")
    Create_Chunk_Surface(output_file, tiles)


if __name__ == "__main__":
    main()