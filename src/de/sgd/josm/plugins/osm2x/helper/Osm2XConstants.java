package de.sgd.josm.plugins.osm2x.helper;

public class Osm2XConstants {
	private Osm2XConstants() {}

	/**
	 * Icons
	 */
	public static final String ICON_PREF_48 = "osm2x_48.png";

	/**
	 * Path to file with export settings
	 */
	public static final String PREF_EXPORT_PREF_FILE = "osm2x.exportpreffile";

	/**
	 * Interpolation distance
	 */
	public static final String PREF_INTERP_DIST = "osm2x.interpdist";
	public static final double DEFAULT_INTERP_DIST = 5.0;

	/**
	 * Filter for barrier dataset
	 */
	public static final String PREF_BARRIER_FILTER = "osm2x.barrierfilter";
	public static final String DEFAULT_BARRIER_FILTER = "barrier=* | natural=* | building=*";

	/**
	 * Whether to copy address to node or not
	 */
	public static final String PREF_COPY_ADDRESS = "osm2x.copyaddr";
	public static final boolean DEFAULT_COPY_ADDRESS = true;

	/**
	 * Filter for highway dataset
	 */
	public static final String PREF_HIGHWAY_FILTER = "osm2x.highwayfilter";
	public static final String DEFAULT_HIGHWAY_FILTER = "highway=*";
}
