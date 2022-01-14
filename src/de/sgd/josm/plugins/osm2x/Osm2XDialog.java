package de.sgd.josm.plugins.osm2x;

import static org.openstreetmap.josm.tools.I18n.tr;

import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.io.IOException;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import javax.swing.AbstractAction;
import javax.swing.JLabel;
import javax.swing.JPanel;

import org.openstreetmap.josm.data.osm.DataSet;
import org.openstreetmap.josm.data.osm.Filter;
import org.openstreetmap.josm.data.osm.Node;
import org.openstreetmap.josm.data.osm.Way;
import org.openstreetmap.josm.data.osm.search.SearchSetting;
import org.openstreetmap.josm.gui.MainApplication;
import org.openstreetmap.josm.gui.SideButton;
import org.openstreetmap.josm.gui.dialogs.FilterTableModel;
import org.openstreetmap.josm.gui.dialogs.ToggleDialog;
import org.openstreetmap.josm.gui.layer.Layer;
import org.openstreetmap.josm.gui.layer.LayerManager.LayerAddEvent;
import org.openstreetmap.josm.gui.layer.LayerManager.LayerChangeListener;
import org.openstreetmap.josm.gui.layer.LayerManager.LayerOrderChangeEvent;
import org.openstreetmap.josm.gui.layer.LayerManager.LayerRemoveEvent;
import org.openstreetmap.josm.gui.layer.OsmDataLayer;
import org.openstreetmap.josm.tools.ImageProvider;

public class Osm2XDialog extends ToggleDialog implements LayerChangeListener  {
	private static final long serialVersionUID = 6317484190555235261L;

	/**
	 * The okay button.
	 */
	private SideButton okButton;

	/**
	 * The reset button
	 */
	private SideButton resetButton;

	/**
	 *
	 */
	private SideButton meshButton;

	/**
	 *
	 */
	private SideButton toOsmButton;

	/**
	 *
	 */
	private JLabel numNodesLabel;

	/**
	 *
	 */
	private JLabel numWaysLabel;

	/**
	 *
	 */
	private JLabel numRelLabel;

	private Osm2XAttributeParser attrParser = null;

	/**
	 *
	 */
	public Osm2XDialog() {
		super(tr("Osm2x"), "osm2x_dlg", tr("Open Osm2x dialog"),
				null, 150);

		try {
			attrParser = new Osm2XAttributeParser();
		} catch (IOException ioe) {
			ioe.printStackTrace();
		}

		resetButton = new SideButton(new AbstractAction() {
			{
				putValue(NAME, tr("Count Nodes"));
				new ImageProvider("dialogs", "select").getResource().attachImageIcon(this, true);
				putValue(SHORT_DESCRIPTION, tr("Do something."));
				//putValue("help", HelpUtil.ht("/Dialog/Osm2X#OK"));
			}
			@Override
			public void actionPerformed(ActionEvent e)
			{
				recalculate();
			}
		});

		okButton = new SideButton(new AbstractAction() {
			{
				putValue(NAME, tr("Split"));
				new ImageProvider("dialogs", "select").getResource().attachImageIcon(this, true);
				putValue(SHORT_DESCRIPTION, tr("Reset current measurement results and delete measurement path."));
				//putValue("help", HelpUtil.ht("/Dialog/Measurement#Reset"));
			}
			@Override
			public void actionPerformed(ActionEvent e) {
				splitLayer();
			}
		});

		meshButton = new SideButton(new AbstractAction() {
			{
				putValue(NAME, tr("Mesh"));
				new ImageProvider("dialogs", "select").getResource().attachImageIcon(this, true);
				putValue(SHORT_DESCRIPTION, tr("Mesh ways."));
				//putValue("help", HelpUtil.ht("/Dialog/Measurement#Reset"));
			}
			@Override
			public void actionPerformed(ActionEvent e) {
				meshNodes();
			}
		});

		toOsmButton = new SideButton(new AbstractAction() {
			{
				putValue(NAME, tr("Shp to OSM"));
				new ImageProvider("dialogs", "select").getResource().attachImageIcon(this, true);
				putValue(SHORT_DESCRIPTION, tr("Convert shape layer to osm layer."));
				//putValue("help", HelpUtil.ht("/Dialog/Measurement#Reset"));
			}
			@Override
			public void actionPerformed(ActionEvent e) {
				shpToOsm();
			}
		});

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

		createLayout(valuePanel, false, Arrays.asList(new SideButton[] {
				resetButton, okButton, meshButton, toOsmButton
		}));

		MainApplication.getLayerManager().addLayerChangeListener(this);

		/*
		 * Weitere moegliche Datenquellen:
		 * geoportal hamburg: Feinkartierung Strasse Hamburg, Kreuzungsskizzen Hamburg
		 */

	}

