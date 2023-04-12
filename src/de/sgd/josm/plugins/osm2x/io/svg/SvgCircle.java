package de.sgd.josm.plugins.osm2x.io.svg;

import de.sgd.josm.plugins.osm2x.helper.Osm2XConversions;

public class SvgCircle extends SvgShape {

	private SvgPoint center;
	private double radius;

	/**
	 *
	 * @param center
	 * @param radius in meters
	 */
	public SvgCircle(SvgPoint center, double radius) {
		this.center = center;
		this.radius = radius;
	}

	/**
	 *
	 * @param center
	 * @param radius in meters
	 * @param clss class of this shape
	 */
	public SvgCircle(SvgPoint center, double radius, String clss) {
		this(center, radius);
		this.clss = clss;
	}

	@Override
	public String toSvg(SvgPoint documentOrigin) {
		double[] c = center.getLocalCoordinates(documentOrigin);
		return String.format("<circle cx=\"%.3f\" cy=\"%.3f\" r=\"%.3f\" class=\"%s\"/>\n",
				c[0], c[1], radius, clss);
	}

	@Override
	double minLat() {
		return center.lat() - radius * Osm2XConversions.METER_TO_LATLON;
	}

	@Override
	double maxLat() {
		return center.lat() + radius * Osm2XConversions.METER_TO_LATLON;
	}

	@Override
	double minLon() {
		return center.lon() - radius * Osm2XConversions.METER_TO_LATLON / Math.cos(center.lat()*Math.PI/180);
	}

	@Override
	double maxLon() {
		return center.lon() + radius * Osm2XConversions.METER_TO_LATLON / Math.cos(center.lat()*Math.PI/180);
	}

}
