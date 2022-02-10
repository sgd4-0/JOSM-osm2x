package de.sgd.josm.plugins.osm2x.helper;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Map.Entry;
import java.util.TreeMap;

import org.openstreetmap.josm.data.osm.Node;

/**
 * Wrapper for josm node
 * @author pasca
 *
 */
public class Osm2XNodeList {

	TreeMap<Double, Node> nodes;

	public Osm2XNodeList() {
		nodes = new TreeMap<>();
	}

	public void add(Node node, double angle) {
		nodes.put(angle, node);
	}

	public List<Node> getLeftNodesWithinAngle(double front_ang, double back_ang) {
		List<Node> nodelist = new ArrayList<>();
		double key = back_ang;
		if (front_ang < back_ang) {
			while (key >= back_ang || key <= front_ang) {
				Entry<Double, Node> e = nodes.higherEntry(key);
				if (e == null) {
					e = nodes.ceilingEntry(0.0);
				}

				key = e.getKey();
				if (key >= back_ang || key <= front_ang)	nodelist.add(e.getValue());
			}
		} else {
			while (key >= back_ang && key <= front_ang) {
				Entry<Double, Node> e = nodes.higherEntry(key);
				if (e == null) {
					break;
				}

				key = e.getKey();
				if (key >= back_ang && key <= front_ang)	nodelist.add(e.getValue());
			}
		}

		return nodelist;
	}

	public List<Node> getRightNodesWithinAngle(double min, double max) {
		List<Node> nodelist = new ArrayList<>();

		double key = max;
		if (min < max) {
			while (key > min) {
				Entry<Double, Node> e = nodes.lowerEntry(key);
				if (e == null) break;
				if ((key = e.getKey()) > min)	nodelist.add(e.getValue());
			}
		} else {	// max < min
			while (key <= max || key >= min) {
				Entry<Double, Node> e = nodes.lowerEntry(key);
				if (e == null) {
					e = nodes.lowerEntry(2*Math.PI);
				}

				key = e.getKey();
				if (key < max || key > min)	nodelist.add(e.getValue());
			}
		}

		return nodelist;
	}

	public Collection<Node> getNodelist() {
		return this.nodes.values();
	}

}
