#include "Spritesplitter.h"

Texture2D SpriteSplitter::Clothing(string hatpath, string headpath, string torsopath, string armspath, string legspath)
{

}

Texture2D SpriteSplitter::SplitByHorizontal(char* path, int slicewidth)
{
	Texture2D unsplittex = LoadTexture(path);
	Image splitted[] = {};

	int texturewidth = unsplittex.width;
	int	textureheight = unsplittex.height;

	Image unsplitimage = LoadImageFromTexture(unsplittex);

	for (int x; x < texturewidth; x+slicewidth)
	{

	}

}