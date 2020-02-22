/* 
  To compile:
  g++ -O3 -o mkm mkm.c -lm

  For a list of command line options: ./mkm
 */

/* BEGIN: Copyright notice for the Mersenne Twister implementation */

/* 
   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.

   Before using, initialize the state by using init_genrand(seed)  
   or init_by_array(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.                          

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote 
        products derived from this software without specific prior written 
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


   Any feedback is very welcome.
   http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
   email: m-mat @ math.sci.hiroshima-u.ac.jp (remove space)
*/

/* END: Copyright notice for the Mersenne Twister implementation */

#include <chrono>
#include <climits>
#include <iostream>
#include <math.h>
#include <string.h>

#define PRINT_TIME_INIT
#define PRINT_TIME_CLUST
/*
#define PRINT_TIME_MAP
#define PRINT_TIME_TOTAL
*/
#define PRINT_MSE
#define PRINT_ITER

using namespace std::chrono;

typedef unsigned char uchar;
typedef unsigned long ulong;

typedef struct 
 {
  double red, green, blue;
 } RGB_Pixel;

typedef struct 
 {
  int size;
  RGB_Pixel center;
 } RGB_Cluster;

typedef struct 
 {
  int width, height;
  int size;
  RGB_Pixel *data;
 } RGB_Image;

/* Maximum possible RGB distance = 3 * 255 * 255 */
#define MAX_RGB_DIST 195075 

/* Mersenne Twister related constants */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

#define MAXBIT 30

static ulong mt[N]; /* the array for the state vector  */
static int mti = N + 1; /* mti == N + 1 means mt[N] is not initialized */

/* initializes mt[N] with a seed */
void 
init_genrand ( ulong s )
{
 mt[0]= s & 0xffffffffUL;
 for ( mti = 1; mti < N; mti++ ) 
  {
   mt[mti] = 
	     ( 1812433253UL * ( mt[mti - 1] ^ ( mt[mti - 1] >> 30 ) ) + mti ); 
   /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
   /* In the previous versions, MSBs of the seed affect   */
   /* only MSBs of the array mt[].                        */
   /* 2002/01/09 modified by Makoto Matsumoto             */
   mt[mti] &= 0xffffffffUL;
   /* for >32 bit machines */
  }
}

ulong 
genrand_int32 ( void )
{
 ulong y;
 static ulong mag01[2] = { 0x0UL, MATRIX_A };
 /* mag01[x] = x * MATRIX_A  for x = 0, 1 */

 if ( mti >= N ) 
  { /* generate N words at one time */
   int kk;

   if ( mti == N + 1 ) 
    {
     /* if init_genrand ( ) has not been called, */
     init_genrand ( 5489UL ); /* a default initial seed is used */
    }

   for ( kk = 0; kk < N - M; kk++ ) 
    {
     y = ( mt[kk] & UPPER_MASK )|( mt[kk + 1] & LOWER_MASK );
     mt[kk] = mt[kk+M] ^ ( y >> 1 ) ^ mag01[y & 0x1UL];
    }
   
   for ( ; kk < N - 1; kk++ ) 
    {
     y = ( mt[kk] & UPPER_MASK )|( mt[kk + 1] & LOWER_MASK );
     mt[kk] = mt[kk + ( M - N )] ^ ( y >> 1 ) ^ mag01[y & 0x1UL];
    }
    
   y = ( mt[N - 1] & UPPER_MASK )|( mt[0] & LOWER_MASK );
   mt[N - 1] = mt[M - 1] ^ ( y >> 1 ) ^ mag01[y & 0x1UL];
   mti = 0;
  }
  
 y = mt[mti++];

 /* Tempering */
 y ^= ( y >> 11 );
 y ^= ( y << 7 ) & 0x9d2c5680UL;
 y ^= ( y << 15 ) & 0xefc60000UL;
 y ^= ( y >> 18 );

 return y;
}

double 
genrand_real2 ( void )
{
 return genrand_int32 ( ) * ( 1.0 / 4294967296.0 );
 /* divided by 2^32 */
}

/* Function for generating a bounded random integer between 0 and RANGE */
/* Source: http://www.pcg-random.org/posts/bounded-rands.html */

