package de.sgd.josm.plugins.osm2x.modules;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.TreeMap;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.openstreetmap.josm.data.coor.LatLon;
import org.openstreetmap.josm.data.osm.DataSet;
import org.openstreetmap.josm.data.osm.Node;
import org.openstreetmap.josm.data.osm.Way;

import de.sgd.josm.plugins.osm2x.helper.IdGenerator;
import de.sgd.josm.plugins.osm2x.helper.Osm2XConversions;
import de.sgd.josm.plugins.osm2x.helper.Osm2XNodeList;

public class Osm2XMesher {

	private Pattern str_to_double;

	private IdGenerator idGen;

	private DataSet original_ds;
	private DataSet interpolated_ds;
	private DataSet ds = null;

	/**
	 *
	 * @param ds the dataset to edit
	 */
	public Osm2XMesher(DataSet ds) {
		this.original_ds = ds;
		this.str_to_double = Pattern.compile("(\\d+\\.?\\d*)");

		// Check if all paths have the width attribute
		boolean width_present = true;
		for (Way w : ds.getWays()) {
			if (!w.hasTag("width")) {
				w.setHighlighted(true);
				width_present = false;
			}
		}

		if (!width_present) {
			this.ds = ds;
		}
	}

	/**
	 * Check if the dataset contains all required attributes.
	 * @return true if all required attributes are present
	 */
	public boolean isDatasetComplete() {
		// Check if all paths have the width attribute
		boolean width_present = true;
		for (Way w : ds.getWays()) {
			if (!w.hasTag("width") && !w.isDeleted()) {
				width_present = false;
			}
		}
		return width_present;
	}

	public void interpolate() {
		DataSet ds = new DataSet(this.original_ds);
		this.idGen = new IdGenerator(ds);

		System.out.println("Start interpolation");

		ds.beginUpdate();
		try
		{
			if (ds != null) {
				for (Way w : ds.getWays()) {

					for (int i = 1; i < w.getRealNodesCount(); i++) {
						Node last_node = w.getNode(i-1);
						Node curr_node = w.getNode(i);

						double dist = last_node.getCoor().greatCircleDistance(curr_node.getCoor());

						if (dist > 5.0) {	// TODO threshold from properties
							int add_nodes = (int) Math.ceil(dist / 5);

							int k = 0;
							for (k = 1; k < add_nodes; k++) {
								// Calculate new latlon and node
								LatLon ll = last_node.getCoor().interpolate(curr_node.getCoor(), (double)k/add_nodes);
								Node nn = new Node(ll);
								nn.setOsmId(this.idGen.generateID(), 1);
								// add new node to dataset and way
								ds.addPrimitive(nn);
								w.addNode(i+k-1, nn);
							}
							i += k-1;
						}
					}
				}
			}
		} catch (Exception e) {
			System.out.println(e.getMessage());
		} finally
		{
			ds.endUpdate();
		}
		this.interpolated_ds = ds;
	}

