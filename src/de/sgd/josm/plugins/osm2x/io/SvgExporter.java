package de.sgd.josm.plugins.osm2x.io;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;

import org.openstreetmap.josm.actions.ExtensionFileFilter;
import org.openstreetmap.josm.data.osm.Node;
import org.openstreetmap.josm.data.osm.Way;
import org.openstreetmap.josm.gui.io.importexport.OsmExporter;
import org.openstreetmap.josm.gui.layer.OsmDataLayer;

import de.sgd.josm.plugins.osm2x.io.svg.SvgCircle;
import de.sgd.josm.plugins.osm2x.io.svg.SvgDocument;
import de.sgd.josm.plugins.osm2x.io.svg.SvgPath;
import de.sgd.josm.plugins.osm2x.io.svg.SvgPoint;

public class SvgExporter extends OsmExporter {

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
				if (!n.isDeleted() && n.hasKey("barrier", "natural"))
				{
					SvgCircle c = new SvgCircle(new SvgPoint(n.lat(), n.lon()), 0.1, "black");
					doc.addGeometry(c);
				}
			}

			// get all ways from dataset
			for (Way w : layer.getDataSet().getWays())
			{
				ArrayList<SvgPoint> waypoints = new ArrayList<>();
				for (Node n : w.getNodes()) {
					waypoints.add(new SvgPoint(n.lat(), n.lon()));
				}
				if (waypoints.size() < 2) continue;	// a way must contain at least two points

				if (w.isClosed()) {
					// create closed path with fill
					doc.addGeometry(new SvgPath(waypoints, "black"));
				} else {
					// create path with stroke
					doc.addGeometry(new SvgPath(waypoints, "black", 0.2));
				}
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
}
