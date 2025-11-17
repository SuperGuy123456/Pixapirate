import pygame

class SpriteSplitter:
    @staticmethod
    def split_horizontal_sprite_sheet(sheet, sprite_width, sprite_height):
        splitted_sprites = []
        for sprite in range(0, sheet.get_width(), sprite_width):
            sp = sheet.subsurface(pygame.Rect(sprite, 0, sprite_width, sprite_height))
            splitted_sprites.append(sp)
        print("Splitted: " + str(len(splitted_sprites)) + " sprites.")
        return splitted_sprites
    @staticmethod
    def split_by_grid(sheet, sprite_width, sprite_height):
        #it goes left to right,top to bottom
        splitted_sprites = []
        for y in range(0, sheet.get_height(), sprite_height):
            for x in range(0, sheet.get_width(), sprite_width):
                sp = sheet.subsurface(pygame.Rect(x, y, sprite_width, sprite_height))
                splitted_sprites.append(sp)
        print("Splitted: " + str(len(splitted_sprites)) + " sprites.")
        return splitted_sprites