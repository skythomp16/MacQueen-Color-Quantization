#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

//Structures for each individual pixel -- for reading purposes to keep each number unsigned.
typedef struct {
    double red, green, blue;
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

#define CREATOR "Rewritten by Skyler Thompson"
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
    unsigned char *inBuf = malloc(sizeof(unsigned char));
    int i = 0;
    double conv;
    int imgSize = (img->height * img->width);
    
    //While there are still pixels left
    while(fread(inBuf, 1, 1, fp) && i < imgSize)
    {
        //Read in pixels using buffer
        conv = *inBuf;
        img->data[i].red = conv;
        fread(inBuf, 1, 1, fp);
        conv = *inBuf;
        img->data[i].green = conv;
        fread(inBuf, 1, 1, fp);
        conv = *inBuf;
        img->data[i].blue = conv;
        i++;
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

/*
    // pixel data
    fwrite(img->data, 3 * img->width, img->height, fp);
    */

    int i = 0; 
    int length = (img->height * img->width);

    while (i < length)
    {
        unsigned char r = (unsigned char)(img->data[i].red);
        unsigned char g = (unsigned char)(img->data[i].green);
        unsigned char b = (unsigned char)(img->data[i].blue);

        fwrite(&r, sizeof(unsigned char), 1, fp);
        fwrite(&g, sizeof(unsigned char), 1, fp);
        fwrite(&b, sizeof(unsigned char), 1, fp);
        i++;
    }
    fclose(fp);
}

PPMImage* macqueenClustering(PPMImage *img, int numColors)
{
    //Filling an array with random numbers for number of colors to be quantized down to
    int numFixedColors = 64;

    //Make an array of clusters
    PPMCluster* clusters = malloc(numFixedColors * sizeof(*clusters));

    //Variable to indicate the number of pixels in the image
    int numPixels = (img->width * img->height);

    //Initialize each center to a random value and give each cluster a size of 1
    for (int i = 0; i < numFixedColors; i++)
    {
        //PPMPixel myTemp = img->data[rand() % (numPixels)];
        //clusters[i].center = myTemp;
        int randomNumber = (int) (rand() / (RAND_MAX + 1.0) * numPixels);
        clusters[i].center.red = img->data[randomNumber].red;
        clusters[i].center.green = img->data[randomNumber].green;
        clusters[i].center.blue = img->data[randomNumber].blue;

        clusters[i].size = 1;
    }

    //Now time for data clustering using k-means
    //Some variables
    int numPass = 1;
    int terminate = numPixels * numPass; //terminates after iterating over every pixel in the image
    int randPixNum;
    int index = 0;
    PPMPixel closest;
    PPMCluster temp;
    double diffG;
    double diffR;
    double diffB;
    double totalRGB = 0.00;
    int nearest;
    double tempTotalRGB;
    int counter = 0;

    //Terminate when criteria is met
    for (index = 0; index < terminate; index++)
     {
        //Now, select a random pixel from the pixel array (pick a random pixel from the image)
        randPixNum = (int) (rand() / (RAND_MAX + 1.0) * numPixels);

        //Set totalRGB to be the highest it can be
        totalRGB = 195075.00; // 3 * 255 * 255 

        //Next, find the closest pixel to this pixel in the centers array
        for (int i = 0; i < numFixedColors; i++) {
            temp = clusters[i];

            diffB = (img->data[randPixNum].blue - temp.center.blue) * (img->data[randPixNum].blue - temp.center.blue);
            diffG = (img->data[randPixNum].green - temp.center.green) * (img->data[randPixNum].green - temp.center.green);
            diffR = (img->data[randPixNum].red - temp.center.red) * (img->data[randPixNum].red - temp.center.red);
            
	        tempTotalRGB = diffR + diffG + diffB;

            if (tempTotalRGB < totalRGB)
            {
                totalRGB = tempTotalRGB;
		        nearest = i;
            } 
        }

        //Update the center in the centers array ci = (Ni ci + xr)/(Ni + 1)
            int old_size = clusters[nearest].size;
            int new_size = old_size + 1;
            
            clusters[nearest].center.red = ((old_size * clusters[nearest].center.red) + img->data[randPixNum].red ) / (double) new_size;
            clusters[nearest].center.green = ((old_size * clusters[nearest].center.green) + img->data[randPixNum].green ) / (double) new_size;
            clusters[nearest].center.blue = ((old_size * clusters[nearest].center.blue) + img->data[randPixNum].blue ) / (double) new_size;
            clusters[nearest].size = new_size;      
    } 

    //Create new image object
    PPMImage *imag;
    imag = (PPMImage *)malloc(sizeof(PPMImage));
    imag->data = (PPMPixel*)malloc(img->width * img->height * sizeof(PPMPixel));
    imag->width = img->width;
    imag->height = img->height;

    //Now quantize the image
    for (int i = 0; i < numPixels; i++)
    {
        totalRGB = 195075.00;
        
        for (int j = 0; j < numFixedColors; j++) 
        {
            diffB = (img->data[i].blue - clusters[j].center.blue) * (img->data[i].blue - clusters[j].center.blue);
            diffG = (img->data[i].green - clusters[j].center.green) * (img->data[i].green - clusters[j].center.green);
            diffR = (img->data[i].red - clusters[j].center.red) * (img->data[i].red - clusters[j].center.red);
            
            tempTotalRGB = diffR + diffG + diffB;

            if (tempTotalRGB < totalRGB)
            {
                totalRGB = tempTotalRGB;
                nearest = j;
            }     
        }
        imag->data[i].red = clusters[nearest].center.red;
        imag->data[i].green = clusters[nearest].center.green;
        imag->data[i].blue = clusters[nearest].center.blue;       
    }

    //Finally, return the quantized image object to main
    return imag;
}

//Function to compute the Mean Squared Error
double computeError(PPMImage *image1, PPMImage *image2)
{
    //First some variables
    int i = 0;
    double err;
    double diffR;
    double diffG;
    double diffB;
    double total = 0;
    int size = image1->width * image1->height;

    //Now loop through every pixel in both images and save the difference for pixel values
    while (i < size)
    {
        //First take differences
        diffR = image1->data[i].red - image2->data[i].red;
        diffG = image1->data[i].green - image2->data[i].green;
        diffB = image1->data[i].blue - image2->data[i].blue;

        //Now square those numbers (which will also turn all negatives into positives)
        diffR = diffR * diffR;
        diffG = diffG * diffG;
        diffB = diffB * diffB;

        //Add these up into a total variable
        total += (diffR + diffG + diffB);

        //Increment i
        i++;
    }

    //Compute Mean Squared Error by dividing total by size
    err = total / size;

    //Now return the error to main
    return err;
}

//Main function
int main() {
    //Create a new image object and read the image in
    PPMImage *image;
    image = readPPM("sample.ppm");

    //Random number generator (for selecting random centers)
    srand(time(NULL));

    //Organize the pixels into clusters
    PPMImage *image2;
    image2 = macqueenClustering(image, 64);

    //Create a new image based on the color quantization
    writePPM("new.ppm", image2);

    //Next, compute the Mean Squared Error and print to console
    double err = computeError(image, image2);
    printf("%f\n", err);

    //Finally, free up all memory allocated in the program (that hasn't already been freed)
    free(image->data);
    free(image2->data);
    free(image);
    free(image2);
    
    //Wait for user response
    printf("Press any key...");
    getchar();
}