	public void createRoadNetwork() {
		// get all ways -> ignore areas, but with warning
		// ignore ways without width attribute
		// ignore ways with attribute mesh=false
		DataSet ds = (this.interpolated_ds == null) ? new DataSet(original_ds) : new DataSet(interpolated_ds);
		this.idGen = new IdGenerator(ds);

		System.out.println("Start meshing");

		ds.beginUpdate();
		try
		{
			HashMap<Node, Osm2XNodeList> addedNodes = new HashMap<>();
			List<Way> addedWays = new ArrayList<>();

			for (Node n : ds.getNodes()) {
				Osm2XNodeList nodes = new Osm2XNodeList();

				if (n.isConnectionNode()) {
					// node connects two or more ways
					TreeMap<Double, Way> angles = getNeighbourNodes(n);
					Entry<Double, Way> last_entry = null;
					for (Entry<Double, Way> entry : angles.entrySet()) {
						if (last_entry == null) {
							last_entry = entry;
							continue;
						}

						Node new_node = new Node(calculateNode(n.getCoor(), last_entry, entry));
						new_node.setOsmId(this.idGen.generateID(), 1);
						nodes.add(new_node, n.getCoor().bearing(new_node.getCoor()));
						last_entry = entry;
					}

					Node new_node = new Node(calculateNode(n.getCoor(), last_entry, angles.firstEntry()));
					new_node.setOsmId(this.idGen.generateID(), 1);
					nodes.add(new_node, n.getCoor().bearing(new_node.getCoor()));
					addedNodes.put(n, nodes);
				}
			}

			// compute parallel ways
			List<Node> more_nodes = new ArrayList<>();
			List<Way> more_ways = new ArrayList<>();
			for (Way w : ds.getWays()) {
				if (w.getRealNodesCount() < 1) continue;

				double width = 0;
				if (w.get("width") != null) {
					width = parseDouble(w.get("width"));
				} else {
					continue;
				}
				// TODO filtern??
				Way way_l = new Way(0);
				way_l.setKeys(w.getKeys());
				Map<String, String> keys = way_l.getKeys();
				keys.put("left", "true");
				way_l.setKeys(keys);

				Way con_l1 = new Way(0);
				con_l1.setKeys(way_l.getKeys());
				con_l1.addNode(w.getNode(0));

				Way con_l2 = new Way(0);
				con_l2.setKeys(way_l.getKeys());

				// ways for rights side
				Way way_r = new Way(0);
				way_r.setKeys(w.getKeys());

				Way con_r1 = new Way(0);
				con_r1.setKeys(w.getKeys());
				con_r1.addNode(w.getNode(0));

				Way con_r2 = new Way(0);
				con_r2.setKeys(w.getKeys());

				Way[] con_r = {con_r1, con_r2};
				Way[] con_l = {con_l1, con_l2};

				if (addedNodes.containsKey(w.getNode(0)) && w.getNodesCount() > 1) {
					double ang1 = w.getNode(0).getCoor().bearing(w.getNode(1).getCoor());
					double ang2 = ang1 > Math.PI ? ang1 - Math.PI : ang1 + Math.PI;

					List<Node> r_nodes = addedNodes.get(w.getNode(0)).getRightNodesWithinAngle(ang1, ang2);
					for (Node n : r_nodes) {
						way_r.addNode(n);
					}
					if (r_nodes.size() > 0) {
						con_r[1].addNode(r_nodes.get(r_nodes.size()-1));
					}

					List<Node> l_nodes = addedNodes.get(w.getNode(0)).getLeftNodesWithinAngle(ang1, ang2);
					for (Node n : l_nodes) {
						way_l.addNode(n);
					}
					if (l_nodes.size() > 0) {
						con_l[1].addNode(l_nodes.get(l_nodes.size()-1));
					}
				}

				for (int i = 1; i < w.getNodesCount()-1; i++) {
					Node n_i = w.getNode(i);
					double ang1 = n_i.getCoor().bearing(w.getNode(i+1).getCoor());
					double ang2 = n_i.getCoor().bearing(w.getNode(i-1).getCoor());

					if (addedNodes.containsKey(n_i)) {
						// add nodes to way
						for (Node n : addedNodes.get(n_i).getRightNodesWithinAngle(ang1, ang2)) {
							way_r.addNode(n);
							con_r[(i+1) % 2].addNode(n);
						}

						for (Node n : addedNodes.get(n_i).getLeftNodesWithinAngle(ang1, ang2)) {
							way_l.addNode(n);
							con_l[(i+1) % 2].addNode(n);
						}

						con_r[i % 2].addNode(n_i);
						con_l[i % 2].addNode(n_i);
					} else {
						double ang = (ang1 > ang2) ? (ang1 + ang2) / 2 + Math.PI : (ang1 + ang2) / 2;

						// Way on the right side
						Node new_node = new Node(Osm2XConversions.calculatePositionFrom(n_i.getCoor(), ang, width/3));
						new_node.setOsmId(this.idGen.generateID(), 1);
						new_node.setKeys(n_i.getKeys());
						more_nodes.add(new_node);
						way_r.addNode(new_node);

						// Way on the left side
						Node new_node1 = new Node(Osm2XConversions.calculatePositionFrom(n_i.getCoor(), ang + Math.PI, width/3));
						new_node1.setOsmId(this.idGen.generateID(), 1);
						new_node1.setKeys(n_i.getKeys());
						more_nodes.add(new_node1);
						way_l.addNode(new_node1);

						con_r[(i+1) % 2].addNode(new_node);
						con_r[i % 2].addNode(n_i);
						con_l[(i+1) % 2].addNode(new_node1);
						con_l[i % 2].addNode(n_i);
					}
				}

				Node last_node = w.getNode(w.getNodesCount()-1);
				con_r[(w.getNodesCount()-1) % 2].addNode(last_node);
				con_l[(w.getNodesCount()-1) % 2].addNode(last_node);

				if (addedNodes.containsKey(last_node)) {
					double ang2 = last_node.getCoor().bearing(w.getNode(w.getNodesCount()-2).getCoor());
					double ang1 = ang2 > Math.PI ? ang2 - Math.PI : ang2 + Math.PI;

					List<Node> r_nodes = addedNodes.get(last_node).getRightNodesWithinAngle(ang1, ang2);
					for (Node n : r_nodes) {
						way_r.addNode(n);
					}
					if (r_nodes.size() > 0) {
						con_r[w.getNodesCount() % 2].addNode(r_nodes.get(0));
					}

					List<Node> l_nodes = addedNodes.get(last_node).getLeftNodesWithinAngle(ang1, ang2);
					for (Node n : l_nodes) {
						way_l.addNode(n);
					}
					if (l_nodes.size() > 0) {
						con_l[w.getNodesCount() % 2].addNode(l_nodes.get(0));
					}
				}

				more_ways.add(way_l);
				more_ways.add(con_l[0]);
				more_ways.add(con_l[1]);
				more_ways.add(way_r);
				more_ways.add(con_r[0]);
				more_ways.add(con_r[1]);
			}

			System.out.println("Add all nodes to dataset");
			for (Osm2XNodeList nodes : addedNodes.values()) {
				for (Node n : nodes.getNodelist()) {
					ds.addPrimitive(n);
				}
			}
			System.out.println("Add all ways to dataset");
			for (Way w : addedWays) {
				ds.addPrimitive(w);
			}

			System.out.println("Add more nodes to dataset");
			for (Node n : more_nodes) {
				ds.addPrimitive(n);
			}
			System.out.println("Add more ways to dataset");
			for (Way w : more_ways) {
				ds.addPrimitive(w);
			}

		} catch (Exception e) {
			System.out.println(e.getMessage());
		} finally
		{
			ds.endUpdate();
		}
		this.ds = ds;
	}

