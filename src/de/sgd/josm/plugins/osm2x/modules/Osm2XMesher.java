package de.sgd.josm.plugins.osm2x.modules;

import java.io.File;
import java.io.IOException;
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
import org.openstreetmap.josm.data.osm.Filter;
import org.openstreetmap.josm.data.osm.FilterMatcher;
import org.openstreetmap.josm.data.osm.Node;
import org.openstreetmap.josm.data.osm.Way;
import org.openstreetmap.josm.data.osm.search.SearchParseError;
import org.openstreetmap.josm.data.osm.search.SearchSetting;
import org.openstreetmap.josm.gui.MainApplication;
import org.openstreetmap.josm.spi.preferences.Config;
import org.openstreetmap.josm.tools.Logging;

import de.sgd.josm.plugins.osm2x.helper.IdGenerator;
import de.sgd.josm.plugins.osm2x.helper.Osm2XConstants;
import de.sgd.josm.plugins.osm2x.helper.Osm2XConversions;
import de.sgd.josm.plugins.osm2x.helper.Osm2XNodeList;
import de.sgd.josm.plugins.osm2x.io.ExportPrefIO;
import de.sgd.josm.plugins.osm2x.preferences.ExportRuleEntry;

public class Osm2XMesher {

	private Pattern strToDouble;

	private IdGenerator idGen;

	private DataSet originalDS;
	private DataSet interpolatedDS;
	private DataSet ds = null;

	/**
	 * Create a new Osm2XMesher object
	 * @param ds the dataset to edit
	 */
	public Osm2XMesher(DataSet ds) {
		this.originalDS = ds;
		this.strToDouble = Pattern.compile("(\\d+\\.?\\d*)");
		this.idGen = new IdGenerator(originalDS);

		// Check if all paths have the width attribute
		boolean width_present = true;
		for (Way w : ds.getWays()) {
			if (!w.hasTag("width")) {
				// highlight paths without width attribute
				w.setHighlighted(true);
				width_present = false;
			}
		}

		if (!width_present) {
			this.ds = ds;
		}
	}

	/**
	 * Prepares the dataset for interpolation and meshing
	 */
	public void correctDataset()
	{
		// 1. read xml with export rules
		// read settings from file
		ExportPrefIO prefIO = new ExportPrefIO(new File(Config.getPref().get(Osm2XConstants.PREF_EXPORT_PREF_FILE)));
		List<ExportRuleEntry> exportRules = new ArrayList<>();

		try {
			exportRules = prefIO.readFromFile();

			// create filter from preferences
			Filter f = new Filter(SearchSetting.readFromString("C " + Config.getPref().get(Osm2XConstants.PREF_HIGHWAY_FILTER, Osm2XConstants.DEFAULT_HIGHWAY_FILTER)));
			f.inverted = true;						// set inverted to true to get matching primitives
			FilterMatcher matcher = FilterMatcher.of(f);

			DataSet ds = MainApplication.getLayerManager().getActiveDataSet();
			// remove all ways that do not match the specified tags
			Osm2XFilter.executeMatching(ds, matcher, Way.class::isInstance);

		} catch (IOException ioe)
		{
			Logging.error(ioe);
		} catch (SearchParseError spe)
		{
			Logging.error(spe);
		}

		// 2. apply export rules to dataset

	}

	/**
	 * To allow changing the path in short intervals, ways with a length greater than a threshold are linearly interpolated.
	 */
	public void interpolate() {
		DataSet newDS = new DataSet(this.originalDS);

		Logging.info("Start interpolation");

		newDS.beginUpdate();
		try
		{
			for (Way w : newDS.getWays()) {
				for (int i = 1; i < w.getRealNodesCount(); i++) {
					Node lastNode = w.getNode(i-1);
					Node currNode = w.getNode(i);

					double dist = lastNode.getCoor().greatCircleDistance(currNode.getCoor());

					double interpDist = Config.getPref().getDouble(Osm2XConstants.PREF_INTERP_DIST, Osm2XConstants.DEFAULT_INTERP_DIST);
					if (dist > interpDist)
					{
						int addNodes = (int) Math.ceil(dist / interpDist);

						int k = 0;
						for (k = 1; k < addNodes; k++) {
							// Calculate new latlon and node
							LatLon ll = lastNode.getCoor().interpolate(currNode.getCoor(), (double)k/addNodes);
							Node nn = new Node(ll);
							nn.setOsmId(this.idGen.generateID(), 1);
							// add new node to dataset and way
							newDS.addPrimitive(nn);
							w.addNode(i+k-1, nn);
						}
						i += k-1;
					}
				}
			}
		} finally
		{
			newDS.endUpdate();
		}
		this.interpolatedDS = newDS;
	}

