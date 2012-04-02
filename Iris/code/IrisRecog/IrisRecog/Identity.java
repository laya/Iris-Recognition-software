package IrisRecog;

/**
 * Identity.java
 *
 * Represents the identity of a scanned subject, which is simply a copy of the
 * unwrapped image resulting from the scanning process and the name of the subject.
 *

 */

import java.io.*;

public class Identity implements Serializable
{
	/*
	 * The name of the identity
	 */
	protected String str_IdentityName;

	/*
	 * The unwrapped image of the identity
	 */
	protected UnwrappedImage uwr_IdentityIris;

	/**
	 * Constructs a new identity with the specified name and unwrapped image.
	 *
	 * @param i The string name of the subject to be stored
	 * @param u The unwrapped image of the subject to be stored
	 */
	public Identity(String i, UnwrappedImage u)
	{
		this.str_IdentityName = i;
		this.uwr_IdentityIris = u;
	}

	/**
	 * Returns the name of the identity.
	 *
	 * @return the name of the identity
	 */
	public String getIdentityName()
	{
		return this.str_IdentityName;
	}

	/**
	 * Returns the unwrapped image of the identity
	 *
	 * @return the unwrapped image of the identity
	 */
	public UnwrappedImage getIdentityIris()
	{
		return this.uwr_IdentityIris;
	}

	/**
	 * Overrides the object toString method
	 *
	 * @return the string representation of the identity (its name)
	 */
	public String toString()
	{
		return this.str_IdentityName;
	}
}