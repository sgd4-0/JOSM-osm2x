package de.sgd.josm.plugins.osm2x.io;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Deque;

import org.openstreetmap.josm.actions.ExtensionFileFilter;
import org.openstreetmap.josm.data.coor.LatLon;
import org.openstreetmap.josm.data.osm.Node;
import org.openstreetmap.josm.data.osm.OsmPrimitive;
import org.openstreetmap.josm.data.osm.Way;
import org.openstreetmap.josm.gui.io.importexport.OsmExporter;
import org.openstreetmap.josm.gui.layer.OsmDataLayer;

import de.sgd.josm.plugins.osm2x.helper.Osm2XConversions;
import de.sgd.josm.plugins.osm2x.io.svg.SvgCircle;
import de.sgd.josm.plugins.osm2x.io.svg.SvgDocument;
import de.sgd.josm.plugins.osm2x.io.svg.SvgPath;
import de.sgd.josm.plugins.osm2x.io.svg.SvgPoint;

public class SvgExporter extends OsmExporter {

	private String naturalKey = "natural";
	private String barrierKey = "barrier";

	public SvgExporter() {
		super(new ExtensionFileFilter("svg", "svg", "Scalable Vector Graphics (SGD) (*.svg)"));
	}

	@Override
	protected void doSave(File file, OsmDataLayer layer) throws IOException {
		// Save map data to *.svg file and create .yaml config file
		String svg_path = file.getAbsolutePath();
		if (!svg_path.endsWith(".svg"))
		{
			svg_path = svg_path.concat(".svg");
		}

		// get data from map and convert to svg
		SvgDocument doc = new SvgDocument();
		layer.data.getReadLock().lock();
		try {
			// get all nodes contained in dataset
			for (Node n : layer.getDataSet().getNodes())
			{
				// if node is part of a way skip this node. It will be added with the way
				if (!n.isDeleted() && !n.isIncomplete() && n.getParentWays().isEmpty())
				{
					// create circle
					SvgCircle c = new SvgCircle(new SvgPoint(n.getCoor()), 0.1, getClass(n));
					doc.addGeometry(c);
				}
			}

			// get all ways from dataset
			for (Way w : layer.getDataSet().getWays())
			{
				if (w.getNodesCount() < 2) continue;	// a way must contain at least two points

				ArrayList<SvgPoint> waypoints = new ArrayList<>();
				if (w.isClosed())	// TODO close path depending on class
				{
					// copy waypoints to svg shape
					for (Node n : w.getNodes()) {
						waypoints.add(new SvgPoint(n.getCoor()));
					}
				}
				else
				{
					// compute new points
					Deque<SvgPoint> tmpPnts = new ArrayDeque<>();	// deque used as lifo stack

					LatLon prevLL = w.getNodes().get(0).getCoor();
					double prevBearing = prevLL.bearing(w.getNodes().get(1).getCoor());
					// calculate first point
					double[] offsets = computeOffsetPoint(prevLL, prevBearing + Math.PI/2, 0.15);
					waypoints.add(new SvgPoint(prevLL.lat() + offsets[0], prevLL.lon() + offsets[1]));
					tmpPnts.addLast(new SvgPoint(prevLL.lat() - offsets[0], prevLL.lon() - offsets[1]));

					LatLon thisLL = null;
					LatLon nextLL = null;
					for (int i = 1; i < w.getNodesCount(); i++)
					{
						thisLL = w.getNodes().get(i).getCoor();

						if (i+1 < w.getNodesCount())
						{
							nextLL = w.getNodes().get(i+1).getCoor();
							double bearing = thisLL.bearing(nextLL);

							double bearDiff = (prevBearing-bearing) < -Math.PI ? (prevBearing - bearing + 2*Math.PI)/2 : (prevBearing-bearing)/2;

							// TODO: get width from attribute or preferences
							offsets = computeOffsetPoint(thisLL, prevBearing-bearDiff + Math.PI/2, 0.15/Math.cos(bearDiff));
							waypoints.add(new SvgPoint(thisLL.lat() + offsets[0], thisLL.lon() + offsets[1]));
							tmpPnts.addLast(new SvgPoint(thisLL.lat() - offsets[0], thisLL.lon() - offsets[1]));

							prevBearing = bearing;
						}
						else
						{
							// thisLL is the last node in this way
							offsets = computeOffsetPoint(thisLL, prevBearing + Math.PI/2, 0.15);
							waypoints.add(new SvgPoint(thisLL.lat() + offsets[0], thisLL.lon() + offsets[1]));
							tmpPnts.addLast(new SvgPoint(thisLL.lat() - offsets[0], thisLL.lon() - offsets[1]));
						}
					}

					while (!tmpPnts.isEmpty())
					{
						// add buffered waypoints in reverse order
						waypoints.add(tmpPnts.removeLast());
					}
				}

				doc.addGeometry(new SvgPath(waypoints, getClass(w), true));		// war vorher w.isClosed()
			}
		} finally {
			// unlock layer
			layer.data.getReadLock().unlock();
		}

		try (FileWriter svgWriter = new FileWriter(svg_path);
				SvgWriter w = new SvgWriter(svgWriter);)
		{
			w.writeDataset(doc);
		}

		String yaml_path = svg_path.replaceFirst(".svg", ".yaml");
		try (FileWriter writer = new FileWriter(yaml_path);
				YamlWriter w = new YamlWriter(writer);)
		{
			w.writeDataset(doc);
		}
	}

	private String getClass(OsmPrimitive osmPrimitive)
	{
		if (osmPrimitive.hasKey(naturalKey))
		{
			return naturalKey + "-" + osmPrimitive.get(naturalKey);
		}
		else if (osmPrimitive.hasKey(barrierKey))
		{
			return barrierKey + "-" + osmPrimitive.get(barrierKey);
		}
		else if (osmPrimitive.hasKey("building"))
		{
			return "building";
		}
		else
		{
			return "";
		}
	}

	/**
	 *
	 * @param bearing
	 * @param distance
	 * @return
	 */
	public double[] computeOffsetPoint(LatLon latlon, double bearing, double distance)
	{
		double[] laLo = {0.0,0.0};
		// compute offsets
		laLo[0] = Math.cos(bearing)*distance * Osm2XConversions.METER_TO_LATLON;
		laLo[1] = Math.sin(bearing)*distance * Osm2XConversions.METER_TO_LATLON / Math.cos(latlon.lat() * Math.PI/180);

		return laLo;
	}
}
