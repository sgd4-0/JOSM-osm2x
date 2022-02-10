package de.sgd.josm.plugins.osm2x.io.svg;

import de.sgd.josm.plugins.osm2x.helper.Osm2XConversions;

public class SvgPoint {

	private double lat;
	private double lon;

	public SvgPoint(double lat, double lon) {
		this.lat = lat;
		this.lon = lon;
	}

	public double lat() {
		return this.lat;
	}

	public double lon() {
		return this.lon;
	}

	/**
	 * Determines if the other point has almost the same lat/lon values.
	 * @param other the other point
	 * @param eps the acceptable difference in meters
	 * @return true if the difference is smaller than the acceptable difference otherwise false
	 */
	public boolean equalsEps(SvgPoint other, double eps) {
		double[] xy = this.getLocalCoordinates(other);

		System.out.printf("Distance from %.7f, %.7f to %.7f, %.7f is %.6f, %.6f [m]\n",
				lat, lon, other.lat, other.lon, xy[0], xy[1]);

		return Math.abs(xy[0]) < eps && Math.abs(xy[1]) < eps;
	}

	/**
	 * Calculates the coordinates of the point starting from the defined origin.
	 * The positive x-axis points to the right, the positive y-axis points down.
	 * @param origin
	 * @return an array containing the distances in meters
	 */
	public double[] getLocalCoordinates(SvgPoint origin) {
		double[] xy = {0.0,0.0};

		xy[0] = (lon - origin.lon) / Osm2XConversions.METER_TO_LATLON * Math.cos((origin.lat + lat) * Math.PI/360);
		xy[1] = (origin.lat - lat) / Osm2XConversions.METER_TO_LATLON;

		return xy;
	}
}
