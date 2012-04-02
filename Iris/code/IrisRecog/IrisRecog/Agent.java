package IrisRecog;

/**
 * Agent.java
 *
 * Represents the agent that is performing all of the functions needed in iris
 * recognition. The agent has a primitive memory to store those eyes that have
 * been scanned previously. When matching a new eye, it scans just as it would to
 * commit to memory but compares that eye against the current memory.
 *

 */

import java.awt.*;
import java.util.*;
import javax.swing.*;
import java.lang.*;

public class Agent
{
    /*
     * Representation of the memory of the agent, which is really a Vector
	 * collection of identities.
     */
    protected Memory mem_Memory;

    /*
     * The current subject being scanned, whether in an attempt to commit to
     * memory or to compare against memory
     */
	protected Subject sub_Subject;

	/*
	 * A reference to the user interface
	 */
	protected GUI gui_View;

	/**
	 * Creates an agent and associates it with the specified user interface.
	 *
	 * @param v A reference to the user interface
	 */
	public Agent(GUI v)
	{
		this.gui_View = v;
		this.mem_Memory = new Memory(); //initially empty memory
	}

	/**
	 * Creates an agent and associates it with the specified user interface and
	 * gives it the specified memory.
	 *
	 * @param v A reference to the user interface
	 * @param mem The vector of identities to be set as the agent's initial memory
	 */
	public Agent(GUI v, Memory mem)
	{
		this.gui_View = v;
		this.mem_Memory = mem;
	}

	/**
	 * Returns the current memory of the agent.
	 *
	 * @return Vector The memory of the agent
	 */
	public Memory getMemory()
	{
		return this.mem_Memory;
	}

	/**
	 * Sets the current memory of the agent to the specified collection of identities.
	 *
	 * @param v The Vector of identities.
	 */
	public void setMemory(Memory v)
	{
		this.mem_Memory = v;
	}

	/**
	 * Returns the subject currently associated with the agent (either by the scanning
	 * or comparing process).
	 *
	 * @return Subject The subject being scanned
	 */
	public Subject getCurrentSubject()
	{
		return this.sub_Subject;
	}

	/**
	 * Simulates the agents scaning the eye of a subject. As scan takes the form
	 * of reading an images from the specified path for the given subject, identified
	 * by name. The agent sets its current subject to the new subject and performs
	 * the scanning operations on that subject. Finally, if the scaning operations
	 * are successful, the identity (along with the unwrapped image) is commited
	 * to the memory of the agent.
	 *
	 * @param name The name of the subject being scanned
	 * @param path The path to the eye image
	 */
	public void scanEye(String name, String path)
	{
		//Set the current subject
		this.sub_Subject = new Subject(name, path);

		Functions.applyMedianFilter(this.sub_Subject);

		//Find the center of the pupil
		Functions.detectPupilCenter(this.sub_Subject);

		//Find the pupil and radii
		Functions.detectIrisRadius(this.sub_Subject);

		//If both the radii are found
		if(this.sub_Subject.getIrisRadius() > 0 && this.sub_Subject.getPupilRadius() > 0)
		{
			//Unroll the iris of subject
			Functions.unrollIris(this.sub_Subject);

			//Add the subject to memory as identity
			this.mem_Memory.addIdentity(new Identity(this.sub_Subject.getName(), this.sub_Subject.getUnwrappedImage()));
		}
		else
			JOptionPane.showMessageDialog(gui_View, "Iris And/Or Pupil Radius Not Found: '" + this.sub_Subject.getName() + "' not successfully commited to my memory", "Error Finding Radii", JOptionPane.ERROR_MESSAGE);
	}

	/**
	 * Rescans the eye of the current subject (primarily for use in testing
	 * theshhold levels.
	 */
	public void rescanEye()
	{
		Functions.detectPupilCenter(this.sub_Subject);

		Functions.detectIrisRadius(this.sub_Subject);

		if(this.sub_Subject.getIrisRadius() > 0 && this.sub_Subject.getPupilRadius() > 0)
			Functions.unrollIris(this.sub_Subject);
		else
			JOptionPane.showMessageDialog(gui_View, "Iris And/Or Pupil Radius Not Found: '" + this.sub_Subject.getName() + "' not successfully commited to my memory", "Error Finding Radii", JOptionPane.ERROR_MESSAGE);
	}

	/**
	 * Matches a new subject against all of the available subjects in memory in an
	 * attempt to determine the identity of the new person. The agent scans the
	 * new eye just as it would in the scanEye function but instead of committing
	 * the eye to memory, it searches its memory to find the best identity match.
	 *
	 * @param path The path to the eye image
	 * @return Identity The identity of the best match the agent can find
	 */
	public Identity matchEye(String path)
	{
		double maxProbability = 1;
		int bestIdentity = 0;

		//Set the current subject
		this.sub_Subject = new Subject(" ", path);

		Functions.applyMedianFilter(this.sub_Subject);

		//Find the center of the pupil
		Functions.detectPupilCenter(this.sub_Subject);

		//Find the pupil and radii
		Functions.detectIrisRadius(this.sub_Subject);

		//If both the radii are found
		if(this.sub_Subject.getIrisRadius() > 0 && this.sub_Subject.getPupilRadius() > 0)
		{
			//Unroll the iris of the subject
			Functions.unrollIris(this.sub_Subject);

			if(this.mem_Memory.size() > 0)
			{
				//Create an array to hold the probability of the matches
				double [] matchProbabilities = new double[this.mem_Memory.size()];

				//For each of the identities in memory, compared the scanned eye to it
				for(int i = 0; i < this.mem_Memory.size(); i++)
				{
					matchProbabilities[i] = Functions.matchIdentity(this.sub_Subject, (Identity)this.mem_Memory.get(i));

					if(matchProbabilities[i] >= maxProbability)
					{
						//maxProbability = matchProbabilities[i];
						bestIdentity = i;
						return((Identity)(this.mem_Memory.get(bestIdentity)));
					}
				}

				//Return the best identity found

			}

				return null;
		}
		else //one or both of radii not found, therefore the new eye cannot be matched to memory
		{
			JOptionPane.showMessageDialog(gui_View, "Iris And/Or Pupil Radius Not Found: Cannot match against memory", "Error Finding Radii", JOptionPane.ERROR_MESSAGE);

			return null;
		}
	}
}