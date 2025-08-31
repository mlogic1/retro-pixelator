#ifndef PIXELATOR_H
#define PIXELATOR_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PImage PImage;
typedef PImage* ImagePtr;

ImagePtr Pixelator_LoadImage(const char* imagePath);
void Pixelator_FreeImage(ImagePtr image);
int Pixelator_SaveImage(ImagePtr image, const char* imagePath);

int Pixelator_DownSampleImage(ImagePtr image, const int blockSize);
int Pixelator_KMeansQuantify(ImagePtr image, int K);

#ifdef __cplusplus
}
#endif

#endif // PIXELATOR_h