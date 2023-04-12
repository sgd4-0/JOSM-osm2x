package de.sgd.josm.plugins.osm2x.helper;

import org.openstreetmap.josm.data.coor.LatLon;

public class Osm2XConversions {
	private Osm2XConversions() {}

	/**
	 * Factor to convert meters to lat/lon. To get lat/lon from meters write
	 * <code>latlon = meters * METER_TO_LATLON</code>.
	 */
	public static final double METER_TO_LATLON = 1/(Math.PI/180 * 6378137.0);

	/**
	 * Factor to convert lat/lon to meters. To get meters from lat/lon write
	 * <code>meters = latlon * LATLON_TO_METER</code>.
	 */
	public static final double LATLON_TO_METER = (Math.PI/180 * 6378137.0);


	/**
	 * Calculate node position from origin in distance with angle
	 * @param origin
	 * @param angle
	 * @param distance
	 * @return
	 */
	public static LatLon calculatePositionFrom(LatLon origin, double angle, double distance) {
		return new LatLon(origin.lat() + Math.cos(angle) * distance * METER_TO_LATLON
				, origin.lon() + Math.sin(angle) * distance / LATLON_TO_METER / Math.cos(origin.lat()/180*Math.PI) );
	}
}
