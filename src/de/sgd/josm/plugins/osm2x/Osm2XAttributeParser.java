package de.sgd.josm.plugins.osm2x;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.Collection;
import java.util.HashMap;

import org.openstreetmap.josm.data.osm.DataSet;
import org.openstreetmap.josm.data.osm.Node;
import org.openstreetmap.josm.data.osm.OsmPrimitive;
import org.openstreetmap.josm.data.osm.Way;

public class Osm2XAttributeParser {

	private HashMap<String, HashMap<String, String>> attributes;

	public Osm2XAttributeParser() throws IOException {
		// TODO load parse table from file
		attributes = new HashMap<>();

		System.out.println("current directory: " + System.getProperty("user.dir"));

		try (InputStream in = Osm2XAttributeParser.class.getResourceAsStream("/data/parsing.yaml");
				BufferedReader yaml_reader = new BufferedReader(new InputStreamReader(in)))
		{
			String line;
			String name = null;
			HashMap<String, String> attr = null;
			while ((line = yaml_reader.readLine()) != null) {
				if (line.startsWith("#")) {
					continue;
				}

				if (!line.startsWith("  ")) {
					if (attr != null) {
						System.out.println("Add map " + name + " with " + attr.size() + " attributes.");
						attributes.put(name, attr);
					}

					attr = new HashMap<>();
					name = line.trim().substring(0, line.trim().lastIndexOf(":"));
				} else {
					String[] s = line.split(":");
					attr.put(s[0].trim(), s[1].trim());
				}
			}
		}
	}

	public DataSet parseNodes(Collection<Node> nodes) {
		DataSet ds = new DataSet();

		for (Node n : nodes)
		{
			if (attributes.containsKey(n.get("name"))) {
				// create new node
				Node new_n = new Node(n);
				new_n.setKeys(copyKeys(n));
				ds.addPrimitive(new_n);
			}
		}
		return ds;
	}

	public DataSet parseWays(Collection<Way> ways) {
		DataSet ds = new DataSet();

		for (Way w : ways)
		{
			if (attributes.containsKey(w.get("name"))) {
				// create new node
				Way new_w = new Way();
				for (Node n : w.getNodes()) {
					Node nn = new Node(n);
					if (!ds.containsNode(nn)) {
						ds.addPrimitive(nn);
					} else {
						for (Node ds_node : ds.getNodes()) {
							if (ds_node.getCoor().equalsEpsilon(nn.getCoor())) {
								nn = ds_node;
								break;
							}
						}
					}

					new_w.addNode(nn);
				}

				new_w.setKeys(copyKeys(w));
				ds.addPrimitive(new_w);
			}
		}
		return ds;

	}

	private HashMap<String, String> copyKeys(OsmPrimitive p) {
		HashMap<String, String> keys = attributes.get(p.get("name"));

		// copy old keys to new keyset
		for (String key : p.getKeys().keySet()) {
			if (keys.containsKey(key) && keys.get(key).startsWith("<")) {
				keys.put(key, p.get(key));
			}
		}


		return keys;
	}
}
