package de.sgd.josm.plugins.osm2x.io.svg;

public abstract class SvgShape {

	String clss = "none";

	abstract String toSvg(SvgPoint documentOrigin);

	abstract double minLat();
	abstract double maxLat();
	abstract double minLon();
	abstract double maxLon();

	/**
	 * Set the fill color of the shape.
	 * @param color
	 */
	void setClass(String clss) {
		this.clss = clss;
	}
}
