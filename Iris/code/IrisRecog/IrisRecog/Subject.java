package IrisRecog;

import java.awt.*;
import java.util.*;
import javax.swing.*;

public class Subject
{
	/*
	 * The original image
	 */
	protected Image img_OriginalImage;
	
	/* 
	 * The grayscale image
	 */
	protected GrayscaleImage gmg_GrayscaleImage;
	
	/*
	 * The edge image (of grayscale type)
	 */
	protected GrayscaleImage emg_EdgeImage;
	
	/* 
	 * The median image
	 */
	protected MedianImage mim_MedianImage;
	
	/*
	 * The unwrapped image
	 */
	protected UnwrappedImage uwr_UnwrappedImage;
	
	/*
	 * The center of the pupil
	 */
	protected Point pnt_PupilCenter;
	
	/* 
	 * The radius of the pupil 
	 */
	protected double dbl_PupilRadius;
	
	/*
	 * The radius of the iris
	 */
	protected double dbl_IrisRadius;
	
	/*
	 * The name of the subject
	 */
	protected String str_Name;
	
	/**
	 * Creates a new subject with the specified name and image specified by the
	 * given path.
	 *
	 * @param n The name of the subject
	 * @param path The path to the eye image of the subject
	 */
	public Subject(String n, String path)
	{
		ImageIcon icn_IconImage = new ImageIcon(Toolkit.getDefaultToolkit().createImage(path));
		
		this.str_Name = n;
		this.img_OriginalImage = icn_IconImage.getImage();
		this.gmg_GrayscaleImage = new GrayscaleImage(this.img_OriginalImage);
		this.mim_MedianImage = null;
		this.uwr_UnwrappedImage = null;
		this.emg_EdgeImage = null;
		this.pnt_PupilCenter = null;
	}
	
	/**
	 * Returns the name of the subject.
	 *
	 * @return the name 
	 */
	public String getName()
	{
		return this.str_Name;
	}
	
	/**
	 * Sets the original image of the subject.
	 *
	 * @param img The original image
	 */
	public void setOriginalImage(Image img)
	{
		this.img_OriginalImage = img;
	}
	
	/**
	 * Returns the original image of the subject.
	 *
	 * @return the original image
	 */
	public Image getOriginalImage()
	{
		return this.img_OriginalImage;
	}
	
	/**
	 * Returns the grayscale image of the subject.
	 *
	 * @return the grayscale image
	 */
	public GrayscaleImage getGrayscaleImage()
	{
		return this.gmg_GrayscaleImage;
	}
	
	/**
	 * Sets the grayscale image of the subject.
	 *
	 * @param img The grayscale image
	 */
	public void setGrayscaleImage(GrayscaleImage img)
	{
		this.gmg_GrayscaleImage = img;
	}
	
	/**
	 * Returns the edge image of the subject.
	 *
	 * @return the grayscale, edge image, of the subject
	 */
	public GrayscaleImage getEdgeImage()
	{
		return this.emg_EdgeImage;
	}
	
	/**
	 * Sets the median image of the subject.
	 *
	 * @param img The median image
	 */
	public void setMedianImage(MedianImage img)
	{
		this.mim_MedianImage = img;
	}
	
	/**
	 * Returns the median image of the subject.
	 *
	 * @return the median image
	 */
	public MedianImage getMedianImage()
	{
		return this.mim_MedianImage;
	}
	
	/**
	 * Sets the edge image of the subject.
	 *
	 * @param img The grayscale, edge image
	 */
	public void setEdgeImage(GrayscaleImage img)
	{
		this.emg_EdgeImage = img;
	}
	
	/**
	 * Returns the unwrapped image of the subject.
	 *
	 * @return the unwrapped image
	 */
	public UnwrappedImage getUnwrappedImage()
	{
		return this.uwr_UnwrappedImage;
	}
	
	/**
	 * Sets the unwrapped image of the subject.
	 *
	 * @param img The unwrapped image
	 */
	public void setUnwrappedImage(UnwrappedImage img)
	{
		this.uwr_UnwrappedImage = img;
	}
	
	/** 
	 * Sets the pupil center of the subject.
	 *
	 * @param p The pupil center
	 */
	public void setPupilCenter(Point p)
	{
		this.pnt_PupilCenter = p;
	}

	/**
	 * Returns the pupil center of the subject.
	 *
	 * @return the pupil center
	 */
	public Point getPupilCenter()
	{
		return this.pnt_PupilCenter;
	}
	
	/**
	 * Returns the iris radius of the subject.
	 *
	 * @return the iris radius
	 */
	public double getIrisRadius()
	{
		return this.dbl_IrisRadius;
	}
	
	/**
	 * Sets the value of the iris radius of the subject.
	 *
	 * @param r The iris radius
	 */
	public void setIrisRadius(double r)
	{
		this.dbl_IrisRadius = r * Globals.LOCALIZATION_PERCENTAGE;
	}
	
	/**
	 * Returns the pupil radius of the subject.
	 *
	 * @return the pupil radius
	 */
	public double getPupilRadius()
	{
		return this.dbl_PupilRadius;
	}
	
	/**
	 * Sets the value of the pupil radius of the subject.
	 *
	 * @param p The pupil radius
	 */
	public void setPupilRadius(double p)
	{
		this.dbl_PupilRadius = p * (1/Globals.LOCALIZATION_PERCENTAGE);
	}
}