uint32_t 
bounded_rand ( const uint32_t range ) 
{
 uint32_t x = genrand_int32 ( );  
 uint64_t m = ( ( uint64_t ) x ) * ( ( uint64_t ) range );
 uint32_t l = ( uint32_t ) m;

 if ( l < range ) 
 {
  uint32_t t = -range;

  if ( t >= range ) 
   {
    t -= range;
    if ( t >= range ) 
     {
      t %= range;
     }
   }
  
  while ( l < t ) 
   {
    x = genrand_int32 ( );  
    m = ( ( uint64_t ) x ) * ( ( uint64_t ) range );
    l = ( uint32_t ) m;
   }
 }
 
 return m >> 32;
}

/* 
  Returns two quasirandom numbers from a 2D Sobol
  sequence. Adapted from Numerical Recipies in C. 
 */

void 
sob_seq ( double *x, double *y ) 
{
 int j, k, l;
 ulong i, im, ipp;

 /* 
   The following variables are static since we want their
   values to remain stored after the function returns. These
   values represent the state of the quasirandom number generator. 
  */

 static double fac;
 static int init = 0;
 static ulong ix1, ix2;
 static ulong in, *iu[2 * MAXBIT + 1];
 static ulong mdeg[3] = { 0, 1, 2 };
 static ulong ip[3] = { 0, 0, 1 };
 static ulong iv[2 * MAXBIT + 1] =
     { 0, 1, 1, 1, 1, 1, 1, 3, 1, 3, 3, 1, 1, 5, 7, 7, 3, 3, 5, 15, 11, 5, 15, 13, 9 };

 /* Initialize the generator the first time the function is called */
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

 /* X and Y will fall in [0,1] */
}

RGB_Image *
read_PPM ( const char *filename, RGB_Pixel *mean )
{
 uchar byte;
 char buff[16];
 int c, max_rgb_val, i = 0;
 FILE *fp;
 RGB_Pixel *pixel;
 RGB_Image *img;

 fp = fopen(filename, "rb");
 if ( !fp ) 
 {
  fprintf ( stderr, "Unable to open file '%s'!\n", filename );
  exit ( EXIT_FAILURE );
 }

 /* read image format */
 if ( !fgets ( buff, sizeof ( buff ), fp ) ) 
  {
   perror ( filename );
   exit ( EXIT_FAILURE );
  }

 /*check the image format to make sure that it is binary */
 if ( buff[0] != 'P' || buff[1] != '6' ) 
  {
   fprintf ( stderr, "Invalid image format (must be 'P6')!\n" );
   exit ( EXIT_FAILURE );
  }

 img = ( RGB_Image * ) malloc ( sizeof ( RGB_Image ) );
 if ( !img ) 
  {
   fprintf ( stderr, "Unable to allocate memory!\n" );
   exit ( EXIT_FAILURE );
  }

 /* skip comments */
 c = getc(fp);
 while ( c == '#' ) 
  {
   while ( getc(fp) != '\n' );
   c = getc ( fp );
  }

 ungetc ( c, fp );

 /* read image dimensions */
 if (fscanf(fp, "%u %u", &img->width, &img->height) != 2) 
  {
   fprintf ( stderr, "Invalid image dimensions ('%s')!\n", filename );
   exit ( EXIT_FAILURE );
  }

 /* read maximum component value */
 if ( fscanf ( fp, "%d", &max_rgb_val ) != 1 ) 
  {
   fprintf ( stderr, "Invalid maximum R, G, B value ('%s')!\n", filename );
   exit ( EXIT_FAILURE );
  }

 /* validate maximum component value */
 if ( max_rgb_val != 255 ) 
  {
   fprintf ( stderr, "'%s' is not a 24-bit image!\n", filename );
   exit ( EXIT_FAILURE );
  }

 while ( fgetc(fp) != '\n' );

 /* allocate memory for pixel data */
 img->size = img->height * img->width;
 img->data = ( RGB_Pixel * ) malloc ( img->size * sizeof ( RGB_Pixel ) );

 if ( !img ) 
  {
   fprintf ( stderr, "Unable to allocate memory!\n");
   exit ( EXIT_FAILURE );
  }

 /* Read in pixels using buffer and calculate center of mass */
 mean->red = mean->green = mean->blue = 0.0;
 while ( fread ( &byte, 1, 1, fp ) && i < img->size )
  {
   pixel = &img->data[i];
   mean->red += ( pixel->red = byte );
   fread ( &byte, 1, 1, fp );
   mean->green += ( pixel->green = byte );
   fread ( &byte, 1, 1, fp );
   mean->blue += ( pixel->blue = byte );
   i++;
  }

 mean->red /= img->size;
 mean->green /= img->size;
 mean->blue /= img->size;

 fclose ( fp );

 return img;
}

