#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

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

#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

static unsigned long mt[N]; /* the array for the state vector  */
static int mti=N+1; /* mti==N+1 means mt[N] is not initialized */

/* initializes mt[N] with a seed */
void init_genrand(unsigned long s)
{
    mt[0]= s & 0xffffffffUL;
    for (mti=1; mti<N; mti++) {
        mt[mti] = 
	    (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti); 
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
        mt[mti] &= 0xffffffffUL;
        /* for >32 bit machines */
    }
}

unsigned long genrand_int32(void)
{
    unsigned long y;
    static unsigned long mag01[2]={0x0UL, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    if (mti >= N) { /* generate N words at one time */
        int kk;

        if (mti == N+1)   /* if init_genrand() has not been called, */
            init_genrand(5489UL); /* a default initial seed is used */

        for (kk=0;kk<N-M;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (;kk<N-1;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
        mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        mti = 0;
    }
  
    y = mt[mti++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}

double genrand_real2(void)
{
    return genrand_int32()*(1.0/4294967296.0); 
    /* divided by 2^32 */
}

//Funcion that will read image information in
static PPMImage *readPPM(const char *filename)
{
    //Variable declarations
    char buff[16];
    PPMImage *img;
    FILE *fp;
    int c, rgb_comp_color;
    unsigned char *inBuf;
    int i;
    double conv;
    int imgSize;

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
    inBuf = malloc(sizeof(unsigned char));
    i = 0;
    imgSize = (img->height * img->width);
    
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
    //Variable Declaration
    FILE *fp;
    unsigned char r;
    unsigned char g;
    unsigned char b;
    int i = 0; 
    int length;

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

    i = 0; 
    length = (img->height * img->width);

    while (i < length)
    {
        r = (unsigned char)(img->data[i].red);
        g = (unsigned char)(img->data[i].green);
        b = (unsigned char)(img->data[i].blue);

        fwrite(&r, sizeof(unsigned char), 1, fp);
        fwrite(&g, sizeof(unsigned char), 1, fp);
        fwrite(&b, sizeof(unsigned char), 1, fp);
        i++;
    }
    fclose(fp);
}

//Helper function for min-max clustering
double min(PPMCluster *a, PPMPixel b, int size)
{
    double delta;
    PPMCluster *cluster;
    double tempTotalRGB;
    double totalRGB = 100000000000;

    for (int i = 0; i < size; i++)
    {
        cluster = &a[i];
        delta = b.red - cluster->center.red;
        tempTotalRGB = (delta * delta);
        delta = b.green - cluster->center.green;
        tempTotalRGB += (delta * delta);
        delta = b.blue - cluster->center.blue;
        tempTotalRGB += (delta * delta);

        if (tempTotalRGB < totalRGB)
        {
            totalRGB = tempTotalRGB;
        }
    }

    return totalRGB;
}

PPMImage* macqueenClustering(PPMImage *img, int numColors)
{
    //Variable Declarations/initializations
    int numPixels = (img->height * img->width);
    PPMCluster *clusters;
    int randomNumber;
    int numFixedColors;
    int numPass = 1;
    int terminate = numPixels * numPass; //terminates after iterating over every pixel in the image
    int randPixNum;
    int index = 0;
    PPMPixel closest;
    double delta = 0.0;
    double totalRGB = 0.00;
    int nearest;
    double tempTotalRGB;
    int counter = 0;
    int old_size;
    int new_size;
    PPMCluster *cluster;
    PPMPixel pixel;
    PPMImage *imag;
    PPMPixel *newPixel;
    int tempIteration = 0;
    int total = 1000000000; //A really high number
    double distance = 0.0;
    double tempDistance = 0.0;
    double red = 0.0;
    double green = 0.0;
    double blue = 0.0;

    //Filling an array with random numbers for number of colors to be quantized down to
    numFixedColors = 32;

    //Make an array of clusters
    clusters = malloc(numFixedColors * sizeof(*clusters));

    //Initialize each center to a random value and give each cluster a size of 1 -- RANDOM METHOD OF INITIALIZATION
    /*
    for (int i = 0; i < numFixedColors; i++)
    {
        randPixNum = (int) (rand() / (RAND_MAX + 1.0) * numPixels);
        cluster = &clusters[i];
        pixel = img->data[randPixNum];
        cluster->center.red = pixel.red;
        cluster->center.green = pixel.green;
        cluster->center.blue = pixel.blue;
        cluster->size = 1;
    }
    */
   
    //Initialize each cluster with min-max initialization

    //Next elements chosen based on min-max approach
    //Large loop is done one time for each cluster
    for (int i = 0; i < numFixedColors; i++)
    {
        //First cluster chosen by taking an average of all pixels
        if (i == 0)
        {
            cluster = &clusters[0];
            //Loop through every pixel to get an average
            for (int j = 0; j < numPixels; j++)
            {
                pixel = img->data[j];
                red += pixel.red;
                green += pixel.green;
                blue += pixel.blue;
            }
            red = red / numPixels;
            green = green / numPixels;
            blue = blue / numPixels;

            cluster->center.red = red;
            cluster->center.green = green;
            cluster->center.blue = blue;
            cluster->size = 1;
        }
        else
        {
            //All other clusters chosen using min-max method
            //Loop through every pixel and find the min-max of it
            for (int j = 0; j < numPixels; j++)
            {
                pixel = img->data[j];
                tempDistance = min(clusters, pixel, i);
                if (tempDistance > distance)
                {
                    distance = tempDistance;
                    tempIteration = j;
                }
            }

            //Choose the greatest of these to be the next cluster center
            cluster = &clusters[i];
            pixel = img->data[tempIteration];
            cluster->center.red = pixel.red;
            cluster->center.green = pixel.green;
            cluster->center.blue = pixel.blue;
            cluster->size = 1;
            distance = 0.0;
        }
    }
/*
    //Now print the centers
    for (int i = 0; i < numFixedColors; i++)
    {
        cluster = &clusters[i];
        double re = cluster->center.red;
        double gr = cluster->center.green;
        double bl = cluster->center.blue;

        printf("(");
        printf("%f", re); printf(",");
        printf("%f", gr); printf(",");
        printf("%f", bl);
        printf(")");
    }
*/
    //Now time for data clustering using k-means
    //Terminate when criteria is met
    for (index = 0; index < terminate; index++)
     {
        //Now, select a random pixel from the pixel array (pick a random pixel from the image)
        randPixNum = (int) (genrand_real2() * numPixels);

        //Set totalRGB to be the highest it can be
        totalRGB = 195075.00; // 3 * 255 * 255 

        //Random pixel
        pixel = img->data[randPixNum];

        //Next, find the closest pixel to this pixel in the centers array
        for (int i = 0; i < numFixedColors; i++) {
            cluster = &clusters[i];
            delta = pixel.red - cluster->center.red;
            tempTotalRGB = (delta * delta);
            delta = pixel.green - cluster->center.green;
            tempTotalRGB += (delta * delta);
            delta = pixel.blue - cluster->center.blue;
            tempTotalRGB += (delta * delta);

            if (tempTotalRGB < totalRGB)
            {
                totalRGB = tempTotalRGB;
		        nearest = i;
            } 
        }

        //Update the center in the centers array ci = (Ni ci + xr)/(Ni + 1)
        cluster = &clusters[nearest];
        old_size = cluster->size;
        new_size = old_size + 1;
        
        cluster->center.red = ((old_size * cluster->center.red) + pixel.red ) / (double) new_size;
        cluster->center.green = ((old_size * cluster->center.green) + pixel.green ) / (double) new_size;
        cluster->center.blue = ((old_size * cluster->center.blue) + pixel.blue ) / (double) new_size;
        cluster->size = new_size;
    }

    //Create new image object
    imag = (PPMImage *)malloc(sizeof(PPMImage));
    imag->data = (PPMPixel*)malloc(img->width * img->height * sizeof(PPMPixel));
    imag->width = img->width;
    imag->height = img->height;

    //Now quantize the image
    for (int i = 0; i < numPixels; i++)
    {
        totalRGB = 195075.00;
        pixel = img->data[i];
        
        for (int j = 0; j < numFixedColors; j++) 
        {
            cluster = &clusters[j];
            delta = pixel.red - cluster->center.red;
            tempTotalRGB = (delta * delta);
            delta = pixel.green - cluster->center.green;
            tempTotalRGB += (delta * delta);
            delta = pixel.blue - cluster->center.blue;
            tempTotalRGB += (delta * delta);

            if (tempTotalRGB < totalRGB)
            {
                totalRGB = tempTotalRGB;
                nearest = j;
            }     
        }

        newPixel = &imag->data[i];
        pixel = clusters[nearest].center;
        newPixel->red = pixel.red;
        newPixel->green = pixel.green;
        newPixel->blue = pixel.blue;       
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
    double delta;
    double total = 0;
    int size = image1->width * image1->height;

    //Now loop through every pixel in both images and save the difference for pixel values
    for (i = 0; i < size; i++)
    {
        //First take differences
        delta = image1->data[i].red - image2->data[i].red;
        total += (delta * delta);
        delta = image1->data[i].green - image2->data[i].green;
        total += (delta * delta);
        delta = image1->data[i].blue - image2->data[i].blue;
        total += (delta * delta);
    }

    //Compute Mean Squared Error by dividing total by size
    err = total / size;

    //Now return the error to main
    return err;
}

//Main function
int main() {
    //Some variables
    PPMImage *image;
    PPMImage *image2;
    double err;
    struct timeval  tv1, tv2;

    //Start timer
    gettimeofday(&tv1, NULL);

    //Create a new image object and read the image in
    image = readPPM("sample4.ppm");

    //Random number generator (for selecting random centers)
    init_genrand(4357U);

    //Organize the pixels into clusters
    image2 = macqueenClustering(image, 64);

    //Create a new image based on the color quantization
    writePPM("new.ppm", image2);

    //Next, compute the Mean Squared Error and print to console
    err = computeError(image, image2);
    printf("%f\n", err);

    //Calculate time and print to console
    gettimeofday(&tv2, NULL);

    printf ("Total time = %f seconds\n",
            (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
            (double) (tv2.tv_sec - tv1.tv_sec));

    //Finally, free up all memory allocated in the program (that hasn't already been freed)
    free(image->data);
    free(image2->data);
    free(image);
    free(image2);

    //Wait for user response
    printf("Press any key...");
    getchar();
}
