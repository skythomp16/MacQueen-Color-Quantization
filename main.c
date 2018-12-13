#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

/*
1) Number of images (8) <-- images in a file
2) Number of colors (4): 32, 64, 128, 256 <-- easy to implement
3) Initialization (2): maximin, k-means++
4) Presentation order (2): pseudo-random, quasi-random
5) Learning rate (6): 0.5, 0.6, 0.7, 0.8, 0.9, 1.0
6) Number of passes (4): 0.25, 0.5, 0.75, 1.0

So, total 8 x 4 x 2 x 2 x 6 x 4 = 3,072

1) K-means++: You should be able to plug-in my code easily. You just have to make sure that it gives at least as good results as maximin. Actually, k-means++ usually gives better.

2) Drawing uniform random integers from a certain range: That code I sent you should work. You just have to test it in a separate program to make sure. 
Pluging in a randomized code that is untested is a recipe for disaster. You had that problem earlier, which was very difficult to identify.

3) Pseudorandom sampling: This is quite tricky. The algorithm is short, but complex (it requires bit operations because it has to be efficient). 
I will give you my copy (which is adapted from a famous book). Solve issues (1) and (2) first and then we will deal with issue (3).

4) Writing an experimental driver (main). It will have a 6-level nested loop, which I'm sure you have never written before (the most I've done was 4-level). 
But, each level has only a few iterations, so it's not a problem.

5) Running experiments

6) Writing a paper
*/

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

/* fix for sobol ulong error */
typedef unsigned long ulong;

#define CREATOR "Rewritten for quantization"
#define RGB_COMPONENT_COLOR 255
#define DIST_MAX 195075

#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

#define MAXBIT 30

static unsigned long mt[N]; /* the array for the state vector  */
static int mti=N+1; /* mti==N+1 means mt[N] is not initialized */

//public variables
double r = 0.0;
double g = 0.0;
double b = 0.0;

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

/* Function for generating a bounded random integer between 0 and range  */
uint32_t bounded_rand(uint32_t range) {
    uint32_t x = genrand_int32();  
    uint64_t m = ( ( uint64_t ) x ) * ( ( uint64_t ) range );
    uint32_t l = ( uint32_t ) m;
    if (l < range) {
        uint32_t t = -range;
        if (t >= range) {
            t -= range;
            if (t >= range) 
                t %= range;
        }
        while (l < t) {
            x = genrand_int32();  
            m = ( ( uint64_t ) x ) * ( ( uint64_t ) range );
            l = ( uint32_t ) m;
        }
    }
    return m >> 32;
}

/* Returns two quasi-random numbers for a 2-dimensional Sobol
   sequence. Adapted from Numerical Recipies in C. */

/* X and Y will be in [0,1] range */
void sobseq ( double *x, double *y ) 
{
 int j, k, l;
 ulong i, im, ipp;

 /* The following variables are "static" since we want their
   values to remain stored after the function returns. These
   values represent the state of the quasi-random number generator. */

 static double fac;
 static int init = 0;
 static ulong ix1, ix2;
 static ulong in, *iu[2 * MAXBIT + 1];
 static ulong mdeg[3] = { 0, 1, 2 };
 static ulong ip[3] = { 0, 0, 1 };
 static ulong iv[2 * MAXBIT + 1] =
     { 0, 1, 1, 1, 1, 1, 1, 3, 1, 3, 3, 1, 1, 5, 7, 7, 3, 3, 5, 15, 11, 5, 15, 13, 9 };

 /* Initialise the generator the first time the function is called */

 if ( !init ) 
  {
   init = 1;
   for ( j = 1, k = 0; j <= MAXBIT; j++, k += 2 ) 
    { 
     iu[j] = &iv[k]; 
    }
    
   for ( k = 1; k <= 2; k++ ) 
    {
     for ( j = 1; j <= mdeg[k]; j++ ) 
      { 
       iu[j][k] <<= ( MAXBIT - j ); 
      }

     for ( j = mdeg[k] + 1; j <= MAXBIT; j++ ) 
      {
       ipp = ip[k];
       i = iu[j - mdeg[k]][k];
       i ^= ( i >> mdeg[k] );

       for ( l = mdeg[k] - 1; l >= 1; l-- )
        {
	 if ( ipp & 1 ) 
	  { 
	   i ^= iu[j - l][k]; 
	  } 

	 ipp >>= 1;
	}

       iu[j][k] = i;
      }
    }

   fac = 1.0 / ( 1L << MAXBIT );
   in = 0;
  }

 /* Now calculate the next pair of numbers in the 2-D Sobol sequence */

 im = in;
 for ( j = 1; j <= MAXBIT; j++ ) 
  {
   if ( !( im & 1 ) ) 
    { 
     break; 
    }

   im >>= 1;
  }

 im = ( j - 1 ) * 2;
 *x = ( ix1 ^= iv[im + 1] ) * fac;
 *y = ( ix2 ^= iv[im + 2] ) * fac;
 
 in++;
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
    int numPixels;

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
        //Read in pixels using buffer and calculate center mass
        conv = *inBuf;
        img->data[i].red = conv;
        r += conv;
        fread(inBuf, 1, 1, fp);
        conv = *inBuf;
        img->data[i].green = conv;
        g += conv;
        fread(inBuf, 1, 1, fp);
        conv = *inBuf;
        img->data[i].blue = conv;
        b += conv;
        i++;
    }

    r = r / imgSize;
    g = g / imgSize;
    b = b / imgSize;

    fclose(fp);
    return img;
}