void 
write_PPM ( const RGB_Image *img, const char *filename )
{
 uchar byte;
 FILE *fp;

 fp = fopen ( filename, "wb" );
 if ( !fp ) 
  {
   fprintf ( stderr, "Unable to open file '%s'!\n", filename );
   exit ( EXIT_FAILURE );
  }

 fprintf ( fp, "P6\n" );
 fprintf ( fp, "%d %d\n", img->width, img->height );
 fprintf ( fp, "%d\n", 255 );

 for ( int i = 0; i < img->size; i++ )
  {
   byte = ( uchar ) img->data[i].red;
   fwrite ( &byte, sizeof ( uchar ), 1, fp );
   byte = ( uchar ) img->data[i].green;
   fwrite ( &byte, sizeof ( uchar ), 1, fp );
   byte = ( uchar ) img->data[i].blue;
   fwrite ( &byte, sizeof ( uchar ), 1, fp );
  }

 fclose ( fp );
}

/* Maximin initialization method */
/* 
   For a comprehensive survey of k-means initialization methods, see
   M. E. Celebi, H. Kingravi, and P. A. Vela, 
   A Comparative Study of Efficient Initialization Methods for the K-Means Clustering Algorithm, 
   Expert Systems with Applications, vol. 40, no. 1, pp. 200–210, 2013.
 */

void 
maximin ( const RGB_Image *img, RGB_Cluster* clusters, const int num_colors, const RGB_Pixel *mean )
{
 int i, j, max_dist_index = 0;
 double delta_red, delta_green, delta_blue;
 double dist, max_dist;
 double *nc_dist;
 RGB_Pixel pixel;
 RGB_Cluster *cluster;

 nc_dist = ( double * ) malloc ( img->size * sizeof ( double ) );

 /* Initialize first center by the mean R, G, B color */
 cluster = &clusters[0];
 cluster->center.red = mean->red;
 cluster->center.green = mean->green;
 cluster->center.blue = mean->blue;
 cluster->size = 1;

 /* Initialize the nearest-center-distance for each pixel */
 for ( j = 0; j < img->size; j++ )
  {
   nc_dist[j] = MAX_RGB_DIST;
  }

 /* Choose the remaining centers using maximin */
 for ( i = 0 + 1; i < num_colors; i++ )
  {
   max_dist = -MAX_RGB_DIST;

   for ( j = 0; j < img->size; j++ )
    {
     /* Cache the pixel */
     pixel = img->data[j];

     /* Compute the pixel's distance to the previously chosen center */
     cluster = &clusters[i - 1];
     delta_red = pixel.red - cluster->center.red;
     delta_green = pixel.green - cluster->center.green;
     delta_blue = pixel.blue - cluster->center.blue;
     dist = delta_red * delta_red + delta_green * delta_green + delta_blue * delta_blue;

     if ( dist < nc_dist[j] )
      {
       /* Update the nearest-center-distance for this pixel */
       nc_dist[j] = dist;
      }

     if ( max_dist < nc_dist[j] )
      {
       /* Update the maximum nearest-center-distance so far */
       max_dist = nc_dist[j];
       max_dist_index = j;
      }
    }

   /* Pixel with maximum distance to its nearest center is chosen as a center */
   cluster = &clusters[i];
   pixel = img->data[max_dist_index];
   cluster->center.red = pixel.red;
   cluster->center.green = pixel.green;
   cluster->center.blue = pixel.blue;
   cluster->size = 1;
  }

 free ( nc_dist );
}

