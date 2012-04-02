package IrisRecog;

/**
 * GUIMenuItem.java
 *
 * Extends the JMenuItem swing component to provide specific background color.
 *

 */

import javax.swing.*;

public class GUIMenuItem extends JMenuItem
{
	/**
	 * Default constructor for creating a new GUI menu item.
	 *
	 * @param s The display string of the menu item
	 */
	public GUIMenuItem(String s)
	{
		super(s);

		this.setBackground(Constants.CLR_DEFAULT_BACKGROUND);
	}
}