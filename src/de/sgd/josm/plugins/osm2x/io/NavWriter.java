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
	private final String tab = "  ";
	private final String xml_node = tab + "<node id=\"%d\" lat=\"%.7f\" lon=\"%.7f\">\n";
	private final String xml_node_pid = tab + "<node id=\"%d\" lat=\"%.7f\" lon=\"%.7f\" pid=\"%s\">\n";
	private final String tag_node = tab + tab + "<%s>%s</%1$s>\n";
	private final String xml_nd = tab + tab + "<nd ref=\"%d\">\n";
	private final String tag_nd = tab + tab + tab + "<%s>%s</%1$s>\n";

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
				if (parentWays.size() >= 1)
				{
					if (n.get("pid") != null)
					{
						writer.write(String.format(xml_node_pid, n.getId(), n.lat(), n.lon(), n.get("pid")));
					}
					else
					{
						writer.write(String.format(xml_node, n.getId(), n.lat(), n.lon()));
					}
					writeNodeTag("barrier", n.get("barrier"), tag_node);

					// get all neighbours
					List<Neighbour> neighbours = getNeighbours(n);

					for (Neighbour nn : neighbours)
					{
						String highway = nn.tags.get("highway");
						String surface = nn.tags.get("surface");

						writer.write(String.format(xml_nd, nn.id));
						writer.write(String.format(tag_nd, "highway", highway));
						writer.write(String.format(tag_nd, "surface", surface));

						//						if (nn.angle > -1.0)
						//						{
						//							writer.write(String.format(tag_nd, "angle", nn.angle));
						//						}
						//						else
						//						{
						//							LatLon p;
						//							if (nn.pid > 0)
						//							{
						//								p = getNeighbour(neighbours, nn.pid);
						//							} else
						//							{
						//								p = getNeighbourWithPID(neighbours, nn.id);
						//							}
						//							if (p != null)
						//							{
						//								// angle not present -> calculate angle
						//								double ang1 = n.getCoor().bearing(nn.latlon);
						//								double ang2 = n.getCoor().bearing(p);	// this is the base angle
						//								double dang = ang2 - ang1;						// delta angle
						//								dang = Math.abs(dang) > Math.PI ? 2.0*Math.PI - Math.abs(dang) : dang;
						//								writer.write(String.format(tab + tab + tab + "<%s>%.6f</%1$s>\n", "angle", dang));
						//							}
						//						}
						writer.write(tab + tab + "</nd>\n");

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
				if (neighbours.contains(nb))
				{
					// check if nb has angle property set
					if (nb.angle > -1.0)
					{
						neighbours.remove(nb);
					}
				}

				neighbours.add(nb);
			}
		}
		return neighbours;
	}

	/**
	 * Search for node with id in set.
	 * @param neighbours the set to search in
	 * @param id the id of the node
	 * @return the node if one is found, otherwise null
	 */
	private LatLon getNeighbour(List<Neighbour> neighbours, long id)
	{
		for (Neighbour n : neighbours)
		{
			if (n.id == id)
			{
				return n.latlon;
			}
		}
		//System.out.println("Could not find a node with id " + id);
		return null;
	}

	/**
	 *
	 * @param neighbours
	 * @param pid
	 * @return
	 */
	private LatLon getNeighbourWithPID(List<Neighbour> neighbours, long pid)
	{
		for (Neighbour n : neighbours)
		{
			if (n.pid == pid)
			{
				return n.latlon;
			}
		}
		//System.out.println("Could not find a node with pid " + pid);
		return null;
	}

	@Override
	public void close() throws IOException {
		writer.close();

	}

}
