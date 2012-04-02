package IrisRecog;

/**
 * PupilCenterDetection.java
 *

 *
 */

 import java.awt.*;
 import java.lang.Object;

public class PupilCenterDetection
{
    static final int BLOCK_SIZE = 30;
    static final int THRESHHOLD = 50;

    static Point getPupilCenter(GrayscaleImage img)
    {
        int left = img.getWidth();
        int top = img.getHeight();
        Point center=new Point(0,0);
        double sum=0;
        int counter=0;

        //For each row in the image
        for(int y = 0; y < img.getHeight(); y++)
        {
              //For each column in the image
              for(int x = 0; x < img.getWidth(); x++)
              {
                     if(isHorizontalBlockAbovePupilThreshold(img, new Point(x,y)))
                     {
                           if( x < left )
                           {
                           		counter = 1;
                                sum = y;
                                left = x;

                                  	System.out.println(x + ", " + y);
                           }

                           else if (x == left)
                           {
                           	  	counter++;
                           	  	sum += y;
                           }

                           x = img.getWidth();
                     }
              }
        }

        center.y = (int)(sum / counter);

        sum = 0;
        counter = 0;

        //For each column in the image
        for(int x = 0; x < img.getWidth(); x++)
        {
              //For each row in the image
              for(int y = 0; y < img.getHeight(); y++)
              {
                     if(isVerticalBlockAbovePupilThreshold (img, new Point(x,y)))
                     {
                           if( y < top )
                           {
                           		counter = 1;
                           		sum = x;
								top = y;

                                System.out.println(x + ", " + y);
                           }
                           else if (y == top)
                           {
                           		counter++;
                           		sum += x;
                           }

                           y = img.getHeight();
                     }
              }
       }

       center.x = (int)(sum / counter);

       return center;
    }

    static boolean isHorizontalBlockAbovePupilThreshold(GrayscaleImage img, Point a)
    {
         //For each pixel in the block
         for(int i = a.x; i < (a.x + BLOCK_SIZE) && i < img.getWidth(); i++)
         {
              //If the intensity of the image at the point specified
              //is not greater than the threshold
              if( img.getIntensity(new Point(i, a.y)) > THRESHHOLD )
              {
                     return false;
              }
         }

         return true;
    }

    static boolean isVerticalBlockAbovePupilThreshold(GrayscaleImage img, Point a)
    {
         //For each pixel in the block
         for(int i = a.y; i < (a.y + BLOCK_SIZE) && i < img.getHeight(); i++)
         {
              //If the intensity of the image at the point specified
              //is not greater than the threshold
              if( img.getIntensity(new Point(a.x, i)) > THRESHHOLD )
              {
                     return false;
              }
         }

         return true;
    }
}