//Function to write the new image after color quantization
void writePPM(const char *filename, PPMImage *img)
{
    //Variable Declaration
    FILE *fp;
    unsigned char re;
    unsigned char gr;
    unsigned char bl;
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
        re = (unsigned char)(img->data[i].red);
        gr = (unsigned char)(img->data[i].green);
        bl = (unsigned char)(img->data[i].blue);

        fwrite(&re, sizeof(unsigned char), 1, fp);
        fwrite(&gr, sizeof(unsigned char), 1, fp);
        fwrite(&bl, sizeof(unsigned char), 1, fp);
        i++;
    }
    fclose(fp);
}

//Helper function for min-max clustering
double min(PPMCluster *c, PPMPixel d, int size)
{
    double delta;
    PPMCluster *cluster;
    double tempTotalRGB;
    double totalRGB = DIST_MAX;

    for (int i = 0; i < size; i++)
    {
        cluster = &c[i];
        delta = d.red - cluster->center.red;
        tempTotalRGB = (delta * delta);
        delta = d.green - cluster->center.green;
        tempTotalRGB += (delta * delta);
        delta = d.blue - cluster->center.blue;
        tempTotalRGB += (delta * delta);

        if (tempTotalRGB < totalRGB)
        {
            totalRGB = tempTotalRGB;
        }
    }

    return totalRGB;
}

