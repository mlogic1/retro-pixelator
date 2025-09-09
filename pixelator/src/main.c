#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pixelator/pixelator.h>

typedef struct {
	const char* input_file;
	const char* output_file;
	int block_size;
	int num_colors;
} Arguments;

void PrintUsage()
{
	printf("usage: pixelator -i input_image -o output_image [-b block_size] [-k num_colors]\n\n\
	input_image - Input image to filter. Currently only jpeg images are supported.\n\n\
	output_image - Output where the filtered image will be saved.\n\n\
	block_size - Downsample block size. Default is 8\n\n\
	num_colors - Number of colors to reduce the input image to.\n");
}

Arguments ParseArguments(int argc, char* argv[])
{
	int opt;
	const char* input_file = NULL;
	const char* output_file = NULL;
	int block_size = 8;
	int num_colors = 24;

	while( (opt = getopt(argc, argv, "i:o:b:k:")) != -1 ){
		switch(opt)
		{
			case 'i':
				input_file = optarg;
				break;

			case 'o':
				output_file = optarg;
				break;

			case 'b':
				block_size = atoi(optarg);
				break;

			case 'k':
				num_colors = atoi(optarg);
				break;

			default:
				break;
		}
	}

	Arguments result = { input_file, output_file, block_size, num_colors };
	return result;
}

int main(int argc, char* argv[])
{
	if (argc < 3){
		PrintUsage();
	}

	Arguments args = ParseArguments(argc, argv);

	if (!args.input_file){
		printf("Input file must be specified\n");
		return EXIT_FAILURE;
	}

	if (!args.output_file){
		printf("Output file must be specified\n");
		return EXIT_FAILURE;
	}

	ImagePtr image = Pixelator_LoadImage(args.input_file);
	if (image == NULL)
	{
		printf("Unable to load input image.");
		return EXIT_FAILURE;
	}
	Pixelator_DownSampleImage(image, args.block_size);
	Pixelator_KMeansQuantify(image, args.num_colors);

	Pixelator_SaveImage(image, args.output_file);
	Pixelator_FreeImage(image);
	return EXIT_SUCCESS;
}