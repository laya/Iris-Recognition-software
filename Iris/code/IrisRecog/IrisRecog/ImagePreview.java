package IrisRecog;

/**
 * ImagePreview.java
 *
 * Creates a component that show image icon Thumbnails previews. It is intended for
 * use as an addition component in the ImageFileChooser dialog. This class
 * implements the PropertyChangeListener interface and responds to changes in
 * the properties of the associated ImageFileChooser dialog and responds
 * accordingly. This class also overrides the paintComponent method and handles
 * drawing itself.
 *

 */

import javax.swing.*;
import java.beans.*;
import java.awt.*;
import java.io.File;

public class ImagePreview extends JComponent implements PropertyChangeListener
{
	/*
	 * The current icn_Thumbnail of the current fil_Current (image).
	 */
    protected ImageIcon icn_Thumbnail = null;

    /*
     * The current fil_Current (image).
     */
    protected File fil_Current = null;

	/**
	 * Creates and image preview object for the specified fil_Current chooser.
	 *
	 * @param fc The fil_Current chooser to associate this component with.
	 */
    public ImagePreview(JFileChooser fc)
    {
    	//Set the the preferred size of the image preview
        this.setPreferredSize(new Dimension(200, 200));

        //Set the property change listener of the fil_Current chooser to this object
        fc.addPropertyChangeListener(this);
    }

    public void loadImage()
    {
    	//If the fil_Current is null
        if (fil_Current == null)
        {
            icn_Thumbnail = null; //set the thumnail to null
            return;
        }

        //Create a temporary image icon from the specified fil_Current
        ImageIcon tmpIcon = new ImageIcon(fil_Current.getPath());

        //If the temporary image icon is not successfully created
        if(tmpIcon != null)
        {
        	//If the icon width is greater than 180
            if (tmpIcon.getIconWidth() > 180)
            {
            	//Scale the icon into the icn_Thumbnail
                icn_Thumbnail = new ImageIcon(tmpIcon.getImage().getScaledInstance(180, -1, Image.SCALE_DEFAULT));
            }
            else
            {
            	//There is no need to scale the icn_Thumbnail
                icn_Thumbnail = tmpIcon;
            }
        }
    }

	/**
	 * Overrides the method of the PropertyChangeListener interface
	 * and responds to PropertyChangeEvents fired by the associated
	 * fil_Current chooser.
	 *
	 * @param e The property change event firing this listener
	 */
    public void propertyChange(PropertyChangeEvent e)
    {
        boolean update = false;

        String prop = e.getPropertyName();

        //If the directory changed, don't show an image.
        if (JFileChooser.DIRECTORY_CHANGED_PROPERTY.equals(prop))
        {
            fil_Current = null;
            update = true;
        }
        //If a fil_Current became selected, find out which one.
        else if (JFileChooser.SELECTED_FILE_CHANGED_PROPERTY.equals(prop))
        {
            fil_Current = (File) e.getNewValue();
            update = true;
        }

        //Update the thumbnial preview accordingly.
        if (update)
        {
            icn_Thumbnail = null;

            if (isShowing())
            {
                loadImage();
                repaint();
            }
        }
    }

	/**
	 * Overrides the paintComponent method of the component to handle its own
	 * painting.
	 *
	 * @param g The graphics object of this component
	 */
    protected void paintComponent(Graphics g)
    {
    	//If the icn_Thumbnail is null
        if (icn_Thumbnail == null)
        {
        	//Attempt to load the image
            loadImage();
        }
        else
        {
        	//Get the x and y position to paint the icon
            int x = getWidth()/2 - icn_Thumbnail.getIconWidth()/2;
            int y = getHeight()/2 - icn_Thumbnail.getIconHeight()/2;

            if (y < 0)
            {
                y = 0;
            }

            if (x < 5)
            {
                x = 5;
            }

            //Draw the icn_Thumbnail in the graphics context
            icn_Thumbnail.paintIcon(this, g, x, y);
        }
    }
}