//Maximin function for initialization
void Maximin(PPMImage *img, PPMCluster* clusters, int numColors)
{
    //Some variables
    PPMCluster *cluster;
    double delta1;
    double delta2;
    double dist;
    double dmax;
    int initCounter = 0;
    double* dj;
    PPMPixel pixel;
    int tempIteration = 0;
    int numPixels = (img->height * img->width);
    dj = malloc(numPixels * sizeof (double));

    //Initialize first cluster
    cluster = &clusters[0];
    cluster->center.red = r;
    cluster->center.green = g;
    cluster->center.blue = b;
    cluster->size = 1;

    for (int j = 0; j < numPixels; j++)
    {
        dj[j] = DIST_MAX;
    }

    //Next elements chosen based on min-max approach
    //Large loop is done one time for each cluster
    for (int i = 0 + 1; i < numColors; i++)
    {
        dmax = -DIST_MAX;
        //All other clusters chosen using min-max method
        //Loop through every pixel and find the min-max of it
        for (int j = 0; j < numPixels; j++)
        {
            //Cache the current pixel
            pixel = img->data[j];

            //Find distance(xj, ci - 1)
            cluster = &clusters[i - 1];
            delta1 = pixel.red - cluster->center.red;
            dist = (delta1 * delta1);
            delta1 = pixel.green - cluster->center.green;
            dist += (delta1 * delta1);
            delta1 = pixel.blue - cluster->center.blue;
            dist += (delta1 * delta1);

            //See if dist < dj
            if (dist < dj[j])
            {
                dj[j] = dist;
            }

            if (dmax < dj[j])
            {
                dmax = dj[j];
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
    }
}

//K-means++ function for cluster initialization
void K_Means_Plus_Plus(PPMImage *img, PPMCluster* clusters, int numFixedColors)
{
int ic, ip;
 size_t num_bytes;
 double dist;
 double sse = 0.0;
 double rand_val;
 double *dist_to_center;
 int num_points = img->height * img->width;
 int num_dims = 3;
 double delta = 0.0;
 double tempTotalRGB = 0.0;
 PPMPixel pixel;
 int randPixNum = 0;
 PPMCluster *cluster;
 PPMCluster *cluster2;

 
 num_bytes = num_dims * sizeof ( double );

  dist_to_center = ( double * ) malloc ( num_points * sizeof ( double ) );

     /* First center is a randomly picked point */
 //memcpy ( img->data[0], img->data[bounded_rand ( num_points )], num_bytes );
    randPixNum = (int) (genrand_real2() * num_points);
    cluster = &clusters[0];
    cluster->center = img->data[randPixNum];
     /* Calculate the current SSE */
 for ( ip = 0; ip < num_points; ip++ )
  {
   //sse += ( dist_to_center[ip] = calc_sqr_euc_dist ( data_set->data[ip], curr_center, num_dims ) );
    pixel = img->data[ip];
    delta = pixel.red - cluster->center.red;
    tempTotalRGB = (delta * delta);
    delta = pixel.green - cluster->center.green;
    tempTotalRGB += (delta * delta);
    delta = pixel.blue - cluster->center.blue;
    tempTotalRGB += (delta * delta);
    sse = ( dist_to_center[ip] = tempTotalRGB );
  }

 /* Choose the remaining ( NUM_CLUSTERS - 1 ) centers */
 for ( ic = 0 + 1; ic < numFixedColors; ic++ )
  {
   rand_val = genrand_real2 ( ) * sse;

   /* Select a point with a probability proportional to its contribution to SSE */
   for ( ip = 0; ip < num_points - 1; ip++ ) 
    {
     if ( rand_val <= dist_to_center[ip] )
      {
       break;
      }
     else
      {
       rand_val -= dist_to_center[ip];
      }
    }

   /* Assign the randomly picked point to the current center */
   cluster = &clusters[ip];
   for ( ip = 0; ip < num_points; ip++ )
    {
    pixel = img->data[ip];
    delta = pixel.red - cluster->center.red;
    tempTotalRGB = (delta * delta);
    delta = pixel.green - cluster->center.green;
    tempTotalRGB += (delta * delta);
    delta = pixel.blue - cluster->center.blue;
    tempTotalRGB += (delta * delta);
    dist = tempTotalRGB;
     //dist = calc_sqr_euc_dist ( data_set->data[ip], curr_center, num_dims );
     
     /* Current center is closer to this point */
     /* Update the SSE and the nearest-center distance */
     
     if ( dist < dist_to_center[ip] ) 
      {
       sse -= ( dist_to_center[ip] - dist );
       dist_to_center[ip] = dist;
      }
      
    }

   /* Assign the new center its value */
   //memcpy ( img->data[ic], curr_center, num_bytes );
   cluster2 = &clusters[ic];
   cluster2->center = cluster->center;
  }

 #ifdef FREE_MEM
 free ( dist_to_center );
 #endif

 //return center;
}

//Data clustering function where all of the magic happens
PPMImage* cluster(PPMImage *img, int numColors, int init, double p_val, double numPass, int presOrder)
{
    //Variable Declarations/initializations
    int numPixels = (img->height * img->width);
    PPMCluster *clusters;
    int randomNumber;
    int numFixedColors;
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
    double tempTotalDist = 0.0;
    double sobx;
    double soby;
    int num_rows = img->height;
    int num_cols = img->width;
    int row_index;
    int col_index;

    //Allocate memory for an array of clusters
    clusters = malloc(numColors * sizeof(*clusters));

    //Initialize clusters based on input from user
    if (init == 0)
    {
        //Call maximin function to initialize clusters
        Maximin(img, clusters, numColors);
    }
    else if (init == 1)
    {
        //Call k-means++ function to initialize that way
        K_Means_Plus_Plus(img, clusters, numFixedColors);
    }

    //Now time for data clustering using k-means
    //Terminate when criteria is met
    for (index = 0; index < terminate; index++)
     {
        
        //Choose a random number (quasi vs pseudo determined by user)
        if (presOrder == 0)
        {
            //Quasirandom
            sobseq ( &sobx, &soby );

            row_index = ( int ) ( soby * num_rows + 0.5 ); /* round */
            if ( row_index == num_rows )
            {
            row_index--;
            }

            col_index = ( int ) ( sobx * num_cols + 0.5 ); /* round */
            if ( col_index == num_cols )
            {
            col_index--;
            }

            randPixNum = ( row_index * num_cols + col_index );
        }
        else {
            //Pseudorandom
            randPixNum = (int) (genrand_real2() * numPixels);
        }

        //Set totalRGB to be the highest it can be
        totalRGB = DIST_MAX; // 3 * 255 * 255 

        //Random pixel
        pixel = img->data[randPixNum];

        //Next, find the closest pixel to this pixel in the centers array
        for (int i = 0; i < numColors; i++) {
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

       double rate = pow ( new_size, -p_val );
       cluster->center.red = rate * pixel.red + ( 1 - rate ) * cluster->center.red;
       cluster->center.green = rate * pixel.green + ( 1 - rate ) * cluster->center.green;
       cluster->center.blue = rate * pixel.blue + ( 1 - rate ) * cluster->center.blue;
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
        totalRGB = DIST_MAX;
        pixel = img->data[i];
        
        for (int j = 0; j < numColors; j++) 
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


void tableGen()
{
    //Some variables
    PPMImage* img;
    PPMImage* img2;
    double err;
    struct timeval  tv1, tv2;
    char *filenames[8] = {"baboon.ppm", "fish.ppm", "girl.ppm", "goldhill.ppm", "kodim05.ppm", "kodim23.ppm", "peppers.ppm", "pills.ppm"};
    const int numColors[4] = {32, 64, 128, 256};
    const int inits[2] = {0, 1}; //Maximin followed by k-means++
    const int pres[2] = {0, 1}; //Quasirandom followed by Pseudorandom
    const double learning[6] = {0.5, 0.6, 0.7, 0.8, 0.9, 1.0};
    const double passes[4] = {0.25, 0.5, 0.75, 1.0};
    char* filename;
    int numColor;
    int init;
    int present;
    double learn;
    double pass;

    //Random number generator (for selecting random centers)
    init_genrand(4357U);

    //Now generate table headers for the new table
    printf("Filename"); printf("\t"); printf("Num_Colors"); printf("\t");
    printf("Initialize"); printf("\t"); printf("Presentation"); printf("\t");
    printf("Learning"); printf("\t"); printf("Passes"); printf("\t");
    printf("MSE"); printf("\n");

    //Start the timer
    gettimeofday(&tv1, NULL);

    //Create a 6-nested loop with 3,072 combinations for printing a table
    //Number of images
    for (int i = 0; i < 8; i++)
    {
        //Read in the image with the specified filename
        filename = filenames[i];
        img = readPPM(filename);

        //Now number of colors
        for (int b = 0; b < 4; b++)
        {
            numColor = numColors[b];
            //Initialization
            for (int c = 0; c < 2; c++)
            {
                init = inits[c];
                //Presentation
                for (int d = 0; d < 2; d++)
                {
                    present = pres[d];
                    //Learning
                    for (int e = 0; e < 6; e++)
                    {
                        learn = learning[e];
                        //Passes
                        for (int f = 0; f < 4; f++)
                        {
                            pass = passes[f];

                            //Deep within the loop, do the clustering of the specified options
                            img2 = cluster(img, numColor, init, learn, pass, present);

                            //Calcule the MSE and save off into a variable
                            err = computeError(img, img2);

                            //Now, print out the information to the table
                            printf("%s", filename); printf("\t"); printf("%d", numColor); printf("\t");
                            printf("%d", init); printf("\t"); printf("%d", present); printf("\t");
                            printf("%f", learn); printf("\t"); printf("%f", pass); printf("\t"); printf("%f", err);
                            
                            //Now new line
                            printf("\n");
                        }
                    }
                }
            }
        }
    }

    //Calculate time and print to console
    gettimeofday(&tv2, NULL);
    printf ("Total time = %f seconds\n",
    (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
    (double) (tv2.tv_sec - tv1.tv_sec));

}

//Main function
int main(int argc, char *argv[]) {
    //Some variables
    PPMImage *image;
    PPMImage *image2;
    double err;
    struct timeval  tv1, tv2;
    const char* filename = argv[1];
    int num_colors = atoi(argv[2]);
    int init = atoi(argv[3]); //0 for maximin and 1 for k-means++
    int presOrder = atoi(argv[4]); //0 for quasirandom and 1 for pseudorandom
    double learnRate = atof(argv[5]); //p_val
    double numPass = atof(argv[6]); //Can be partial hence the double data type
    char* outputfilename;

    //Start timer
    gettimeofday(&tv1, NULL);

    //Create a new image object and read the image in
    image = readPPM(filename);

    //Random seed based on the time since Jan. 1 1970
    init_genrand(time(NULL));

    //Organize the pixels into clusters
    image2 = cluster(image, num_colors, init, learnRate, numPass, presOrder);

    //Create a new image based on the color quantization
    outputfilename = strtok(argv[2], ".");
    outputfilename = strcat(outputfilename, "_");
    outputfilename = strcat(outputfilename, argv[1]);
    outputfilename = strcat(outputfilename, ".ppm");
    writePPM(outputfilename, image2);

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