/* Color quantization using Macqueen's k-means algorithm */
/* 
  For detailed information, see
  S. Thompson, M. E. Celebi, and K. H. Buck, 
  Fast Color Quantization Using Macqueen’s K-Means Algorithm, 
  Journal of Real-Time Image Processing, to appear 
  (https://doi.org/10.1007/s11554-019-00914-6), 2020.
 */

RGB_Image* 
macqueen_cluster ( const RGB_Image *in_img, const int num_colors, const int pres_order, 
		   const double lr_exp, const double sample_rate, RGB_Pixel *mean )
{
 int i, j;
 int max_pres, min_dist_index;
 int row_index, col_index, rand_index;
 int old_size, new_size;
 double min_dist, dist;
 double delta_red, delta_green, delta_blue;
 double sob_x, sob_y;
 double rate;
 RGB_Cluster *clusters, *cluster;
 RGB_Pixel in_pix, *out_pix;
 RGB_Image *out_img;

 clusters = ( RGB_Cluster * ) malloc ( num_colors * sizeof ( RGB_Cluster ) );

 auto start = high_resolution_clock::now();
    
 /* Initialize cluster centers */
 maximin ( in_img, clusters, num_colors, mean );
    
 auto stop = high_resolution_clock::now ( );
 auto duration = duration_cast<microseconds> ( stop - start ); 
    
 #ifdef PRINT_TIME_INIT
 printf ( "Initialization time = %g\n", duration.count ( ) / 1e3 );
 #endif

 start = high_resolution_clock::now ( );

 /* Clustering pixel data using Macqueen's k-means algorithm */
 max_pres = in_img->size * sample_rate; 
 for ( i = 0; i < max_pres; i++ )
  {
   /* Choose a pixel quasi- or pseudo-randomly */
   if ( pres_order == 0 )
    {
     /* Quasirandom */
     sob_seq ( &sob_x, &sob_y );

     row_index = ( int ) ( sob_y * in_img->height + 0.5 ); /* round */
     if ( row_index == in_img->height )
      {
       row_index--;
      }

     col_index = ( int ) ( sob_x * in_img->width + 0.5 ); /* round */
     if ( col_index == in_img->width )
      {
       col_index--;
      }

     rand_index = row_index * in_img->width + col_index;
    }
   else 
    {
     /* Pseudorandom */
     /* rand_index = ( int ) ( genrand_real2 ( ) * in_img->size ); */
     rand_index = bounded_rand ( in_img->size );
    }
      
   /* Cache the chosen pixel */
   in_pix = in_img->data[rand_index];

   /* Find the nearest center */
   min_dist = MAX_RGB_DIST; 
   min_dist_index = -INT_MAX;
   for ( j = 0; j < num_colors; j++ ) 
    {
     cluster = &clusters[j];
     delta_red = in_pix.red - cluster->center.red;
     delta_green = in_pix.green - cluster->center.green;
     delta_blue = in_pix.blue - cluster->center.blue;
     dist = delta_red * delta_red + delta_green * delta_green + delta_blue * delta_blue;

     if ( dist < min_dist )
      {
       min_dist = dist;
       min_dist_index = j;
      }  
    }

   /* Update the nearest center */
   cluster = &clusters[min_dist_index];
   old_size = cluster->size;
   new_size = old_size + 1;
   rate = pow ( new_size, -lr_exp );
   cluster->center.red += rate * ( in_pix.red - cluster->center.red );
   cluster->center.green += rate * ( in_pix.green - cluster->center.green );
   cluster->center.blue += rate * ( in_pix.blue - cluster->center.blue );
   cluster->size = new_size;
  }
    
 stop = high_resolution_clock::now ( );
 duration = duration_cast<microseconds> ( stop - start ); 
 #ifdef PRINT_TIME_CLUST
 printf ( "Clustering time = %g\n", duration.count ( ) / 1e3 );
 #endif

 out_img = (RGB_Image *) malloc(sizeof(RGB_Image));
 out_img->data = (RGB_Pixel *) malloc(in_img->size * sizeof(RGB_Pixel));
 out_img->width = in_img->width;
 out_img->height = in_img->height;
 out_img->size = in_img->size;

 start = high_resolution_clock::now ( );

 /* Now quantize the image */
 for ( i = 0; i < in_img->size; i++ )
  {
   /* Cache the pixel */
   in_pix = in_img->data[i];
        
   /* Find the nearest center */
   min_dist = MAX_RGB_DIST;
   min_dist_index = -INT_MAX;
   for ( j = 0; j < num_colors; j++ ) 
    {
     cluster = &clusters[j];
            
     delta_red = in_pix.red - cluster->center.red;
     delta_green = in_pix.green - cluster->center.green;
     delta_blue = in_pix.blue - cluster->center.blue;
     dist = delta_red * delta_red + delta_green * delta_green + delta_blue * delta_blue;

     if ( dist < min_dist )
      {
       min_dist = dist;
       min_dist_index = j;
      }     
    }

   /* Replace the input color with the nearest color in the palette */
   in_pix = clusters[min_dist_index].center;
   out_pix = &out_img->data[i];
   out_pix->red = in_pix.red;
   out_pix->green = in_pix.green;
   out_pix->blue = in_pix.blue;       
  }
    
 stop = high_resolution_clock::now ( );
 duration = duration_cast<microseconds> ( stop - start ); 
 #ifdef PRINT_TIME_MAP
 printf ( "Mapping time = %g\n", duration.count ( ) / 1e3 );
 #endif

 free ( clusters );

 return out_img;
}

