package IrisRecog;

/**
 * UnwrappedImage.java
 *
 * Represents an image that contains the unrolled, unwrapped, version of the
 * subject's eye. The image is a grayscale format.
 *

 */

import java.awt.*;
import java.awt.image.*;
import javax.swing.*;
import java.io.*;

public class UnwrappedImage extends GrayscaleImage
{
	/*
	 * The height of the unwrapped image
	 */
	protected int int_Height;

	/*
	 * The width of the unwrapped image
	 */
	protected int int_Width;

	/**
	 * Constructs and unwrapped image with the specified width and height.
	 *
	 * @param w The width
	 * @param h The height
	 */
	public UnwrappedImage(int w, int h)
	{
		super(w, h);

		this.int_Arr_Pixels = new int[w][h];
		this.int_Width = w;
		this.int_Height = h;
	}

	/**
	 * Returns the height of the unwrapped image
	 *
	 * @return the height
	 */
	public int getHeight()
	{
		return this.int_Height;
	}

	/**
	 * Returns the width of the unwrapped image
	 *
	 * @return the width
	 */
	public int getWidth()
	{
		return this.int_Width;
	}

	/**
	 * Returns and image object version of the unwrapped image.
	 *
	 * @return the image
	 */
	public Image getImage()
	{
		//Create the buffered image with the correct size
		BufferedImage bImage = new BufferedImage(this.int_Width, this.int_Height, BufferedImage.TYPE_BYTE_GRAY);

		//Get the graphics of the buffered image
		Graphics g = bImage.getGraphics();

		//For each row in image
		for(int i = 0; i < int_Arr_Pixels.length; i++)
		{
			//For each column in image
			for(int j = 0; j < int_Arr_Pixels[0].length; j++)
			{
				//Set the color to the specified intensity
				g.setColor(new Color(int_Arr_Pixels[i][j] ,int_Arr_Pixels[i][j], int_Arr_Pixels[i][j]));

				//Draw a single pixel (ie: line with lenght = 1)
				g.drawLine(i, j, i , j);
			}
		}

		return (Image)bImage;
	}
}