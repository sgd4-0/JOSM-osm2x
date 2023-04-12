package de.sgd.josm.plugins.osm2x;

import static org.openstreetmap.josm.tools.I18n.tr;

import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import javax.swing.AbstractAction;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;

import org.openstreetmap.josm.data.osm.DataSet;
import org.openstreetmap.josm.data.osm.Filter;
import org.openstreetmap.josm.data.osm.FilterMatcher;
import org.openstreetmap.josm.data.osm.Node;
import org.openstreetmap.josm.data.osm.Relation;
import org.openstreetmap.josm.data.osm.Way;
import org.openstreetmap.josm.data.osm.search.SearchParseError;
import org.openstreetmap.josm.data.osm.search.SearchSetting;
import org.openstreetmap.josm.gui.MainApplication;
import org.openstreetmap.josm.gui.SideButton;
import org.openstreetmap.josm.gui.dialogs.ToggleDialog;
import org.openstreetmap.josm.gui.layer.Layer;
import org.openstreetmap.josm.gui.layer.LayerManager.LayerAddEvent;
import org.openstreetmap.josm.gui.layer.LayerManager.LayerChangeListener;
import org.openstreetmap.josm.gui.layer.LayerManager.LayerOrderChangeEvent;
import org.openstreetmap.josm.gui.layer.LayerManager.LayerRemoveEvent;
import org.openstreetmap.josm.gui.layer.OsmDataLayer;
import org.openstreetmap.josm.spi.preferences.Config;
import org.openstreetmap.josm.tools.ImageProvider;
import org.openstreetmap.josm.tools.Logging;

import de.sgd.josm.plugins.osm2x.helper.Osm2XConstants;
import de.sgd.josm.plugins.osm2x.modules.Osm2XFilter;
import de.sgd.josm.plugins.osm2x.modules.Osm2XMesher;
import de.sgd.josm.plugins.osm2x.modules.Osm2XMesherChecker;

public class Osm2XDialog extends ToggleDialog implements LayerChangeListener  {
	private static final long serialVersionUID = 6317484190555235261L;

	/**
	 * The button to divide the map into roads and barriers
	 */
	private SideButton splitButton;

	/**
	 * The reset button
	 */
	private SideButton nodecountButton;

	/**
	 * The create road network button
	 */
	private SideButton meshButton;

	/**
	 * The label to display the number of nodes
	 */
	private JLabel numNodesLabel;

	/**
	 * The label to display the number of ways
	 */
	private JLabel numWaysLabel;

	/**
	 * The label to display the number of relations
	 */
	private JLabel numRelLabel;

	/**
	 * Create Osm2X dialog
	 */
	public Osm2XDialog() {
		super(tr("Shared Guide Dog 4.0 Map Creator"), "osm2x_48", tr("Open the SGD4.0 Map Creator dialog"),
				null, 150);
		nodecountButton = new SideButton(new AbstractAction() {
			{
				putValue(NAME, tr("Count Nodes"));
				new ImageProvider("dialogs/select").getResource().attachImageIcon(this, true);
				putValue(SHORT_DESCRIPTION, tr("Count nodes."));
			}
			@Override
			public void actionPerformed(ActionEvent e)
			{
				recalculate();
			}
		});

		splitButton = new SideButton(new AbstractAction() {
			{
				putValue(NAME, tr("Split"));
				new ImageProvider("dialogs/split").getResource().attachImageIcon(this, true);
				putValue(SHORT_DESCRIPTION, tr("Split the current layer into ways and barriers."));
			}
			@Override
			public void actionPerformed(ActionEvent e) {
				splitLayer();
			}
		});

		meshButton = new SideButton(new AbstractAction() {
			{
				putValue(NAME, tr("Mesh"));
				new ImageProvider("dialogs/mesh").getResource().attachImageIcon(this, true);
				putValue(SHORT_DESCRIPTION, tr("Mesh ways."));
			}
			@Override
			public void actionPerformed(ActionEvent e) {

				Osm2XMesherChecker mesherChecker = new Osm2XMesherChecker();
				int retCode = mesherChecker.doChecks();

				if (retCode == Osm2XMesherChecker.FAILURE)
				{
					Logging.warn("Mesher checker returned failures");
					return;
				}

				Osm2XMesher mesher = new Osm2XMesher(MainApplication.getLayerManager().getEditDataSet());
				mesher.interpolate();
				mesher.createRoadNetwork();

				Layer meshed = new OsmDataLayer(mesher.getModifiedDataset(), "meshed", null);
				MainApplication.getLayerManager().addLayer(meshed);
			}
		});

		// Create new display panel
		JPanel valuePanel = new JPanel(new GridLayout(0,2));

		valuePanel.add(new JLabel(tr("Ways with missing tags")));
		numNodesLabel = new JLabel(getNodeCount());
		valuePanel.add(numNodesLabel);

		valuePanel.add(new JLabel(tr("Addresses found")));
		numWaysLabel = new JLabel(getAddrCount());
		valuePanel.add(numWaysLabel);

		valuePanel.add(new JLabel(tr("Missing ways to entrances")));
		numRelLabel = new JLabel(getMissingAddrLinks());
		valuePanel.add(numRelLabel);

		this.setPreferredSize(new Dimension(0, 92));

		createLayout(valuePanel, false, Arrays.asList(
				nodecountButton, splitButton, meshButton));

		MainApplication.getLayerManager().addLayerChangeListener(this);

		/*
		 * Weitere moegliche Datenquellen:
		 * geoportal hamburg: Feinkartierung Strasse Hamburg, Kreuzungsskizzen Hamburg
		 */
	}

