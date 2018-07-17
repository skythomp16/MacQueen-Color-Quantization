#include<stdio.h>
#include<stdlib.h>
#include <time.h>

//Structures for each individual pixel
// red, green, blue should be of type 'double'
typedef struct {
    char red, green, blue;
} PPMPixel;

//Structure for cluster
typedef struct {
    int size;
    PPMPixel center;
} PPMCluster;

//Image structure
typedef struct {
    unsigned int width, height;  //Included for rebuilding the image
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
    if (fscanf(fp, "%u %u", &img->width, &img->height) != 2) {
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
    img->data = (PPMPixel*)malloc(img->width * img->height * sizeof(PPMPixel));

    if (!img) {
        fprintf(stderr, "Unable to allocate memory\n");
        exit(1);
    }

    //read pixel data from file
    if (fread(img->data, 3 * img->width, img->height, fp) != img->height) {
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
    fprintf(fp, "%d %d\n", img->width, img->height);

    // rgb component depth
    fprintf(fp, "%d\n", RGB_COMPONENT_COLOR);

    // pixel data
    fwrite(img->data, 3 * img->width, img->height, fp);
    fclose(fp);
}

void macqueenClustering(PPMImage *img, int numColors)
{
    //Filling an array with random numbers for number of colors to be quantized down to
    int numFixedColors = 64;

    //Random number generator (for selecting random centers)
    srand(time(0));

    //Make an array of clusters to coorespond with the array of centers
    PPMCluster* clusters = malloc(numFixedColors * sizeof(*clusters));

    //Variable to indicate the number of pixels in the image
    int numPixels = (img->width * img->height);

    //Initialize each center to a random value and give each cluster a size of 1
    for (int i = 0; i < numFixedColors; i++)
    {
        PPMPixel myTemp = img->data[rand() % (numPixels)];
        clusters[i].center = myTemp;
        clusters[i].size = 1;
    }

    //Now time for data clustering using k-means
    //First, Some variables
    int terminate = numPixels; //terminates after iterating over every pixel in the image
    int randPixNum;
    int index = 0;
    PPMPixel closest;
    PPMCluster temp;
    int diffG;
    int diffR;
    int diffB;
    int totalRGB = 0;
    int nearest;
    double tempTotalRGB;

    int counter = 0;

    //Terminate when criteria is met
    for (index = 0; index < terminate; index++ )
     {
        //Now, select a random pixel from the pixel array (pick a random pixel from the image)
        randPixNum = rand();
        PPMPixel randPix = img->data[randPixNum];

        //Set totalRGB to be the highest it can be
        totalRGB = 195075; // 3 * 255 * 255 

        //Next, find the closest pixel to this pixel in the centers array
        for (int i = 0; i < numFixedColors; i++) {
            temp = clusters[i];

            diffB = (randPix.blue - temp.center.blue) * (randPix.blue - temp.center.blue);
            diffG = (randPix.green - temp.center.green) * (randPix.green - temp.center.green);
            diffR = (randPix.red - temp.center.red) * (randPix.red - temp.center.red);
            
	        tempTotalRGB = diffR + diffG + diffB;

            if (tempTotalRGB < totalRGB)
            {
                totalRGB = tempTotalRGB;
		        nearest = i;
                /*
                printf("I entered this function!   ");
                counter++;
                printf("%d", randPix.blue);
                printf("   ");
                printf("%d\n", counter);
                */
            }
        
        }
        //Update the center in the centers array ci = (Ni ci + xr)/(Ni + 1)
            int old_size = clusters[nearest].size;
            int new_size = old_size + 1;

            clusters[nearest].center.red = ( old_size * clusters[nearest].center.red + randPix.red ) / (double) new_size;
            clusters[nearest].center.green = ( old_size * clusters[nearest].center.green + randPix.green ) / (double) new_size;
            clusters[nearest].center.blue = ( old_size * clusters[nearest].center.blue + randPix.blue ) / (double) new_size;
            clusters[nearest].size = new_size;      
    } 


    //For testing purposes to see the size of each of the 64 data clusters
    counter = 1;
    int totalSize = 0;
    for (int i = 0; i < numFixedColors; i++)
    {
        int size = clusters[i].size;
        totalSize += size;
        printf("%d", counter);
        printf("     ");
        counter++;
        printf("%d", size);
        printf("     ");
        printf("%d\n", totalSize);
    }

    //Now quantize the image


    
}

//Main function
int main() {
    //Create a new image object and read the image in
    PPMImage *image;
    image = readPPM("sample.ppm");

    //Organize the pixels into clusters
    macqueenClustering(image, 64);

    //Create a new image based on the color quantization
    writePPM("new.ppm", image);

    //Wait for user response
    printf("Press any key...");
    getchar();
}
