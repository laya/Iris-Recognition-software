package IrisRecog;

/**
 * MedianImage.java
 *
 * Subclasses the unwrapped image class to provided one more method of
 * functionality to retrieve pixel intensities.
 *

 */

import java.awt.*;
import java.awt.image.*;
import javax.swing.*;
import java.io.*;

public class MedianImage extends UnwrappedImage implements Serializable
{
	/**
	 * Constructor creates a median image with the specified width and
	 * height by calling the unwrapped image parent constructor.
	 *
	 * @param w The width of the image
	 * @param h The height of the image
	 */
	public MedianImage(int w, int h)
	{
		super(w, h);
	}
}