	/**
	 * Count nodes without 'highway', 'surface' and 'width' attribute
	 * @return number of nodes as string
	 */
	protected String getNodeCount() {
		DataSet ds;
		if ((ds = MainApplication.getLayerManager().getEditDataSet()) != null) {
			int counter = 0;
			for (Way w : ds.getWays())
			{
				if (!w.isDeleted() && (!w.hasTag("highway") || !w.hasTag("surface")
						|| !w.hasTag("width")))
				{
					counter++;
				}
			}

			return Integer.toString(counter);
		}
		return tr("-1");
	}

	/**
	 * Count nodes with tag 'entrance' and 'addr:street'
	 * @return
	 */
	protected String getAddrCount() {
		DataSet ds;
		if ((ds = MainApplication.getLayerManager().getEditDataSet()) != null) {
			Collection<Node> nodes = ds.getNodes();

			int numNodes = 0;
			for (Node n : nodes)
			{
				if (n.hasTag("entrance") && n.hasTag("addr:street") && !n.isDeleted())
				{
					numNodes++;
				}
			}

			return Integer.toString(numNodes);
		}
		return tr("-1");
	}

	/**
	 *
	 * @return
	 */
	protected String getMissingAddrLinks() {
		DataSet ds;
		if ((ds = MainApplication.getLayerManager().getEditDataSet()) != null) {
			Collection<Node> nodes = ds.getNodes();

			int numNodes = 0;
			for (Node n : nodes)
			{
				if (n.hasTag("entrance") && !n.isDeleted()) {
					boolean hasWay = false;
					for (Way w : n.getParentWays()) {
						if (w.hasTag("highway")) {
							hasWay = true;
							break;
						}
					}
					numNodes = hasWay ? numNodes : numNodes+1;
				}
			}

			return Integer.toString(numNodes);
		}
		return tr("-1");
	}

	/**
	 * Recalculate all labels
	 */
	protected void recalculate() {
		numRelLabel.setText(getMissingAddrLinks());
		numWaysLabel.setText(getAddrCount());
		numNodesLabel.setText(getNodeCount());
	}

