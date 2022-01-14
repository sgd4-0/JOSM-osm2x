package de.sgd.josm.plugins.osm2x.io;

import java.io.Closeable;
import java.io.FileWriter;
import java.io.IOException;
import java.util.List;

import org.openstreetmap.josm.data.osm.DataSet;
import org.openstreetmap.josm.data.osm.Node;
import org.openstreetmap.josm.data.osm.Way;

public class NavWriter implements Closeable {

	FileWriter writer;

	public NavWriter(FileWriter writer) {
		this.writer = writer;
	}

	public void writeDataset(DataSet ds) throws IOException {
		// TODO write dataset to file
		if (ds != null)
		{
			writer.write("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
			writer.write(String.format("<nodelist version=\"%s\">\n", ds.getVersion()));

			for (Node n : ds.getNodes())
			{
				List<Way> parentWays = n.getParentWays();
				if (parentWays.size() > 1)
				{
					writer.write(String.format("  <node id=\"%d\" lat=\"%.7f\" lon=\"%.7f\">\n",
							n.getId(), n.lat(), n.lon()));

					for (Way w : n.getParentWays())
					{
						String highway = w.get("highway");
						String surface = w.get("surface");

						for (Node nn : w.getNeighbours(n))
						{
							writer.write(String.format("    <nd ref=\"%d\">\n", nn.getId()));
							writer.write(String.format("      <highway>%s</highway>\n", highway));
							writer.write(String.format("      <surface>%s</surface>\n", surface));
							writer.write("    </nd>\n");
						}
					}

					writer.write("  </node>\n");
				}
			}

			writer.write("</nodelist>\n");
		}
	}

	@Override
	public void close() throws IOException {
		// TODO Auto-generated method stub

	}

}
