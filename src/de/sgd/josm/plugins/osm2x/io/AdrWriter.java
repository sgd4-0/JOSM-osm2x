package de.sgd.josm.plugins.osm2x.io;

import java.io.Closeable;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.TreeMap;

import org.openstreetmap.josm.data.osm.DataSet;
import org.openstreetmap.josm.data.osm.Node;

public class AdrWriter implements Closeable {

	// define some formattings
	private static final String TAB = "    ";
	private static final String NODE_FORMAT = TAB + TAB + "{\"address\": \"%s\", \"nodes\": [%s]}";

	FileWriter writer;

	public AdrWriter(FileWriter writer) {
		this.writer = writer;
	}

	public void writeDataset(DataSet ds) throws IOException {
		// write dataset to file
		if (ds != null)
		{
			TreeMap<String, ArrayList<Long>> adresses = new TreeMap<>();
			for (Node n : ds.getNodes())
			{
				if (isNodeComplete(n)) {
					String adr = String.format("%s %s", n.get("addr:street"), n.get("addr:housenumber"));

					if (adresses.containsKey(adr)) {
						adresses.get(adr).add(n.getId());
					} else {
						ArrayList<Long> vals = new ArrayList<>();
						vals.add(n.getId());
						adresses.put(adr, vals);
					}
				}
			}

			// write json to file
			writer.write("{\n");
			writer.write(TAB + "\"addresslist\": [\n");
			boolean isFirst = true;
			for (String key : adresses.keySet()) {
				if (!isFirst)
				{
					writer.write(",\n");
				}
				writer.write(String.format(NODE_FORMAT, key, arrToString(adresses.get(key))));
				isFirst = false;
			}
			writer.write("\n" + TAB + "]\n");
			writer.write("}\n");
		}
	}

	@Override
	public void close() throws IOException {
		writer.close();
	}

	private boolean isNodeComplete(Node n) {
		return n.hasTag("entrance") && n.hasTag("addr:street") && n.hasTag("addr:housenumber")
				&& !n.isDeleted() && (n.getParentWays().size() > 0);
	}

	private String arrToString(List<Long> list) {
		String line = "";
		for (long i : list) {
			line = line.concat("\"" + Long.toString(i) + "\", ");
		}
		return line.substring(0,line.length()-2);
	}
}
