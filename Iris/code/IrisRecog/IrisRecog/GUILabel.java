package IrisRecog;

/**
 * GUILabel.java
 *
 * Extends the JLabel swing component to provide specific color and functionality.
 *

 */

import javax.swing.*;

public class GUILabel extends JLabel
{
	/**
	 * Default constructor with blank label.
	 */
	public GUILabel()
	{
		super();

		this.setBackground(Constants.CLR_DEFAULT_BACKGROUND);
	}

	/**
	 * Constructor for creating a new GUI label with an image icon.
	 *
	 * @param img The image icon to display in the label
	 */
	public GUILabel(ImageIcon img)
	{
		super(img);

		this.setBackground(Constants.CLR_DEFAULT_BACKGROUND);
	}

	/**
	 * Constructor for creating a new GUI label with text and horizontal alignment.
	 *
	 * @param s The string to display in the label
	 * @param aling The horizontal alignment of the text
	 */
	public GUILabel(String s, int align)
	{
		super(s, align);

		this.setBackground(Constants.CLR_DEFAULT_BACKGROUND);
	}

}