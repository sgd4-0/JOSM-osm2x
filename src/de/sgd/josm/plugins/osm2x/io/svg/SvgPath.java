package de.sgd.josm.plugins.osm2x.io.svg;

import java.util.ArrayList;
import java.util.List;

public class SvgPath extends SvgShape {

	boolean isClosed;
	List<SvgPoint> points;

	public SvgPath(List<SvgPoint> points) {
		this.points = new ArrayList<>(points);

		isClosed = points.get(0).equalsEps(points.get(points.size()-1), 1E-3);
	}

	/**
	 *
	 * @param points
	 * @param clss
	 */
	public SvgPath(List<SvgPoint> points, String clss, boolean isClosed) {
		this(points);
		this.isClosed = isClosed;
		this.clss = clss;
	}

	@Override
	String toSvg(SvgPoint documentOrigin) {
		double[] xy = points.get(0).getLocalCoordinates(documentOrigin);
		String svg = String.format("<path d=\"M %.3f,%.3f L ", xy[0], xy[1]);

		for (int i = 1; i < points.size(); i++) {
			xy = points.get(i).getLocalCoordinates(documentOrigin);
			svg = svg.concat(String.format("%.3f,%.3f ", xy[0], xy[1]));
		}

		svg = isClosed ? svg.concat("z\" ") : svg.concat("\" ");
		svg = svg.concat(String.format("class=\"%s\"/>\n", clss));

		return svg;
	}

	@Override
	double minLat() {
		double lat = points.get(0).lat();
		for (SvgPoint p : points) {
			lat = p.lat() < lat ? p.lat() : lat;
		}
		return lat;
	}

	@Override
	double maxLat() {
		double lat = points.get(0).lat();
		for (SvgPoint p : points) {
			lat = p.lat() > lat ? p.lat() : lat;
		}
		return lat;
	}

	@Override
	double minLon() {
		double lon = points.get(0).lon();
		for (SvgPoint p : points) {
			lon = p.lon() < lon ? p.lon() : lon;
		}
		return lon;
	}

	@Override
	double maxLon() {
		double lon = points.get(0).lon();
		for (SvgPoint p : points) {
			lon = p.lon() > lon ? p.lon() : lon;
		}
		return lon;
	}
}