/* Color quantization using Lloyd's k-means algorithm */
/* 
   For application of Lloyd's k-means algorithm to color quantization, see
   M. E. Celebi, Improving the Performance of K-Means for Color Quantization, 
   Image and Vision Computing, vol. 29, no. 4, pp. 260–271, 2011.
 */

RGB_Image* 
lloyd_cluster ( const RGB_Image *in_img, const int num_colors, 
		const int max_iters, RGB_Pixel *mean )
{
 int i, j, min_dist_index;
 int num_iters, num_changes;
 int size;
 int *member;
 double min_dist, dist;
 double delta_red, delta_blue, delta_green;
 #ifdef PRINT_OBJ
 double old_obj, new_obj = DBL_MAX;
 #endif
 RGB_Cluster *clusters, *tmp_clusters, *cluster;
 RGB_Pixel in_pix, *out_pix;
 RGB_Image *out_img;

 clusters = ( RGB_Cluster * ) malloc ( num_colors * sizeof ( RGB_Cluster ) );
 tmp_clusters = ( RGB_Cluster * ) malloc ( num_colors * sizeof ( RGB_Cluster ) );
 member = ( int * ) malloc ( in_img->size * sizeof ( int ) );

 auto start = high_resolution_clock::now ( );
    
 /* Initialize cluster centers */
 maximin ( in_img, clusters, num_colors, mean );
    
 auto stop = high_resolution_clock::now ( );
 auto duration = duration_cast<microseconds> ( stop - start ); 
 #ifdef PRINT_TIME_INIT
 printf ( "Initialization time = %g\n", duration.count ( ) / 1e3 );
 #endif

 start = high_resolution_clock::now ( );

 num_iters = 0;

 /* Clustering pixel data using Lloyd's k-means algorithm */
 do
  {
   num_iters++;
   num_changes = 0;

   #ifdef PRINT_OBJ
   old_obj = new_obj;
   new_obj = 0.0;
   #endif

   /* Reset the new clusters */ 
   for ( j = 0; j < num_colors; j++ )
    {
     cluster = &tmp_clusters[j];
     cluster->center.red = 0.0;
     cluster->center.green = 0.0;
     cluster->center.blue = 0.0;
     cluster->size = 0;
    }

   for ( i = 0; i < in_img->size; i++ )
    {
     /* Cache the pixel */
     in_pix = in_img->data[i];
 
     /* Find the nearest center */
     min_dist = MAX_RGB_DIST; 
     min_dist_index = -INT_MAX;
     for ( j = 0; j < num_colors; j++ ) 
      {
       cluster = &clusters[j];
 
       delta_red = in_pix.red - cluster->center.red;
       delta_green = in_pix.green - cluster->center.green;
       delta_blue = in_pix.blue - cluster->center.blue;
       dist = delta_red * delta_red + delta_green * delta_green + delta_blue * delta_blue;
 
       if ( dist < min_dist )
        {
         min_dist = dist;
         min_dist_index = j;
        } 
      }
      
     #ifdef PRINT_OBJ
     new_obj += min_dist;
     #endif

     if ( ( num_iters == 1 ) || ( member[i] != min_dist_index ) )
      {
       /* Update the membership of the pixel */
       member[i] = min_dist_index;
       num_changes++;
      }
	
     /* Update the temp center of the nearest cluster */
     cluster = &tmp_clusters[min_dist_index];
     cluster->center.red += in_pix.red;
     cluster->center.green += in_pix.green;
     cluster->center.blue += in_pix.blue;
     cluster->size += 1;
    }

   /* Update all centers */
   for ( j = 0; j < num_colors; j++ )
    {
     cluster = &tmp_clusters[j];
     if ( ( size = cluster->size ) != 0 )
      {
       clusters[j].center.red = cluster->center.red / size;
       clusters[j].center.green = cluster->center.green / size;
       clusters[j].center.blue = cluster->center.blue / size;
      }
    }

   #ifdef PRINT_OBJ
   printf ( "iteration %d: obj = %g ; delta obj = %g [# changes = %d]\n", 
	    num_iters, new_obj, 
	    num_iters == 1 ? 0.0 : ( old_obj - new_obj ) / old_obj,  
     	    num_changes );
   #endif
  }
 while ( 0 < num_changes && num_iters < max_iters );
    
 stop = high_resolution_clock::now ( );
 duration = duration_cast<microseconds> ( stop - start ); 
 #ifdef PRINT_TIME_CLUST
 printf ( "Clustering time = %g\n", duration.count ( ) / 1e3 );
 #endif

 out_img = ( RGB_Image * ) malloc ( sizeof ( RGB_Image ) );
 out_img->data = ( RGB_Pixel * ) malloc ( in_img->size * sizeof ( RGB_Pixel ) );
 out_img->width = in_img->width;
 out_img->height = in_img->height;
 out_img->size = in_img->size;

 start = high_resolution_clock::now ( );

 /* Now quantize the image */
 for ( i = 0; i < in_img->size; i++ )
  {
   /* Cache the pixel */
   in_pix = in_img->data[i];
        
   /* Find the nearest center */
   min_dist = MAX_RGB_DIST;
   min_dist_index = -INT_MAX;
   for ( j = 0; j < num_colors; j++ ) 
    {
     cluster = &clusters[j];
	    	    
     delta_red = in_pix.red - cluster->center.red;
     delta_green = in_pix.green - cluster->center.green;
     delta_blue = in_pix.blue - cluster->center.blue;
     dist = delta_red * delta_red + delta_green * delta_green + delta_blue * delta_blue;

     if ( dist < min_dist )
      {
       min_dist = dist;
       min_dist_index = j;
      }     
    }

   /* Replace the input color with the nearest color in the palette */
   in_pix = clusters[min_dist_index].center;
   out_pix = &out_img->data[i];
   out_pix->red = in_pix.red;
   out_pix->green = in_pix.green;
   out_pix->blue = in_pix.blue;       
  }
    
 stop = high_resolution_clock::now ( );
 duration = duration_cast<microseconds> ( stop - start ); 
 #ifdef PRINT_TIME_MAP
 printf ( "Mapping time = %g\n", duration.count ( ) / 1e3 );
 #endif

 #ifdef PRINT_ITER
 printf ( "Number of iterations = %d\n", num_iters );
 #endif
 
 free ( clusters );
 free ( tmp_clusters );
 free ( member );

 return out_img;
}