	/**
	 * Create barriers and highways layer
	 */
	protected void splitLayer() {
		// check if layer highways and barriers is present
		int answer = -1;
		List<Layer> layers = MainApplication.getLayerManager().getLayers();
		for (Layer l : layers)
		{
			if (l.getName().equalsIgnoreCase("barriers") || l.getName().equalsIgnoreCase("highways"))
			{
				if (answer < 0)
				{
					answer = JOptionPane.showConfirmDialog(MainApplication.getMainFrame(),
							"Layer with name 'barriers' or 'highways' detected. Do you want to continue and delete these layers?",
							"Confirm to delete layer", JOptionPane.YES_NO_OPTION);
				}
				if (answer == JOptionPane.YES_OPTION)
				{
					MainApplication.getLayerManager().removeLayer(l);
				}
				else
				{
					return;
				}
			}
		}

		// get current active dataset
		DataSet ds = MainApplication.getLayerManager().getEditDataSet();
		if (ds != null)
		{
			// create ds_new as a copy from ds
			DataSet ds_new = new DataSet(ds);
			ds_new.beginUpdate();
			try {
				// remove all relations
				for (Relation r : ds_new.getRelations()) {
					ds_new.removePrimitive(r.getPrimitiveId());
				}

				// copy address tags from building to entrance node
				if (Config.getPref().getBoolean(Osm2XConstants.PREF_COPY_ADDRESS, Osm2XConstants.DEFAULT_COPY_ADDRESS))
				{
					for (Node n : ds_new.getNodes())
					{
						if (n.hasTag("entrance") && !n.hasTag("addr:street"))
						{
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
						}
					}
				}

				// create filter from preferences
				Filter f = new Filter(SearchSetting.readFromString("C " + Config.getPref().get(Osm2XConstants.PREF_HIGHWAY_FILTER, Osm2XConstants.DEFAULT_HIGHWAY_FILTER)));
				f.inverted = true;						// set inverted to true to get matching primitives
				FilterMatcher matcher = FilterMatcher.of(f);

				// remove all ways that do not match the specified tags
				Osm2XFilter.executeMatching(ds_new, matcher, Way.class::isInstance);
				// remove all nodes that do not match the filter
				Osm2XFilter.executeMatching(ds_new, matcher, Node.class::isInstance);

				matcher.reset();
			} catch (SearchParseError e) {
				// display warning
				JOptionPane.showMessageDialog(MainApplication.getMainFrame(), "SearchParseError: Check highway filtering and try again.", "SearchParseError", JOptionPane.WARNING_MESSAGE);
				e.printStackTrace();
				return;
			} finally {
				ds_new.endUpdate();
			}

			Layer highways = new OsmDataLayer(ds_new, "highways", null);
			MainApplication.getLayerManager().addLayer(highways);

			// create barrier layer
			// copy current dataset to ds_new
			ds_new = new DataSet(ds);
			ds_new.beginUpdate();
			try {
				// remove all relations
				for (Relation r : ds_new.getRelations()) {
					ds_new.removePrimitive(r.getPrimitiveId());
				}

				// create filter
				Filter f = new Filter(SearchSetting.readFromString("C " + Config.getPref().get(Osm2XConstants.PREF_BARRIER_FILTER, Osm2XConstants.DEFAULT_BARRIER_FILTER)));
				f.inverted = true;
				FilterMatcher matcher = FilterMatcher.of(f);

				// remove all ways that do not match the specified tags
				Osm2XFilter.executeMatching(ds_new, matcher, Way.class::isInstance);
				// remove all nodes that do not match the filter
				Osm2XFilter.executeMatching(ds_new, matcher, Node.class::isInstance);

				matcher.reset();
			} catch (SearchParseError e) {
				JOptionPane.showMessageDialog(MainApplication.getMainFrame(), "SearchParseError: Check barrier filtering and try again.", "SearchParseError", JOptionPane.WARNING_MESSAGE);
				e.printStackTrace();
				return;
			} finally {
				ds_new.endUpdate();
			}
			Layer barrier = new OsmDataLayer(ds_new, "barriers", null);
			MainApplication.getLayerManager().addLayer(barrier);
		}
	}

	protected void shpToOsm() {
		// translate selected layer to osm
		DataSet ds = MainApplication.getLayerManager().getEditDataSet();

		//		if (ds != null && attrParser != null) {
		//			ds.getReadLock().lock();
		//			try {
		//				DataSet newDs;
		//				// if something is selected parse only selected
		//				Collection<Way> ways = ds.getSelectedWays();
		//				Collection<Node> nodes = ds.getSelectedNodes();
		//
		//				if (!ways.isEmpty()) {
		//					newDs = attrParser.parseWays(ways);
		//				} else if (!nodes.isEmpty()) {
		//					newDs = attrParser.parseNodes(nodes);
		//				} else if (!(ways = ds.getWays()).isEmpty()) {
		//					newDs = attrParser.parseWays(ways);
		//				} else {
		//					newDs = attrParser.parseNodes(ds.getNodes());
		//				}
		//
		//				String name = MainApplication.getLayerManager().getEditLayer().getName();
		//				name = name.substring(0, name.lastIndexOf("."));
		//
		//				boolean isLayerPresent = false;
		//				Layer newLayer = new OsmDataLayer(newDs, name, null);
		//				for (Layer l : MainApplication.getLayerManager().getLayers()) {
		//					if (l.getName().equalsIgnoreCase(name)) {
		//						// merge layer
		//						isLayerPresent = true;
		//						l.mergeFrom(newLayer);
		//						break;
		//					}
		//				}
		//
		//				if (!isLayerPresent) {
		//					MainApplication.getLayerManager().addLayer(newLayer);
		//				}
		//			} finally {
		//				ds.getReadLock().unlock();
		//			}
		//		}
	}

	@Override
	public void layerAdded(LayerAddEvent e) {
		recalculate();
	}

	@Override
	public void layerOrderChanged(LayerOrderChangeEvent e) {
		recalculate();
	}

	@Override
	public void layerRemoving(LayerRemoveEvent e) {
		if (!MainApplication.getLayerManager().getLayers().isEmpty()) {
			recalculate();
		}
	}
}
