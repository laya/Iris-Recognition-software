package IrisRecog;

/**
 * ImageFilter.java
 *
 * Extends the FileFilter swing class to provided functionality for retriving
 * and selecting only jpg/jpeg image files in the ImageFileChooser.
 *

 */

import java.io.File;
import javax.swing.*;
import javax.swing.filechooser.*;

public class ImageFilter extends FileFilter
{
	/**
	 * Determines if a file is accepted by this filter
	 *
	 * @param f The file that is being attempted to be acccepted
	 * @return true if the file is accepted, false otherwise
	 */
    public boolean accept(File f)
    {
    	//Accept the file if it is a directory
        if(f.isDirectory())
        	return true;

        //Save the file extension
        String str_Extension = Functions.getExtension(f);

        //If the extension is not null and it is either JPEG or JPG then accept the file, otherwise reject it
        if(str_Extension != null && (str_Extension.equals(Constants.STR_JPEG_EXTENSION) || str_Extension.equals(Constants.STR_JPG_EXTENSION)) )
            return true;
		else
        	return false;
    }

	/**
	 * Return the description of this file filter
	 *
	 * @return the description
	 */
    public String getDescription()
    {
        return "JPEG Image File Formats ...";
    }
}