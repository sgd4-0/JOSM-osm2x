package de.sgd.josm.plugins.osm2x.io;

import java.io.Closeable;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;

import org.openstreetmap.josm.data.Bounds;
import org.openstreetmap.josm.data.coor.LatLon;
import org.openstreetmap.josm.data.osm.DataSet;
import org.openstreetmap.josm.data.osm.Node;
import org.openstreetmap.josm.data.osm.Way;
import org.openstreetmap.josm.gui.MainApplication;

import de.sgd.josm.plugins.osm2x.io.svg.SvgCircle;
import de.sgd.josm.plugins.osm2x.io.svg.SvgDocument;
import de.sgd.josm.plugins.osm2x.io.svg.SvgPath;
import de.sgd.josm.plugins.osm2x.io.svg.SvgPoint;

public class SvgWriter implements Closeable {

	FileWriter writer;
	LatLon origin;

	public SvgWriter(FileWriter writer) {
		this.writer = writer;
	}

	public void writeDataset(DataSet ds) throws IOException {
		if (ds != null)
		{
			Bounds pb = MainApplication.getMap().mapView.getRealBounds();

			System.out.printf("RealBounds: %.7f %.7f %.7f %.7f\n", pb.getMinLat(), pb.getMaxLat(), pb.getMinLon(), pb.getMaxLon());

			//this.origin = new LatLon(pb.maxNorth, pb.minEast);
			this.origin = new LatLon(pb.getMaxLat(), pb.getMinLon());
			System.out.println("Origin coordinates: " + origin.toDisplayString());

			SvgDocument doc = new SvgDocument();


			// create svg file from layer
			for (Node n : ds.getNodes())
			{
				// if node is part of a way skip this node
				if (!n.isDeleted() && n.hasKey("barrier", "natural"))
				{
					SvgCircle c = new SvgCircle(new SvgPoint(n.lat(), n.lon()), 0.1, "black");
					doc.addGeometry(c);
				}
			}

			for (Way w : ds.getWays())
			{
				ArrayList<SvgPoint> waypoints = new ArrayList<>();
				for (Node n : w.getNodes()) {
					waypoints.add(new SvgPoint(n.lat(), n.lon()));
				}
				if (waypoints.size() < 2) continue;

				if (w.isClosed()) {
					// create closed path with fill
					doc.addGeometry(new SvgPath(waypoints, "black"));
				} else {
					// create path with stroke
					doc.addGeometry(new SvgPath(waypoints, "black", 0.2));
				}
			}

			doc.writeToFile(writer);
		}
	}

	@Override
	public void close() throws IOException {
		// TODO Auto-generated method stub
		writer.close();
	}
}
