package de.sgd.josm.plugins.osm2x.io.svg;

import de.sgd.josm.plugins.osm2x.Osm2XConversions;

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
	 */
	public SvgCircle(SvgPoint center, double radius, String fill) {
		this(center, radius);
		this.fill = fill;
	}

	/**
	 *
	 * @param center
	 * @param radius in meters
	 */
	public SvgCircle(SvgPoint center, double radius, String stroke, double stroke_width) {
		this(center, radius);
	}

	@Override
	public String toSvg(SvgPoint documentOrigin) {
		double[] c = center.getLocalCoordinates(documentOrigin);

		String svg = String.format("<circle cx=\"%.3f\" cy=\"%.3f\" r=\"%.3f\" ", c[0], c[1], radius);

		svg = svg.concat(String.format("fill=\"%s\" stroke=\"%s\" stroke-width=\"%.3f\"", fill, stroke, stroke_width));

		return svg.concat("/>\n");
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
