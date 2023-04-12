package de.sgd.josm.plugins.osm2x.io;

import java.io.Closeable;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import org.openstreetmap.josm.data.coor.LatLon;
import org.openstreetmap.josm.data.osm.DataSet;
import org.openstreetmap.josm.data.osm.Node;
import org.openstreetmap.josm.data.osm.Way;

public class NavWriter implements Closeable {

	// define formats
	private static final String TAB = "  ";
	private static final String XML_NODE = TAB + "<node id=\"%d\" lat=\"%.7f\" lon=\"%.7f\">\n";
	private static final String XML_NODE_PID = TAB + "<node id=\"%d\" lat=\"%.7f\" lon=\"%.7f\" pid=\"%s\">\n";
	private static final String TAG_NODE = TAB + TAB + "<%s>%s</%1$s>\n";
	private static final String XML_ND = TAB + TAB + "<nd ref=\"%d\">\n";
	private static final String TAG_ND = TAB + TAB + TAB + "<%s>%s</%1$s>\n";

	FileWriter writer;

	protected class Neighbour {
		long id;
		long pid;
		double angle;
		LatLon latlon;
		Map<String, String> tags;

		public Neighbour(long node_id, LatLon ll, Map<String, String> tags) {
			id = node_id;
			latlon = ll;
			this.tags = tags;
			pid = -1;
			angle = -1.0;
		}

		@Override
		public boolean equals(Object o)
		{
			if (this == o)
				return true;
			if (!(o instanceof Neighbour))
				return false;
			Neighbour other = (Neighbour) o;
			return this.id == other.id;
		}

		@Override
		public final int hashCode()
		{
			return (int)id;
		}

		public String to_string()
		{
			return String.format("id: %d, pid: %d, angle: %.3f", id, pid, angle);
		}
	}

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
				if (!parentWays.isEmpty())
				{
					if (n.get("pid") != null)
					{
						writer.write(String.format(XML_NODE_PID, n.getId(), n.lat(), n.lon(), n.get("pid")));
					}
					else
					{
						writer.write(String.format(XML_NODE, n.getId(), n.lat(), n.lon()));
					}
					writeNodeTag("barrier", n.get("barrier"), TAG_NODE);

					// get all neighbours
					List<Neighbour> neighbours = getNeighbours(n);

					for (Neighbour nn : neighbours)
					{
						String highway = nn.tags.get("highway");
						String surface = nn.tags.get("surface");

						writer.write(String.format(XML_ND, nn.id));
						writer.write(String.format(TAG_ND, "highway", highway));
						writer.write(String.format(TAG_ND, "surface", surface));
						writer.write(TAB + TAB + "</nd>\n");
					}
					writer.write(TAB + "</node>\n");
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

	/**
	 * Get a list with all neighbours for the Node n
	 * @param n
	 * @return
	 */
	private List<Neighbour> getNeighbours(Node n)
	{
		List<Neighbour> neighbours = new ArrayList<>();
		for (Way w : n.getParentWays())
		{
			// get neighbouring nodes
			// create class Neighbour
			String angle = w.get("angle");
			for (Node nd : w.getNeighbours(n))
			{
				Neighbour nb = new Neighbour(nd.getOsmId(), nd.getCoor(), w.getKeys());
				if (angle != null)
				{
					nb.angle = Double.valueOf(angle);
				}

				String pid = nd.get("pid");
				if (pid != null)
				{
					nb.pid = Long.valueOf(pid);
				}

				// check if neighbour already exists
				if (neighbours.contains(nb) && (nb.angle > -1.0))
				{
					neighbours.remove(nb);
				}
				neighbours.add(nb);
			}
		}
		return neighbours;
	}

	//	/**
	//	 * Search for node with id in set.
	//	 * @param neighbours the set to search in
	//	 * @param id the id of the node
	//	 * @return the node if one is found, otherwise null
	//	 */
	//	private LatLon getNeighbour(List<Neighbour> neighbours, long id)
	//	{
	//		for (Neighbour n : neighbours)
	//		{
	//			if (n.id == id)
	//			{
	//				return n.latlon;
	//			}
	//		}
	//		//System.out.println("Could not find a node with id " + id);
	//		return null;
	//	}
	//
	//	/**
	//	 *
	//	 * @param neighbours
	//	 * @param pid
	//	 * @return
	//	 */
	//	private LatLon getNeighbourWithPID(List<Neighbour> neighbours, long pid)
	//	{
	//		for (Neighbour n : neighbours)
	//		{
	//			if (n.pid == pid)
	//			{
	//				return n.latlon;
	//			}
	//		}
	//		//System.out.println("Could not find a node with pid " + pid);
	//		return null;
	//	}

	@Override
	public void close() throws IOException {
		writer.close();

	}

}
