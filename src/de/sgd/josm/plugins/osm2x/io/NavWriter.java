package de.sgd.josm.plugins.osm2x.io;

import java.io.Closeable;
import java.io.FileWriter;
import java.io.IOException;
import java.util.List;

import org.openstreetmap.josm.data.osm.DataSet;
import org.openstreetmap.josm.data.osm.Node;
import org.openstreetmap.josm.data.osm.Way;

public class NavWriter implements Closeable {

	// define formats
	private final String tab = "  ";
	private final String xml_node = tab + "<node id=\"%d\" lat=\"%.7f\" lon=\"%.7f\">\n";
	private final String tag_node = tab + tab + "<%s>%s</%1$s>\n";
	private final String xml_nd = tab + tab + "<nd ref=\"%d\">\n";
	private final String tag_nd = tab + tab + tab + "<%s>%s</%1$s>\n";

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
					writer.write(String.format(xml_node, n.getId(), n.lat(), n.lon()));
					writeNodeTag("pid", n.get("pid"), tag_node);
					writeNodeTag("barrier", n.get("barrier"), tag_node);

					for (Way w : n.getParentWays())
					{
						String highway = w.get("highway");
						String surface = w.get("surface");

						for (Node nn : w.getNeighbours(n))
						{
							writer.write(String.format(xml_nd, nn.getId()));
							writer.write(String.format(tag_nd, "highway", highway));
							writer.write(String.format(tag_nd, "surface", surface));

							writeNodeTag("angle", w.get("angle"), tag_nd);
							writer.write(tab + tab + "</nd>\n");
						}
					}

					writer.write(tab + "</node>\n");
				}
			}

			writer.write("</nodelist>\n");
		}
	}

	private void writeNodeTag(String key, String tag, String format)
	{
		if (key != null && tag != null)
		{
			try {
				writer.write(String.format(format, key, tag));
			} catch (IOException ioe) {
				System.err.println("Could not write to file");
			}
		}
	}

	@Override
	public void close() throws IOException {
		// TODO Auto-generated method stub

	}

}
