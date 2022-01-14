package de.sgd.josm.plugins.osm2x.helper;

import java.util.Vector;
import java.util.concurrent.ThreadLocalRandom;

import org.openstreetmap.josm.data.osm.DataSet;
import org.openstreetmap.josm.data.osm.Node;

public class IdGenerator {
	private Vector<Long> ids;

	public IdGenerator(DataSet ds) {
		if (ds != null)
		{
			ids = new Vector<>();
			for (Node n : ds.getNodes()) {
				ids.add(n.getId());
			}
		}
	}

	public Long generateID() {
		Long id = ThreadLocalRandom.current().nextLong(1000000, 1000000000);

		while (ids.contains(id)) {
			id = ThreadLocalRandom.current().nextLong(1000000, 1000000000);
		}

		ids.add(id);
		return id;
	}


}
