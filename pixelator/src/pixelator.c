#include <pixelator/pixelator.h>
#include <stdlib.h>
#include <turbojpeg.h>
#include <stdio.h>
#include <float.h>

typedef struct
{
	int r;
	int g;
	int b;
}JColor;

struct PImage
{
	JColor* pixels;
	int width;
	int height;
};


ImagePtr Pixelator_LoadImage(const char* imagePath)
{
	FILE* fp = fopen(imagePath, "rb");

	if (fp == NULL){
		// set some error
		return NULL;
	}

	fseek(fp, 0, SEEK_END);
	long int inputImageSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	unsigned char* inputImageBuffer = malloc(sizeof(unsigned char) * inputImageSize);

	size_t readBytes = fread(inputImageBuffer, sizeof(unsigned char), inputImageSize, fp);

	tjhandle instance = tjInitDecompress();
	int w, h, jpegSubsamp, colorSpace;
	if (tjDecompressHeader3(instance, inputImageBuffer, inputImageSize, &w, &h, &jpegSubsamp, &colorSpace)){
		tjDestroy(instance);
		free(inputImageBuffer);
		return NULL;
	}

	ImagePtr image = malloc(sizeof(PImage));
	image->width = w;
	image->height = h;

	unsigned char* tmpRGBBuffer = malloc(sizeof(unsigned char) * w * h * 3);
	if (tjDecompress2(instance, inputImageBuffer, inputImageSize, tmpRGBBuffer, w, 0, h, TJPF_RGB, TJFLAG_FASTDCT) != 0){
		tjDestroy(instance);
		free(tmpRGBBuffer);
		free(inputImageBuffer);
		free(image);
		return NULL;
	}

	image->pixels = malloc(sizeof(JColor) * w * h * 3);
	const int stride = 3;
	int numPixels = w * h;
	for (int i = 0; i < numPixels; ++i){
		int index = i * stride;
		image->pixels[i].r = tmpRGBBuffer[index];
		image->pixels[i].g = tmpRGBBuffer[index + 1];
		image->pixels[i].b = tmpRGBBuffer[index + 2];
	}

	free(tmpRGBBuffer);
	free(inputImageBuffer);
	tjDestroy(instance);
	return image;
}

void Pixelator_FreeImage(PImage* image)
{
	free(image->pixels);
	free(image);
	image->height = 0;
	image->width = 0;
}

void Pixelator_GetImageResolution(ImagePtr image, int* width, int* height)
{
	if (image == NULL){
		return;
	}

	*width = image->width;
	*height = image->height;
}

int Pixelator_SaveImage(ImagePtr image, const char* imagePath)
{
	tjhandle instance = tjInitCompress();

	unsigned char* jpegBuff = NULL;
	unsigned long int jpegSize = 0;

	// convert image to RGB buffer
	const int numPixels = image->width * image->height;
	unsigned int rgbBufferSize = numPixels * 3;
	unsigned char* tmpRGBBuffer = malloc(sizeof(unsigned char) * rgbBufferSize);
	const int stride = 3;

	int index = 0;
	for (int i = 0; i < numPixels; ++i){

		tmpRGBBuffer[index] 	= image->pixels[i].r;
		tmpRGBBuffer[index + 1] = image->pixels[i].g;
		tmpRGBBuffer[index + 2] = image->pixels[i].b;

		index += stride;
	}

	if (tjCompress2(instance, tmpRGBBuffer, image->width, 0, image->height, TJPF_RGB, &jpegBuff, &jpegSize, TJSAMP_444, 90, TJFLAG_FASTDCT) != 0)
	{
		// prepare some error message
		free(tmpRGBBuffer);
		tjDestroy(instance);
		return 1;
	}

	FILE* fp = fopen(imagePath, "wb");
	if (fp == NULL){
		// set some error
		free(tmpRGBBuffer);
		tjDestroy(instance);
		return -1;
	}

	fwrite(jpegBuff, sizeof(char), jpegSize, fp);
	fclose(fp);
	free(jpegBuff);
	free(tmpRGBBuffer);
	tjDestroy(instance);
	return 0;
}

