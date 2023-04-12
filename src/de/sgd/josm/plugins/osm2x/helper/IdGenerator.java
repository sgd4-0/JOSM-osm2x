package de.sgd.josm.plugins.osm2x.helper;

import java.util.ArrayList;
import java.util.concurrent.ThreadLocalRandom;

import org.openstreetmap.josm.data.osm.DataSet;
import org.openstreetmap.josm.data.osm.Node;

/**
 * Generator for unique ids
 */
public class IdGenerator {
	private ArrayList<Long> ids;	// vector containing all ids

	/**
	 * Initialize new id generator
	 * @param ds DataSet to import ids from
	 */
	public IdGenerator(DataSet ds) {
		if (ds != null)
		{
			// initialize new vector and add all ids from dataset
			ids = new ArrayList<>();
			for (Node n : ds.getNodes()) {
				ids.add(n.getId());
			}
		}
	}

	/**
	 * Generate new unique id
	 * @return unique id
	 */
	public Long generateID() {
		// generate new random id in range from 1000000 to 1000000000
		Long id = ThreadLocalRandom.current().nextLong(1000000, 1000000000);

		while (ids.contains(id)) {
			// if id is already contained in dataset generate new
			id = ThreadLocalRandom.current().nextLong(1000000, 1000000000);
		}

		ids.add(id);	// add generated id to vector and return id
		return id;
	}
}
