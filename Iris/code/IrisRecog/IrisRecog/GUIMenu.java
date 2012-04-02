package IrisRecog;

/**
 * GUIMenu.java
 *
 * Extends the JMenu swing component to provide specific background color.
 *

 */

import javax.swing.*;

public class GUIMenu extends JMenu
{
	/**
	 * Default constructor for creating a new GUI menu with the specified name
	 *
	 * @param s The display string of the menu
	 */
	public GUIMenu(String s)
	{
		super(s);

		this.setBackground(Constants.CLR_DEFAULT_BACKGROUND);
	}
}