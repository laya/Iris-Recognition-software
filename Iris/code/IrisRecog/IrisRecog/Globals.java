package IrisRecog;

/**
 * Globals.java
 *
 * This static abstract class, non-instantiatable, defines the constants used
 * throughout the functions and methods of this application.
 *

 */

import java.awt.*;

public abstract class Globals
{
	/*
	 * The variance (in intensity amount) used in the pupil center detection algorithm.
	 */
    public static int PUPIL_VARIANCE = 17;

   	public static double PUPIL_BLOCK_SIZE_PORTION = .80;

   	public static int GUASSIAN_WIDTH = 30;

   	public static int EDGE_THRESHHOLD = 50;

   	public static double CIRCLE_PERCENT_MATCH = .90;

   	public static int MEDIAN_FILTER_WINDOW_SIZE = 8;

   	public static double LOCALIZATION_PERCENTAGE = .80;
}