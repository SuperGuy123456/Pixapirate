#pragma once
#include "raylib.h"
#include <vector>
#include <string>

using namespace std;

class SpriteSplitter
{
public:
	static Texture2D Clothing(string hatpath, string headpath, string torsopath, string armspath, string legspath); //Preconfigured to provide a list of all of the pre split frames from clothing
	static Texture2D SplitByHorizontal(char* path, int slicewidth); //Loads a image and splits it by a slice width (cookie cutter style)
};