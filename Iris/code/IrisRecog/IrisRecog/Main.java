package IrisRecog;/** * Main.java * * Drives the creation of the model-view design and starts the application GUI. * */public class Main{	/**	 * Main method initiates the model and the desing, relates them together,	 * and starts the application GUI.	 *	 * @param args String array of command line arguments	 */	public static void main(String [] args)	{		GUI gui_View = new GUI();		IrisRecog irg_Model = new IrisRecog(gui_View);		gui_View.setModel(irg_Model);	}}