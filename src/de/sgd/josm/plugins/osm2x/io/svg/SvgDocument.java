package de.sgd.josm.plugins.osm2x.io.svg;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Writer;
import java.util.ArrayList;

import org.openstreetmap.josm.data.coor.EastNorth;
import org.openstreetmap.josm.data.coor.LatLon;
import org.openstreetmap.josm.data.projection.ProjectionRegistry;

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
		SvgPoint documentOrigin = new SvgPoint(maxLat, minLon);		// svg origin is in top-left corner
		System.out.println("Document origin is at: " + maxLat + ", " + minLon);

		LatLon origin = new LatLon(maxLat, minLon);
		System.out.println("Document origin from latlon: " + origin.getX() + ", " + origin.getY());


		EastNorth en = origin.getEastNorth(ProjectionRegistry.getProjection());

		System.out.println("Document origin in EastNorth: " + en.getX() + ", " + en.getY());

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

		// write css style to svg document
		try (InputStream in = SvgDocument.class.getResourceAsStream("/data/svg_style.css");
				BufferedReader cssReader = new BufferedReader(new InputStreamReader(in)))
		{
			writer.write("  <style>\n");
			String cssline;
			while ((cssline = cssReader.readLine()) != null) {
				writer.write("    " + cssline + "\n");
			}
			writer.write("  </style>\n");
		} catch (IOException e) {
			e.printStackTrace();
		}

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
