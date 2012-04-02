package IrisRecog;

/**
 * ImageFileChooser.java
 *
 * Extends the JFileChooser swing component by adding an image preview pane
 * to the right and setting the filter to select only jpg/jpeg images.
 *

 */

import java.io.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.filechooser.*;

public class ImageFileChooser extends JFileChooser
{
	/**
	 * Constructs a new file chooser to find image files
	 */
	public ImageFileChooser()
    {
		this.addChoosableFileFilter(new ImageFilter());
		this.setAcceptAllFileFilterUsed(false);

		try
		{
			//Set the default directory to the image folder
			this.setCurrentDirectory(new File("../images"));
		}
		catch(Exception e) { }

		//Add the preview pane.
		this.setAccessory(new ImagePreview(this));
	}
}