	public DataSet getModifiedDataset() {
		return (this.ds == null) ? this.original_ds : ds;
	}

	private TreeMap<Double, Way> getNeighbourNodes(Node node) {
		TreeMap<Double, Way> neighbours = new TreeMap<>();

		for (Way w : node.getParentWays()) {
			for (Node n : w.getNeighbours(node)) {
				neighbours.put(node.getCoor().bearing(n.getCoor()), w);
			}
		}
		return neighbours;
	}

	/**
	 * Calculate a new node based on the width of way 1 and way 2.
	 * @param origin the node connecting both ways
	 * @param way1 the first way
	 * @param way2 the second way
	 * @return coordinates for the calculated node
	 */
	private LatLon calculateNode(LatLon origin, Entry<Double, Way> way1, Entry<Double, Way> way2) {
		// calculate distance in meters
		double b1 = parseDouble(way1.getValue().get("width"));
		double b2 = parseDouble(way2.getValue().get("width"));
		double ang_diff = way2.getKey() - way1.getKey();

		double x = (b1 * Math.sin(way2.getKey()) + b2 * Math.sin(way1.getKey()))
				/ 3.0 / Math.sin(ang_diff);

		double y = (b1 * Math.cos(way2.getKey()) + b2 * Math.cos(way1.getKey()))
				/ 3.0 / Math.sin(ang_diff);

		x = origin.lon() + x*Osm2XConversions.METER_TO_LATLON/Math.cos(origin.lat()/180*Math.PI);
		y = origin.lat() + y*Osm2XConversions.METER_TO_LATLON;

		return new LatLon(y,x);
	}

	/**
	 *
	 * @param s the string to parse
	 * @return the number
	 */
	private double parseDouble(String s) {
		double d = 0;
		if (s == null) return d;
		try {
			d = Double.valueOf(s);
		} catch (NumberFormatException nfe) {
			// try to parse with regex
			Matcher matcher = str_to_double.matcher(s);
			if (matcher.find()) {
				try {
					d = Double.valueOf(matcher.group());
				} catch (NumberFormatException nfe1) {
					System.err.println(nfe1.getMessage());
				}
			}
		}
		return d;
	}
}