/* Calculate the MSE between two RGB images */

double 
calc_MSE ( const RGB_Image *img1, const RGB_Image *img2 )
{
 double delta, total = 0.0;

 for ( int i = 0; i < img1->size; i++ )
  {
   delta = img1->data[i].red - img2->data[i].red;
   total += delta * delta;
   delta = img1->data[i].green - img2->data[i].green;
   total += delta * delta;
   delta = img1->data[i].blue - img2->data[i].blue;
   total += delta * delta;
  }

 return total / img1->size;
}

static void
print_usage ( char *prog_name )
{
 fprintf ( stderr, "Color Quantization Using Macqueen's K-Means Algorithm\n\n" ); 
 fprintf ( stderr, "Reference: S. Thompson, M. E. Celebi, and K. H. Buck, Fast Color Quantization Using Macqueen’s K-Means Algorithm, Journal of Real-Time Image Processing, to appear (https://doi.org/10.1007/s11554-019-00914-6), 2020.\n\n" ); 
 fprintf ( stderr, "Usage: %s -i <input image> -o <output image> -n <# colors> -a <algorithm> -p <presentation order> -e <exponent> -s <sampling rate> -r <# runs> -d <seed> -t <# iters>\n\n", prog_name );
 fprintf ( stderr, "All parameters are optional except for the <input image>\n\n" );
 fprintf ( stderr, "-i <input image>: input image in binary ppm format\n\n" ); 
 fprintf ( stderr, "-o <output image>: output image in binary ppm format (default = out.ppm)\n\n" ); 
 fprintf ( stderr, "-n <# colors>: # colors (integer greater than 1; default = 256).\n\n" ); 
 fprintf ( stderr, "-a <algorithm>: clustering algorithm (0: Macqueen, 1: Lloyd; default = 0)\n\n" );
 fprintf ( stderr, "-p <presentation order>: presentation order for Macqueen's algorithm (0: quasirandom, 1: pseudorandom; default = 0)\n\n" );
 fprintf ( stderr, "-e <exponent>: learning rate exponent for Macqueen's algorithm (double-precision floating point in [0.5, 1]; default = 0.5)\n\n" );
 fprintf ( stderr, "-s <sampling rate>: sampling rate for Macqueen's algorithm (double-precision floating point in (0, 1]; default = 1.0)\n\n" );
 fprintf ( stderr, "-r <# runs>: # independent runs for Macqueen's algorithm with pseudorandom presentation (positive integer; default = 1)\n\n" );
 fprintf ( stderr, "-d <seed>: seed for the pseudorandom number generator for Macqueen's algorithm (nonnegative integer; default = # secs. since 1/1/1970 UTC)\n\n" );
 fprintf ( stderr, "-t <# iters>: max. # iterations for Lloyd's algorithm (positive integer; default = %d)\n\n", INT_MAX );
 fprintf ( stderr, "The program generally runs faster if one or more of the following holds: i) image dimensions are small, ii) <# colors> is small, iii) <algorithm> is 0 (Macqueen), iv) <exponent> is small, v) <sampling rate> is small.\n\n" );
 fprintf ( stderr, "Many image manipulation software can display/convert/process PPM images including Irfanview (http://www.irfanview.com), GIMP (http://www.gimp.org), Netpbm (http://netpbm.sourceforge.net), and ImageMagick (http://www.imagemagick.org/script/index.php).\n\n" );

 exit ( EXIT_FAILURE );
}

