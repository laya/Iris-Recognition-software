package IrisRecog;

/**
 * IrisRecog.java
 *
 * The model portion of the application (in the model-view design) that conducts
 * the operations requested by the view (the GUI). This class makes use of many
 * of the public static functions available in the Functions class. Logically,
 * this class represents the abilities of the agt_Agent and defines the actions it
 * is capable of completing.
 *

 */

import java.awt.*;
import java.awt.image.*;
import java.util.*;
import java.lang.*;

public class IrisRecog
{
	/*
	 * A reference to the user interface.
	 */
	protected static GUI gui_UserInterface;

	/*
	 * The agent of the model
	 */
	protected Agent agt_Agent;

	/**
	 * Creates the model and relates it to the specified user interface.
	 *
	 * @param ui The user interface to associated with the model
	 */
	public IrisRecog(GUI ui)
	{
		this.gui_UserInterface = ui;
		this.agt_Agent = new Agent(ui);
	}

	/**
	 * Returns the agent that is associated with the model.
	 *
	 * @return The agent
	 */
	public Agent getAgent()
	{
		return this.agt_Agent;
	}

	/**
	 * Set the agent that the model is associated with.
	 *
	 * @param a The agent
	 */
	public void setAgent(Agent a)
	{
		this.agt_Agent = a;
	}

	/**
	 * Tells the agent to scan the eye that is specified by the
	 * name of the subject and the path of the image of the eye of the
	 * subject.
	 *
	 * @param name The name of the subject being scanned
	 * @param path The path to the image of the eye of the subject
	 */
	public void scanEye(String name, String path)
	{
		this.agt_Agent.scanEye(name, path);
	}

	/**
	 * Tells the agent to rescan the eye of the current subject.
	 */
	public void rescanEye()
	{
		if(this.agt_Agent.getCurrentSubject() != null)
			this.agt_Agent.rescanEye();
	}

	/**
	 * Tells the agent to match eye image of the specified path
	 * against its memory.
	 *
	 * @param path The path to the image of the eye being matched
	 */
	public String matchEye(String path)
	{
    Identity ident=(Identity)this.agt_Agent.matchEye(path);
    if(ident==null)
    return "No Matching!!";
    else
    return ("Welcome "+(this.agt_Agent.matchEye(path)).getIdentityName());
	//	return (this.agt_Agent.matchEye(path)).getIdentityName();
	}

	/**
	 * Returns the original image of the current subject being processed
	 * by the agent.
	 *
	 * @return the image
	 */
	public Image getOriginalImage()
	{
		return this.agt_Agent.getCurrentSubject().getOriginalImage();
	}

	/**
	 * Returns the grayscale image of the current subject being processed
	 * by the agent.
	 *
	 * @return the image
	 */
	public Image getGrayscaleImage()
	{
		return this.agt_Agent.getCurrentSubject().getGrayscaleImage().getImage();
	}

	/**
	 * Returns the median filter image of the current subject being processed
	 * by the agent.
	 *
	 * @return the image
	 */
	public Image getMedianFilterImage()
	{
		return this.agt_Agent.getCurrentSubject().getMedianImage().getImage();
	}

	/**
	 * Returns the edge image of the current subject being processed by the
	 * agent.
	 *
	 * @return the image
	 */
	public Image getEdgeImage()
	{
		return this.agt_Agent.getCurrentSubject().getEdgeImage().getImage();
	}

	/**
	 * Returns the unwrapped image of the current subject being processed by the
	 * agent.
	 *
	 * @return the image
	 */
	public Image getUnwrappedImage()
	{
		if(this.agt_Agent.getCurrentSubject().getUnwrappedImage() == null)
			return null;
		else
			return this.agt_Agent.getCurrentSubject().getUnwrappedImage().getImage();
	}

	/**
	 * Returns the pupil center of the current subject being processed by the
	 * agent.
	 *
	 * @return The point of the center of the pupil
	 */
	public Point getSubjectPupilCenter()
	{
		return this.agt_Agent.getCurrentSubject().getPupilCenter();
	}

	/**
	 * Returns the iris radius of the current subject being processed by the agent.
	 *
	 * @return The radius of the iris
	 */
	public double getSubjectIrisRadius()
	{
		return this.agt_Agent.getCurrentSubject().getIrisRadius();
	}

	/**
	 * Returns the pupil radius of the current subject being processed by the agent.
	 *
	 * @return The radius of the pupil
	 */
	public double getSubjectPupilRadius()
	{
		return this.agt_Agent.getCurrentSubject().getPupilRadius();
	}
}