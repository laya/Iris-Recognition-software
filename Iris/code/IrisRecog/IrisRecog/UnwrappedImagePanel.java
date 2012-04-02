package IrisRecog;

/**
 * ImagePanel.java
 *
 * Extends the functionality of the JPanel swing component to be able to draw
 * centered images in the panel. This class overrides the paintComponent method
 * and handles its own drawing.
 *

 */

import java.awt.*;
import javax.swing.*;
import javax.swing.border.*;

public class UnwrappedImagePanel extends JPanel
{
	private Image img;

	/**
	 * Creates an image panel with lowered bevel and white background.
	 */
	public UnwrappedImagePanel()
	{
		this.setBorder(BorderFactory.createBevelBorder(BevelBorder.LOWERED));
		this.setBackground(Constants.CLR_DEFAULT_BACKGROUND);

		img = null;
	}

	public void setImage(Image i)
	{
		img = i;

		this.repaint();
	}

	public void paintComponent(Graphics g)
	{
		//Clear the contents of the panel
		g.clearRect(0, 0, this.getWidth(), this.getHeight());

		//If the image of the panel has been set
		if(img!=null)
		{
			ImageIcon scaledImage; //the image icon used to scale
			double scaleFactor;	   //the percentage amount of scaling exhbited to fit in the panel
			int yOffset;		   //the amount the image is shifted down to be centered vertically

			scaledImage = new ImageIcon(img.getScaledInstance(this.getWidth(), -1, Image.SCALE_DEFAULT));

			//Calculate the distance the image needs to be shifted down
			yOffset = (this.getHeight() / 2) - (scaledImage.getIconHeight() / 2);

			//Calculate the amount that the image is scaled
			scaleFactor = (this.getWidth() * 1.0) / img.getWidth(null);

			//Scale the image and paint it using the graphics object of this panel component
			scaledImage.paintIcon(this, g, 0, yOffset);
		}
  	}
}