void
mean_stdev ( const double *data, const int num_elems, double *mean, double *stdev )
{
 *mean = *stdev = 0.0;

 if ( num_elems == 1 ) 
  {
   *mean = data[0];
   return;
  }

 for ( int i = 0; i < num_elems; i++ )
  {
   *mean += data[i];
  }

 *mean /= num_elems;

 for ( int i = 0; i < num_elems; i++ )
  {
   *stdev += ( data[i] - *mean ) * ( data[i] - *mean );
  }

 *stdev = sqrt ( *stdev / ( num_elems - 1 ) );
}

int 
main ( int argc, char **argv ) 
{
 char in_file_name[256];
 char out_file_name[256] = "out.ppm";
 int num_colors = 256;
 int algo = 0;
 int pres_order = 0;
 int num_runs = 1;
 int seed = -1;
 int max_iters = INT_MAX;
 double lr_exp = 0.5;
 double sample_rate = 1.0;
 RGB_Pixel mean;
 RGB_Image *in_img, *out_img;

 if ( argc == 1 )
  {
   print_usage ( argv[0] );
  }

 for ( int i = 1; i < argc; i++ )
  {
   if ( !strcmp ( argv[i], "-i" ) )
    {
     strcpy ( in_file_name, argv[++i] );
    }
   else if ( !strcmp ( argv[i], "-o" ) )
    {
     strcpy ( out_file_name, argv[++i] );
    }
   else if ( !strcmp ( argv[i], "-n" ) )
    {
     num_colors = atoi ( argv[++i] );
     
     if ( num_colors < 2 ) 
      {
       print_usage ( argv[0] );
     }
    }
   else if ( !strcmp ( argv[i], "-a" ) )
    {
     algo = atoi ( argv[++i] );
     
     if ( algo != 0 && algo != 1 ) 
      {
       print_usage ( argv[0] );
      }
    }
   else if ( !strcmp ( argv[i], "-p" ) )
    {
     pres_order = atoi ( argv[++i] );
     
     if ( pres_order != 0 && pres_order != 1 ) 
      {
       print_usage ( argv[0] );
      }
    }
   else if ( !strcmp ( argv[i], "-e" ) )
    {
     lr_exp = atof ( argv[++i] );
     
     if ( lr_exp < 0.5 || 1.0 < lr_exp ) 
      {
       print_usage ( argv[0] );
      }
    }
   else if ( !strcmp ( argv[i], "-s" ) )
    {
     sample_rate = atof ( argv[++i] );
     
     if ( sample_rate <= 0.0 || 1.0 < sample_rate ) 
      {
       print_usage ( argv[0] );
      }
    }
   else if ( !strcmp ( argv[i], "-r" ) )
    {
     num_runs = atoi ( argv[++i] );

     if ( num_runs < 1 ) 
      {
       print_usage ( argv[0] );
      }
    }
   else if ( !strcmp ( argv[i], "-d" ) )
    {
     seed = atoi ( argv[++i] );
    }
   else if ( !strcmp ( argv[i], "-t" ) )
    {
     max_iters = atoi ( argv[++i] );
     
     if ( max_iters < 1 ) 
      {
       print_usage ( argv[0] );
      }
    }
   else
    {
     print_usage ( argv[0] );
    }
  }

 in_img = read_PPM ( in_file_name, &mean );

 if ( pres_order == 1 )
  {
   init_genrand ( seed < 0 ? time ( NULL ) : seed );
  }

 auto start = high_resolution_clock::now ( );

 if ( algo == 0 )
  {
   if ( pres_order == 0 || num_runs == 1 )
    {
     out_img = macqueen_cluster ( in_img, num_colors, pres_order, lr_exp, sample_rate, &mean );
     write_PPM ( out_img, out_file_name  );
     #ifdef PRINT_MSE
     printf ( "MSE = %.2f\n", calc_MSE ( in_img, out_img ) );
     #endif
     free ( out_img->data );
     free ( out_img );
    }
   else
    {
     double mean_mse, stdev_mse;
     double *mse = ( double * ) malloc ( num_runs * sizeof ( double ) );

     for ( int r = 0; r < num_runs; r++ )
      {
       out_img = macqueen_cluster ( in_img, num_colors, pres_order, lr_exp, sample_rate, &mean );
       mse[r] = calc_MSE ( in_img, out_img );

       free ( out_img->data );
       free ( out_img );
      }
       
     mean_stdev ( mse, num_runs, &mean_mse, &stdev_mse );
     #ifdef PRINT_MSE
     printf ( "MSE = $%.1f_{%.1f}$\n", mean_mse, stdev_mse );
     #endif
     free ( mse );
    }
  }
 else
  {
   out_img = lloyd_cluster ( in_img, num_colors, max_iters, &mean );
   write_PPM ( out_img, out_file_name  );
   #ifdef PRINT_MSE
   printf ( "MSE = %.2f\n", calc_MSE ( in_img, out_img ) );
   #endif
   free ( out_img->data );
   free ( out_img );
  }
    
 auto stop = high_resolution_clock::now ( );
 auto duration = duration_cast<microseconds> ( stop - start ); 
   
 #ifdef PRINT_TIME_TOTAL
 printf ( "Total time = %g\n", duration.count ( ) / 1e3 );
 #endif

 free ( in_img->data );
 free ( in_img );

 return EXIT_SUCCESS;
}
