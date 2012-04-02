package IrisRecog;

/**
 * AgentFileChooser.java
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

public class AgentFileChooser extends JFileChooser
{
	public AgentFileChooser()
    {
		//Add the custom agent fiel filter
		this.addChoosableFileFilter(new AgentFilter());
		this.setAcceptAllFileFilterUsed(false);

		//Set default directory to agents folder
		try
		{
			this.setCurrentDirectory(new File("../agents"));
		}
		catch(Exception e) { }
	}
}