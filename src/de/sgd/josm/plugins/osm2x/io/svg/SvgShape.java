package de.sgd.josm.plugins.osm2x.io.svg;

public abstract class SvgShape {

	String fill = "none";
	String stroke = "none";
	double stroke_width = 0.0;

	abstract String toSvg(SvgPoint documentOrigin);

	abstract double minLat();
	abstract double maxLat();
	abstract double minLon();
	abstract double maxLon();

	/**
	 * Set the fill color of the shape.
	 * @param color
	 */
	void setFill(String color) {
		this.fill = color;
	}

	/**
	 * Set stroke with specified color and width.
	 * @param color the color as string
	 * @param width the width in mm
	 */
	void setStroke(String color, double width) {
		this.stroke = color;
		this.stroke_width = width;
	}

}
