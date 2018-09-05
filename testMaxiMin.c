    //Loop through every pixel and find the pixel furthest away from the closest center in the centers array
    totalRGB = 0;
    //Loop through numFixedColors times
    for (int index = 0; index < numFixedColors; index++)
    {
        //On the first iteration, assign cluster to be the only cluster already done
        if (index == 0)
        {
            cluster = &clusters[0];
        }
        else 
        {
            //Else, find the nearest of the two clusters
            for (int i = 1; i < index; i++)
            {
                tempCluster1 = &clusters[i];
                tempCluster2 = &clusters[i - 1];
                delta = (tempCluster1->center.red + tempCluster2->center.red) * (tempCluster1->center.red + tempCluster2->center.red);
                delta += (tempCluster1->center.green + tempCluster2->center.green) * (tempCluster1->center.green + tempCluster2->center.green);
                delta += (tempCluster1->center.blue + tempCluster2->center.green) * (tempCluster1->center.blue + tempCluster2->center.green);

                tempTotalRGB = delta;

                if (tempTotalRGB < total)
                {
                    cluster = &clusters[i];
                }
            }
        }
        //Use the closest of the previous clusters and loop through each pixel and find the largest distance away from it.  This pixel will be the new cluster center.
        for (int i = 0; i < numPixels; i++)
        {
            pixel = img->data[i];
            delta = (cluster->center.red + pixel.red) * (cluster->center.red + pixel.red);
            delta += (cluster->center.green + pixel.green) * (cluster->center.green + pixel.green);
            delta += (cluster->center.blue + pixel.blue) * (cluster->center.blue + pixel.blue);

            tempTotalRGB = delta;

            if (tempTotalRGB > totalRGB)
            {
                totalRGB = tempTotalRGB;
                tempIteration = i;
            }
        }
        if (index != numFixedColors - 1)
        {
            clusters[index + 1].center.red = img->data[tempIteration].red;
            clusters[index + 1].center.green = img->data[tempIteration].green;
            clusters[index + 1].center.blue = img->data[tempIteration].blue;
            clusters[index + 1].size = 1;
        }
    }