	protected String getNodeCount() {
		DataSet ds;
		if ((ds = MainApplication.getLayerManager().getEditDataSet()) != null) {
			int counter = 0;
			for (Way w : ds.getWays())
			{
				if (!w.isDeleted())
				{
					if (!w.hasTag("highway") || !w.hasTag("surface")
							|| !w.hasTag("width")) {
						counter++;
					}
				}
			}

			return Integer.toString(counter);
		}
		return tr("-1");
	}

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
					List<Way> l = n.getParentWays();
					boolean hasWay = false;
					for (Way w : l) {
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

	protected void recalculate() {
		numRelLabel.setText(getMissingAddrLinks());
		numWaysLabel.setText(getAddrCount());
		numNodesLabel.setText(getNodeCount());
	}

	protected void splitLayer() {
		// TODO check if layer highways and barriers is present
		// get all nodes and ways
		// add ways (including nodes) with tag highway to layer

		DataSet ds = MainApplication.getLayerManager().getEditDataSet();
		Osm2XFilter filter = new Osm2XFilter();

		// add all highway nodes to layer
		Layer highways = new OsmDataLayer(filter.getHighways(ds), "highways", null);
		MainApplication.getLayerManager().addLayer(highways);

		// add all barrier nodes to layer
		Layer barriers = new OsmDataLayer(filter.getBarriers(ds), "barriers", null);
		MainApplication.getLayerManager().addLayer(barriers);
	}

	protected void meshNodes() {
		// TODO check missing tags and missing ways to entrances
		System.out.println("Interpolate nodes");
		Osm2XMesher mesher = new Osm2XMesher(MainApplication.getLayerManager().getEditDataSet());

		if (!mesher.isDatasetComplete()) {
			FilterTableModel ftm = MainApplication.getMap().filterDialog.getFilterModel();
			ftm.addFilter(new Filter(SearchSetting.readFromString("C width:")));
			ftm.executeFilters();
			return;
		}

		mesher.interpolate();
		mesher.createRoadNetwork();

		Layer meshed = new OsmDataLayer(mesher.getModifiedDataset(), "meshed", null);
		MainApplication.getLayerManager().addLayer(meshed);
	}

	protected void shpToOsm() {
		// translate selected layer to osm
		DataSet ds = MainApplication.getLayerManager().getEditDataSet();

		if (ds != null && attrParser != null) {
			try {
				ds.getReadLock().lock();

				DataSet ds_new;
				// if something is selected parse only selected
				Collection<Way> ways = ds.getSelectedWays();
				Collection<Node> nodes = ds.getSelectedNodes();

				if (ways.size() > 0) {
					System.out.println("Parse selected ways");
					ds_new = attrParser.parseWays(ways);
				} else if (nodes.size() > 0) {
					System.out.println("Parse selected nodes");
					ds_new = attrParser.parseNodes(nodes);
				} else if ((ways = ds.getWays()).size() > 0) {
					System.out.println("Parse ways");
					ds_new = attrParser.parseWays(ways);
				} else {
					System.out.println("Parse nodes");
					ds_new = attrParser.parseNodes(ds.getNodes());
				}

				String name = MainApplication.getLayerManager().getEditLayer().getName();
				name = name.substring(0, name.lastIndexOf("."));

				boolean isLayerPresent = false;
				Layer newLayer = new OsmDataLayer(ds_new, name, null);
				for (Layer l : MainApplication.getLayerManager().getLayers()) {
					if (l.getName().equalsIgnoreCase(name)) {
						// merge layer
						isLayerPresent = true;
						l.mergeFrom(newLayer);
						break;
					}
				}

				if (!isLayerPresent) {
					MainApplication.getLayerManager().addLayer(newLayer);
				}
			} finally {
				ds.getReadLock().unlock();
			}
		}
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
		if (MainApplication.getLayerManager().getLayers().size() > 0) {
			recalculate();
		}
	}
}