	/**
	 * Create parallel ways and connect them to original ways
	 */
	public void createRoadNetwork() {
		// get all ways -> ignore areas, but with warning
		// ignore ways without width attribute
		// ignore ways with attribute mesh=false
		DataSet ds = (this.interpolatedDS == null) ? new DataSet(originalDS) : new DataSet(interpolatedDS);

		System.out.println("Start meshing");

		ds.beginUpdate();
		try
		{
			HashMap<Node, Osm2XNodeList> addedNodes = new HashMap<>();
			List<Way> addedWays = new ArrayList<>();

			for (Node n : ds.getNodes()) {
				Osm2XNodeList nodes = new Osm2XNodeList();
				// check that nodes have an id
				if (n.getId() == 0)
				{
					n.setOsmId(this.idGen.generateID(), 1);
				}

				// set original node as parent id -> is copying all keys correct?
				Map<String, String> tags = n.getKeys();
				tags.put("pid", Long.toString(n.getId()));

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
						new_node.setKeys(tags);
						nodes.add(new_node, n.getCoor().bearing(new_node.getCoor()));
						last_entry = entry;
					}

					Node new_node = new Node(calculateNode(n.getCoor(), last_entry, angles.firstEntry()));
					new_node.setOsmId(this.idGen.generateID(), 1);
					new_node.setKeys(tags);
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
					if (width < 1.5) continue;
				} else {
					continue;
				}
				// TODO filtern??
				Way way_l = new Way(0);
				way_l.setKeys(w.getKeys());
				Map<String, String> keys_l = way_l.getKeys();
				keys_l.put("left", "true");

				Way con_l1 = new Way(0);
				con_l1.setKeys(keys_l);
				con_l1.addNode(w.getNode(0));

				Way con_l2 = new Way(0);
				con_l2.setKeys(keys_l);

				// add angle tag to left way
				keys_l.put("angle", "0.0");
				way_l.setKeys(keys_l);

				// ways for rights side
				Way way_r = new Way(0);

				Way con_r1 = new Way(0);
				con_r1.setKeys(w.getKeys());
				con_r1.addNode(w.getNode(0));

				Way con_r2 = new Way(0);
				con_r2.setKeys(w.getKeys());

				// add angle tag to right ways
				Map<String, String> keys_r = way_l.getKeys();
				keys_r.put("angle", "0.0");
				way_r.setKeys(keys_r);

				// add angle to middle ways
				Map<String, String> keys = w.getKeys();
				keys.put("angle", "0.0");
				w.setKeys(keys);

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
						Map<String, String> tags_ = n_i.getKeys();
						tags_.put("pid", Long.toString(n_i.getId()));
						new_node.setKeys(tags_);
						more_nodes.add(new_node);
						way_r.addNode(new_node);

						// Way on the left side
						Node new_node1 = new Node(Osm2XConversions.calculatePositionFrom(n_i.getCoor(), ang + Math.PI, width/3));
						new_node1.setOsmId(this.idGen.generateID(), 1);
						new_node1.setKeys(tags_);
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

	/**
	 * Returns the dataset we are currently working on
	 * @return
	 */
	public DataSet getModifiedDataset() {
		return (this.ds == null) ? this.originalDS : ds;
	}

	/**
	 * Get all neighbour nodes to node with an angle
	 * @param node
	 * @return
	 */
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
		double deltaAng = way2.getKey() - way1.getKey();

		double x = (b1 * Math.sin(way2.getKey()) + b2 * Math.sin(way1.getKey()))
				/ 3.0 / Math.sin(deltaAng);

		double y = (b1 * Math.cos(way2.getKey()) + b2 * Math.cos(way1.getKey()))
				/ 3.0 / Math.sin(deltaAng);

		x = origin.lon() + x*Osm2XConversions.METER_TO_LATLON/Math.cos(origin.lat()/180*Math.PI);
		y = origin.lat() + y*Osm2XConversions.METER_TO_LATLON;

		return new LatLon(y,x);
	}

	/**
	 * Convert string to double without throwing an exception
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
			Matcher matcher = strToDouble.matcher(s);
			if (matcher.find()) {
				try {
					d = Double.valueOf(matcher.group());
				} catch (NumberFormatException nfe1) {
					Logging.error(nfe1);
				}
			}
		}
		return d;
	}
}