int Pixelator_DownSampleImage(ImagePtr image, const int blockSize)
{
	if ( blockSize < 0 || blockSize > 64){
		return 1;
	}

	int w = image->width, h = image->height;

	for (int i = 0; i < h; i += blockSize){
		for (int j = 0; j < w; j += blockSize){
			JColor blockAvgColor = {0, 0, 0};
			int pixelCount = 0;
			int rTotal = 0, gTotal = 0, bTotal = 0;
			for (int di = i; di < (i + blockSize); ++di){
				for (int dj = j; dj < (j + blockSize); ++dj){

					if (dj >= w || di >= h) continue; // bounds check

					if (dj == 49){
						int x = 1;
					}

					int index = di * w + dj;
					JColor pixel = image->pixels[index];

					rTotal += pixel.r;
					gTotal += pixel.g;
					bTotal += pixel.b;
					++pixelCount;
				}
			}
			if (pixelCount == 0)
				continue;

			blockAvgColor.r = rTotal / pixelCount;
			blockAvgColor.g = gTotal / pixelCount;
			blockAvgColor.b = bTotal / pixelCount;
			// block average calculated, write it to all pixels
			for (int di = i; di < (i + blockSize); ++di)
				for (int dj = j; dj < (j + blockSize); ++dj){
					if (dj >= w || di >= h) continue;

					int index = di * w + dj;
					image->pixels[index] = blockAvgColor;
				}
		}
	}

	return 0;
}

float utility_colorDistance(const JColor* colorA, const JColor* colorB)
{
	return 	(colorA->r - colorB->r) * (colorA->r - colorB->r) +
		(colorA->g - colorB->g) * (colorA->g - colorB->g) +
		(colorA->b - colorB->b) * (colorA->b - colorB->b);
}

static void utility_kMeansCalculateCenteroids(ImagePtr image, int K, JColor* outCentroids, int iterations)
{
	int numPixels = image->width * image->height;
	JColor* centroids = malloc(sizeof(JColor) * K);
	int* assingments = malloc(sizeof(int) * numPixels);

	for (int i = 0; i < K; ++i)
	{
		centroids[i] = image->pixels[rand() % numPixels];
	}

	for (int iter = 0; iter < iterations; ++iter){
		// assign
		for (int i = 0; i < numPixels; ++i){
			float minDist = FLT_MAX;
			int best = 0;
			for (int j = 0; j < K; ++j){
				float dist = utility_colorDistance(&image->pixels[i], &centroids[j]);
				if (dist < minDist){
					minDist = dist;
					best = j;
				}
			}
			assingments[i] = best;
		}

		// update
		JColor* newCentroids = malloc(sizeof(JColor) * K);
		int* counts = (int*)calloc(K, sizeof(int));

		for (int i = 0; i < numPixels; ++i)
		{
			int cluster = assingments[i];
			newCentroids[cluster].r += image->pixels[i].r;
			newCentroids[cluster].g += image->pixels[i].g;
			newCentroids[cluster].b += image->pixels[i].b;
			counts[cluster]++;
		}

		for (int j = 0; j < K; ++j)
		{
			if (counts[j] > 0)
			{
				newCentroids[j].r /= counts[j];
				newCentroids[j].g /= counts[j];
				newCentroids[j].b /= counts[j];
			}
			else
			{
				newCentroids[j] = image->pixels[rand() % numPixels];
			}
		}

		free(centroids);
		centroids = newCentroids;
		free(counts);
	}

	for (int i = 0; i < K; ++i){
		outCentroids[i] = centroids[i];
	}

	free(assingments);
	free(centroids);
}

int Pixelator_KMeansQuantify(ImagePtr image, int K)
{
	if (K > 64){
		// set some error message
		return -1;
	}
	JColor* centroids = malloc(sizeof(JColor) * K);
	utility_kMeansCalculateCenteroids(image, K, centroids, 10);
	const int numPixels = image->width * image->height;

	for (int i = 0; i < numPixels; ++i){
		float minDist = FLT_MAX;
		int best = 0;

		for (int j = 0; j < K; ++j){
			float dist = utility_colorDistance(&image->pixels[i], &centroids[j]);
			if (dist < minDist){
				minDist = dist;
				best = j;
			}
		}
		image->pixels[i] = centroids[best];
	}

	free(centroids);
	return 0;
}