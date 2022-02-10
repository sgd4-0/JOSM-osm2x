package de.sgd.josm.plugins.osm2x.io.svg;

import java.io.IOException;
import java.io.Writer;
import java.util.ArrayList;

public class SvgDocument {

	ArrayList<SvgShape> shapes;

	double minLon, minLat, maxLon, maxLat;

	public SvgDocument() {
		shapes = new ArrayList<>();

		// set lon/lat to max values
		minLon = 180;
		maxLon = -180;
		minLat = 90;
		maxLat = -90;
	}

	public void addGeometry(SvgShape geometry) {
		shapes.add(geometry);

		if (geometry.minLon() < minLon) {
			minLon = geometry.minLon();
		} else if (geometry.maxLon() > maxLon) {
			maxLon = geometry.maxLon();
		}

		if (geometry.minLat() < minLat) {
			minLat = geometry.minLat();
		} else if (geometry.maxLat() > maxLat) {
			maxLat = geometry.maxLat();
		}
	}

	public void writeToFile(Writer writer) throws IOException {
		// Origin for svg/inkscape
		SvgPoint documentOrigin = new SvgPoint(maxLat, minLon);
		double[] xy = getDocSize();

		// write head
		writer.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
		String line = String.format("<svg width=\"%dpx\" "
				+ "height=\"%dpx\" "
				+ "viewport=\"0 0 %1$d %2$d\" "
				+ "version=\"1.1\" "
				+ "xmlns=\"http://www.w3.org/2000/svg\" "
				+ "xmlns:svg=\"http://www.w3.org/2000/svg\">\n",
				(int)Math.ceil(xy[0]), (int)Math.ceil(xy[1]));
		writer.write(line);
		writer.write(String.format("  <desc>Document origin (bottom-left corner): lat=%.7f, lon=%.7f</desc>\n\n",
				minLat, minLon));

		// write body
		String indent = "  ";
		for (SvgShape shp : shapes) {
			writer.write(indent + shp.toSvg(documentOrigin));
		}

		writer.write("</svg>\n");
	}

	/**
	 * Get the document size in meters.
	 * @return array [width, height] in meters
	 */
	public double[] getDocSize() {
		SvgPoint documentOrigin = new SvgPoint(maxLat, minLon);
		SvgPoint pnt = new SvgPoint(minLat, maxLon);
		return pnt.getLocalCoordinates(documentOrigin);
	}

	/**
	 * Get the global coordinates of the bottom left corner of the map.
	 * @return the global coordinates of the bottom left corner of the map.
	 */
	public double[] getMapOrigin() {
		double[] d = {minLat, minLon};
		return d;
	}
}
