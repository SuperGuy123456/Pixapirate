#pragma once
#include "raylib.h"
#include "../Engine/Allstructs.h"
#include <vector>
#include <string>

using namespace std;

class SpriteSplitter
{
public:
	static void Clothing(const char* hatpath, //Preconfigured to provide a list of all of the pre split frames from clothing
		const  char* headpath,
		const char* torsopath,
		const char* armspath,
		const char* legspath,
		AnimationFrames& hatvector, //refs to change the vectors directly >:)
		AnimationFrames& headvector,
		AnimationFrames& torsovector,
		AnimationFrames& armsvector,
		AnimationFrames& legsvector); 
	static vector<Texture2D> SplitByHorizontal(const char* path, float slicewidth, float scalefactor); //Loads a image and splits it by a slice width (cookie cutter style)
};