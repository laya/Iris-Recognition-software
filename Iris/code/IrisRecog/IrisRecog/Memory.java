package IrisRecog;

/**
 * Memory.java
 *
 * Represents the memory of the agent, which is simply a vector of identities.
 * The memory can be serialized (used to save memory to disk and load memory
 * into an agent from disk).
 *

 *
 */

import java.util.*;
import java.io.*;

public class Memory extends Vector implements Serializable
{
	/**
	 * Constructor creates a an empty memory.
	 */
	public Memory()
	{
		super();
	}

	/**
	 * Adds an identity to the memory.
	 *
	 * @param s The identity to add
	 */
	public void addIdentity(Identity s)
	{
		this.add(s);
	}

	/**
	 * Returns the identity from memory at the specified index.
	 *
	 * @param int The index
	 * @return the identity at the specified index
	 */
	public Identity getIdentity(int index)
	{
		return (Identity)this.get(index);
	}
}