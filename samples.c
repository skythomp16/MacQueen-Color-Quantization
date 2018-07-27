    //Taking all of the red out of red pixels, all of the green out of green pixels, and all of the blue out of blue pixels -- for testing purposes
    /*
    int i;
    if (img) {

        for (i = 0; i < img->x*img->y; i++) {
            img->data[i].red = RGB_COMPONENT_COLOR - img->data[i].red;
            img->data[i].green = RGB_COMPONENT_COLOR - img->data[i].green;
            img->data[i].blue = RGB_COMPONENT_COLOR - img->data[i].blue;
        }
    }
    */

           /* Comments from Dr. Celebi
            // NEW
            // old_size = fetch the current size of cluster i
            // new_size = old_size + 1;
            // center[i].red = ( old_size * center[i].red + randPix.red ) / ( double ) new_size;
            // center[i].green = ( old_size * center[i].green + randPix.green ) / ( double ) new_size;
            // center[i].blue = ( old_size * center[i].blue + randPix.blue ) / ( double ) new_size;
            // update size of cluster i as new_size
        */


       /*
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
*/

                /*
                printf("I entered this function!   ");
                counter++;
                printf("%d", randPix.blue);
                printf("   ");
                printf("%d\n", counter);
                */

/*
for (int i = 0; i < img->height * img->width; i++)
{
    printf("%d", (int)img->data[i].red);
    //printf("    ");
    printf("%d", (int)img->data[i].green);
    //printf("    ");
    printf("%d", (int)img->data[i].blue);
    printf("\n");
}
*/

for (int i = 0; i < img->height * img->width; i++)
{
    printf("%d", (int)pix[i]);
    printf("\n");
}

    //For testing purposes -- prints the rgb values of each center
    for (int i = 0; i < numFixedColors; i++)
    {
        printf("%f", clusters[i].center.red);
        printf("     ");
        printf("%f", clusters[i].center.green);
        printf("     ");
        printf("%f\n", clusters[i].center.blue);
    }


   //Initializing clusters
    //First some variables
    double total = 0.0;
    double total2 = 0.0;
    double s;
    int j = 0;
    int ind = 0;
    int quit = 0;

    //Create an array of cumulative values with the same number of elements as pixels(r + g + b used for this)
    double *array = malloc(numPixels * sizeof(double));

    for (int i = 0; i < numPixels; i++)
    {
        //Adding all rgb values for each pixel
        s = img->data[i].red + img->data[i].green + img->data[i].blue;

        //Adding these values to the running total
        total += s;

        //Adding these values to the array
        array[i] = s;
    }

    //Now loop through again and create a proportion array using total the the sum of rgb values for each pixel
    for (int i = 0; i < numPixels; i++)
    {
        array[i] = array[i] / total;

        total2 += array[i];

        array[i] = array[i] + total2;
    }


    //Initialize p to a random value between 0 and 1
    //double p = (double)rand() / (double)RAND_MAX;

    double p = 0.00;

    //loop through cumulative probablility array and pick centroids
    while (j < numPixels || quit != 1)
    {
        if (array[j] >= p)
        {
            PPMPixel temp = img->data[j];
            clusters[ind].center = temp;
            ind++; 
            p += 0.01;
            if(ind == 63)
            {
                quit = 1;
            }
        }

        j++;
    }