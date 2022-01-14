package de.sgd.josm.plugins.osm2x;

import java.util.List;

import org.openstreetmap.josm.data.osm.DataSet;
import org.openstreetmap.josm.data.osm.Node;
import org.openstreetmap.josm.data.osm.Relation;
import org.openstreetmap.josm.data.osm.Way;

public class Osm2XFilter {
	public Osm2XFilter() {

	}

	/**
	 *
	 * @param ds_orig
	 * @return
	 */
	public DataSet getHighways(DataSet ds_orig) {
		if (ds_orig != null) {
			// create new dataset as copy from original dataset
			DataSet ds_new = new DataSet(ds_orig);
			ds_new.beginUpdate();

			try {
				// copy address tags from building to entrance node
				for (Node n : ds_new.getNodes()) {
					if (n.hasTag("entrance") && !n.hasTag("addr:street")) {
						try {
							List<Way> refs = n.getParentWays();
							for (Way p : refs) {
								if (p.hasTag("addr:street") && p.hasTag("building")) {
									n.put("addr:street", p.get("addr:street"));
									n.put("addr:housenumber", p.get("addr:housenumber"));
									n.put("addr:city", p.get("addr:city"));
									n.put("addr:postcode", p.get("addr:postcode"));
									break;
								}
							}
						} catch (Exception e) {
							System.out.println(e.getMessage());
						}
					}
				}

				// remove all relations
				for (Relation r : ds_new.getRelations()) {
					ds_new.removePrimitive(r.getPrimitiveId());
				}

				// remove all ways that do not match the specified tags
				for (Way w : ds_new.getWays()) {
					if (w.isIncomplete() || !w.hasTag("highway") || w.isOutsideDownloadArea()) {
						ds_new.removePrimitive(w.getPrimitiveId());
					}
				}

				// remove all ways without connection to other ways
				for (Way w : ds_new.getWays()) {
					boolean con = false;
					for (Node wn : w.getNodes()) {
						if (wn.isReferredByWays(2)) {
							con = true;
							break;
						}
					}
					if (!con) {
						ds_new.removePrimitive(w.getPrimitiveId());
					}
				}

				// remove all nodes that are not included in any way
				removeLonelyNodes(ds_new, "entrance");
			} finally {
				ds_new.endUpdate();
			}

			return ds_new;
		}
		return new DataSet();
	}

	/**
	 *
	 * @param ds_orig
	 * @return
	 */
	public DataSet getBarriers(DataSet ds_orig) {
		if (ds_orig != null) {
			// create new dataset as copy from original dataset
			DataSet ds_new = new DataSet(ds_orig);
			ds_new.beginUpdate();

			try {
				// remove ways that are incomplete or do not have any of the keys
				for (Way n : ds_new.getWays()) {
					if (n.hasKey("building", "barrier"))
					{
						continue;
					}
					ds_new.removePrimitive(n.getPrimitiveId());
				}

				// remove all relations
				for (Relation r : ds_new.getRelations()) {
					ds_new.removePrimitive(r.getPrimitiveId());
				}

				// remove nodes that are not included in any way and do not match the keys
				removeLonelyNodes(ds_new, "barrier", "natural");
			} finally {
				ds_new.endUpdate();
			}

			return ds_new;
		}
		return new DataSet();
	}

	/**
	 * Removes nodes from the dataset that are outside the downloaded area or
	 * not referred to by any way. If a node contains any of the keys, it will not be removed.
	 * @param ds
	 * @param key
	 * @return
	 */
	private DataSet removeLonelyNodes(DataSet ds, String... key) {
		for (Node n : ds.getNodes()) {
			if ((!n.isReferredByWays(1) && !n.hasKey(key)) || n.isOutsideDownloadArea()) {
				ds.removePrimitive(n.getPrimitiveId());
			}
		}
		return ds;
	}
}
