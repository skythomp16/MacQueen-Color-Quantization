#include<stdio.h>
#include<stdlib.h>
#include <time.h>

//Structures for each individual pixel
typedef struct {
    unsigned char red, green, blue;
} PPMPixel;

//Structure for cluster
typedef struct {
    int size;
    PPMPixel center;
} PPMCluster;

//Image structure
typedef struct {
    int x, y;  //Included for rebuilding the image
    PPMPixel *data;
} PPMImage;

#define CREATOR "RPFELGUEIRAS"
#define RGB_COMPONENT_COLOR 255

//Funcion that will read image information in
static PPMImage *readPPM(const char *filename)
{
    char buff[16];
    PPMImage *img;
    FILE *fp;
    int c, rgb_comp_color;

    //open PPM file for reading
    fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Unable to open file '%s'\n", filename);
        exit(1);
    }

    //read image format
    if (!fgets(buff, sizeof(buff), fp)) {
        perror(filename);
        exit(1);
    }

    //check the image format to make sure that it is binary
    if (buff[0] != 'P' || buff[1] != '6') {
        fprintf(stderr, "Invalid image format (must be 'P6')\n");
        exit(1);
    }

    //alloc memory for image
    img = (PPMImage *)malloc(sizeof(PPMImage));
    if (!img) {
        fprintf(stderr, "Unable to allocate memory\n");
        exit(1);
    }

    //check for comments
    c = getc(fp);
    while (c == '#') {
        while (getc(fp) != '\n');
        c = getc(fp);
    }

    ungetc(c, fp);

    //read image size information
    if (fscanf(fp, "%d %d", &img->x, &img->y) != 2) {
        fprintf(stderr, "Invalid image size (error loading '%s')\n", filename);
        exit(1);
    }

    //read rgb component
    if (fscanf(fp, "%d", &rgb_comp_color) != 1) {
        fprintf(stderr, "Invalid rgb component (error loading '%s')\n", filename);
        exit(1);
    }

    //check rgb component depth
    if (rgb_comp_color != RGB_COMPONENT_COLOR) {
        fprintf(stderr, "'%s' does not have 8-bits components\n", filename);
        exit(1);
    }

    while (fgetc(fp) != '\n');
    //memory allocation for pixel data
    img->data = (PPMPixel*)malloc(img->x * img->y * sizeof(PPMPixel));

    if (!img) {
        fprintf(stderr, "Unable to allocate memory\n");
        exit(1);
    }

    //read pixel data from file
    if (fread(img->data, 3 * img->x, img->y, fp) != img->y) {
        fprintf(stderr, "Error loading image '%s'\n", filename);
        exit(1);
    }

    fclose(fp);
    return img;
}

//Function to write the new image after color quantization
void writePPM(const char *filename, PPMImage *img)
{
    FILE *fp;
    //open file for output
    fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Unable to open file '%s'\n", filename);
        exit(1);
    }

    //write the header file
    //image format
    fprintf(fp, "P6\n");

    //comments
    fprintf(fp, "# Created by %s\n", CREATOR);

    //image size
    fprintf(fp, "%d %d\n", img->x, img->y);

    // rgb component depth
    fprintf(fp, "%d\n", RGB_COMPONENT_COLOR);

    // pixel data
    fwrite(img->data, 3 * img->x, img->y, fp);
    fclose(fp);
}

void colorQuantizationPPM(PPMImage *img, int numColors)
{
    //Filling an array with random numbers for number of colors to be quantized down to
    int num[64];
    for (int i = 0; i < 63; i++)
    {
        //Format (rand() % (upper - lower + 1)) + lower.  In this case lower is 1 and upper is all pixels in the image
        num[i] = (rand() % ((img->x * img->y) - 1 + 1)) + 1;
    }

    //Randomly pick centroid points and place them in an array of centers
    PPMPixel *centers = malloc(sizeof(centers) * 64);

    for (int i = 0; i < 63; i++)
    {
        centers[i] = img->data[num[i]];
        printf("%d", centers[i].red);
        printf("\n");
    }

    //Now time for data clustering using k-means
    //First, Some variables
    int terminate = (img->x * img->y); //terminates after going through every pixel in the image
    int randPixNum;
    int index = 0;
    PPMPixel closest;
    PPMPixel temp;
    int diffG;
    int diffR;
    int diffB;
    int totalRGB = 0;
    int first = 1;
    PPMCluster cluster;

    do {
        //Now, select a random pixel from the pixel array
        randPixNum = rand();
        PPMPixel randPix = img->data[randPixNum];

        //Next, find the closest pixel to this pixel in the centers array
        for (int i = 0; i < terminate; i++) {
            temp = img->data[i];

            diffB = (randPix.blue - temp.blue) * (randPix.blue - temp.blue);
            diffG = (randPix.green - temp.green) * (randPix.green - temp.green);
            diffR = (randPix.red - temp.red) * (randPix.red - temp.red);

            int tempTotalRGB = diffR + diffG + diffB;

            if (tempTotalRGB > totalRGB || first)
            {
                totalRGB = tempTotalRGB;
                first = 0;
            }

        }

        //Update the center in the centers array ci = (Ni ci + xr)/(Ni + 1)
        

        //Increment cluster size (Ni) by 1
        cluster.size += 1;

        //Implement the loop by 1;
        index++;
    } while (index < sizeof(centers)); //Terminate when all pixels in the center array have been gone through

}

//Main function
int main() {

    //Create a new image object and read the image in
    PPMImage *image;
    image = readPPM("sample.ppm");

    //Random number generator (for selecting random centers)
    srand(time(0));

    //Run color quantization on the image
    colorQuantizationPPM(image, 64);

    //Create a new image based on the color quantization
    writePPM("new.ppm", image);

    //Wait for user response
    printf("Press any key...");
    getchar();
}
