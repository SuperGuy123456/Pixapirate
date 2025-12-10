import xml.etree.ElementTree as ET
import SpriteSplitter as SpriteSplitter
import csv
import os
import pygame
pygame.init()
pygame.display.set_mode((1, 1))  # Required for image conversion if needed

def LoadTileset(path):
    tileset_image = pygame.image.load(path)
    tiles = SpriteSplitter.SpriteSplitter.split_by_grid(tileset_image, 16, 16)

    resizedtiles = []
    for tile in tiles:
        resizedtile = pygame.transform.scale(tile, (48, 48))
        resizedtiles.append(resizedtile)

    return resizedtiles

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

            # Pad to 16x16 if needed
            while len(chunk_data) < chunk_size:
                chunk_data.append([0] * chunk_size)
            for row in chunk_data:
                while len(row) < chunk_size:
                    row.append(0)

            # Convert chunk position to pixels
            pixel_x = x * tile_width
            pixel_y = y * tile_height
            chunks_data.append((pixel_x, pixel_y, chunk_data))

    return chunks_data

def write_chunks_to_csv(chunks_data, output_file):
    with open(output_file, 'w', newline='') as f:
        writer = csv.writer(f)
        for x, y, chunk in chunks_data:
            writer.writerow([f"CHUNK {x},{y}"])
            for row in chunk:
                writer.writerow(row)
            writer.writerow([])
def Create_Chunk_Surface(path_to_csv, tiles):
    tile_size = 48
    chunk_width = 16
    chunk_height = 16
    chunkobjects = []

    WATER_TILE_INDEXES = [
        86, 4, 5, 6, 7, 8, 9,
        20, 21, 22, 23, 24, 25,
        36, 37, 38, 39, 40, 41
    ]

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
                    # Create visual chunk surface
                    chunk_surface = pygame.Surface((chunk_width * tile_size, chunk_height * tile_size), pygame.SRCALPHA)
                    chunk_surface.fill((0, 0, 0, 0))

                    # Create water collider surface
                    collider_surface = pygame.Surface((chunk_width * tile_size, chunk_height * tile_size), pygame.SRCALPHA)
                    collider_surface.fill((0, 0, 0, 0))

                    for y, row in enumerate(chunkdata):
                        for x, tile_index in enumerate(row):
                            if tile_index != 0:
                                tile_image = tiles[tile_index - 1]
                                pos_x = x * tile_size
                                pos_y = y * tile_size
                                chunk_surface.blit(tile_image, (pos_x, pos_y))
                            if tile_index in WATER_TILE_INDEXES:   # <-- changed here
                                pygame.draw.rect(
                                    collider_surface,
                                    (255, 255, 255, 255),  # Solid white
                                    (x * tile_size, y * tile_size, tile_size, tile_size)
                                )

                    chunkobjects.append((chunk_x, chunk_y, chunk_surface, collider_surface))
                    print(f"Loaded Chunk at ({chunk_x}, {chunk_y})")
                    chunkdata = []

            else:
                introw = [int(i) for i in stripped.split(",") if i != ""]
                chunkdata.append(introw)

        # Final chunk fallback
        if chunkdata:
            chunk_surface = pygame.Surface((chunk_width * tile_size, chunk_height * tile_size), pygame.SRCALPHA)
            chunk_surface.fill((0, 0, 0, 0))

            collider_surface = pygame.Surface((chunk_width * tile_size, chunk_height * tile_size), pygame.SRCALPHA)
            collider_surface.fill((0, 0, 0, 0))

            for y, row in enumerate(chunkdata):
                for x, tile_index in enumerate(row):
                    if tile_index != 0:
                        tile_image = tiles[tile_index - 1]
                        pos_x = x * tile_size
                        pos_y = y * tile_size
                        chunk_surface.blit(tile_image, (pos_x, pos_y))
                    if tile_index in WATER_TILE_INDEXES:   # <-- changed here
                        pygame.draw.rect(
                            collider_surface,
                            (255, 255, 255, 255),
                            (x * tile_size, y * tile_size, tile_size, tile_size)
                        )

            chunkobjects.append((chunk_x, chunk_y, chunk_surface, collider_surface))
            print(f"Loaded Final Chunk at ({chunk_x}, {chunk_y})")

    # Sort and save
    chunkobjects.sort(key=lambda tup: (tup[1], tup[0]))

    visual_output_folder = os.path.join(os.path.dirname(__file__), "startlvl", "chunks")
    collider_output_folder = os.path.join(os.path.dirname(__file__), "startlvl", "chunkcolls")
    os.makedirs(visual_output_folder, exist_ok=True)
    os.makedirs(collider_output_folder, exist_ok=True)

    for i, (x, y, img, collider_img) in enumerate(chunkobjects):
        chunk_filename = f"Chunk_{i}.png"
        chunk_path = os.path.join(visual_output_folder, chunk_filename)
        pygame.image.save(img, chunk_path)

        collider_filename = f"Chunk_{i}_collider.png"
        collider_path = os.path.join(collider_output_folder, collider_filename)
        pygame.image.save(collider_img, collider_path)

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
