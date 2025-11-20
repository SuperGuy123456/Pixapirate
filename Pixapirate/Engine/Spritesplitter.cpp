#include "Spritesplitter.h"

void  SpriteSplitter::Clothing(const char* hatpath,
    const  char* headpath,
    const char* torsopath,
    const char* armspath,
    const char* legspath,
    AnimationFrames& hatvector,
    AnimationFrames& headvector,
    AnimationFrames& torsovector,
    AnimationFrames& armsvector,
    AnimationFrames& legsvector)
{
    vector<Texture2D> outframes; //every 7 frames is next body part (for each bodypart, the first four frames are running anim and last 3 are idle, 7 total)

    vector<Texture2D> hat_frames = SplitByHorizontal(hatpath, 15, 3.0f); //Each frame is 15 width x 24 high
    vector<Texture2D> head_frames = SplitByHorizontal(headpath, 15, 3.0f);
    vector<Texture2D> torso_frames = SplitByHorizontal(torsopath, 15, 3.0f);
    vector<Texture2D> arms_frames = SplitByHorizontal(armspath, 15, 3.0f);
    vector<Texture2D> legs_frames = SplitByHorizontal(legspath, 15, 3.0f);

    vector<vector<Texture2D>> bodyframes = { hat_frames, head_frames, torso_frames, arms_frames, legs_frames };
    vector<AnimationFrames*> animframesptrs = { &hatvector, &headvector,&torsovector,&armsvector,&legsvector };

    int counter = 0;

    for (const vector<Texture2D>& bodyframe : bodyframes) //hat_frames, head_frames etc
    {
        for (int findex = 0; findex < 7; findex++)
        {
            if (findex < 4)
            {
                animframesptrs[counter]->run.push_back(bodyframe[findex]);
            }
            else
            {
                animframesptrs[counter]->idle.push_back(bodyframe[findex]);
            }
        }
        counter++;
    }

}

vector<Texture2D> SpriteSplitter::SplitByHorizontal(const char* path, float slicewidth, float scaleFactor)
{
    Texture2D unsplittex = LoadTexture(path);
    Image unsplitimage = LoadImageFromTexture(unsplittex);
    UnloadTexture(unsplittex); // Free GPU memory

    vector<Texture2D> splittedtxs;

    float texturewidth = static_cast<float>(unsplitimage.width);
    float textureheight = static_cast<float>(unsplitimage.height);

    for (float x = 0; x < texturewidth; x += slicewidth)
    {
        Image splitpart = ImageCopy(unsplitimage);
        ImageCrop(&splitpart, Rectangle{ x, 0, slicewidth, textureheight });

        if (scaleFactor != 1.0f)
        {
            int newWidth = static_cast<int>(slicewidth * scaleFactor);
            int newHeight = static_cast<int>(textureheight * scaleFactor);
            ImageResizeNN(&splitpart, newWidth, newHeight); // Nearest-neighbor scaling
        }

        Texture2D frame = LoadTextureFromImage(splitpart);
        SetTextureFilter(frame, TEXTURE_FILTER_POINT); // Preserve pixel sharpness
        splittedtxs.push_back(frame);

        UnloadImage(splitpart);
    }

    UnloadImage(unsplitimage); // Free CPU memory
    return splittedtxs;
}