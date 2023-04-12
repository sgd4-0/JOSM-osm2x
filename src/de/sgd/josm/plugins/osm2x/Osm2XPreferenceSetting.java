package de.sgd.josm.plugins.osm2x;

import static org.openstreetmap.josm.tools.I18n.tr;

import java.awt.Dimension;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.GridLayout;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSeparator;
import javax.swing.JSpinner;
import javax.swing.JTabbedPane;
import javax.swing.JTextField;
import javax.swing.SpinnerNumberModel;
import javax.swing.SwingConstants;

import org.openstreetmap.josm.gui.preferences.DefaultTabPreferenceSetting;
import org.openstreetmap.josm.gui.preferences.PreferenceTabbedPane;
import org.openstreetmap.josm.spi.preferences.Config;
import org.openstreetmap.josm.tools.GBC;
import org.openstreetmap.josm.tools.ImageProvider;
import org.openstreetmap.josm.tools.Logging;

import de.sgd.josm.plugins.osm2x.helper.Osm2XConstants;
import de.sgd.josm.plugins.osm2x.io.ExportPrefIO;
import de.sgd.josm.plugins.osm2x.preferences.ExportRuleEntry;
import de.sgd.josm.plugins.osm2x.preferences.ExportRuleTable;

/**
 * Preference dialog
 *
 */
public class Osm2XPreferenceSetting extends DefaultTabPreferenceSetting {

	/**
	 * Main panel with tabs for general and export settings
	 */
	public final JTabbedPane tabPane = new JTabbedPane();

	private ExportPrefIO prefIO;

	private ExportRuleTable exportRuleTable;

	private final JSpinner spinner = new JSpinner(new SpinnerNumberModel(5.0, 2.0, null, 0.1));
	private final JTextField tagsBarrier = new JTextField();
	private final JCheckBox checkCopyAdr = new JCheckBox("Copy address to node");
	private final JTextField tagsHighway = new JTextField();

	public Osm2XPreferenceSetting() {
		super(Osm2XConstants.ICON_PREF_48,
				tr("Shared Guide Dog 4.0 Map Creator Preferences"),
				tr("Convert and export OSM data to be used on the Shared Guide Dog 4.0<br><br>"
						+ "This is an experimental version. Be careful when using it."));

		prefIO = new ExportPrefIO(new File(Config.getPref().get(Osm2XConstants.PREF_EXPORT_PREF_FILE)));
	}

	/**
	 * Create settings tab with general settings
	 * @return JPanel
	 */
	protected JPanel createSettings() {
		JPanel general = new JPanel(new GridBagLayout());
		general.setBorder(BorderFactory.createEmptyBorder(0, 0, 0, 0));
		general.setName(tr("SGD4.0 Map Creator"));

		// options for interpolation
		JLabel jLabelSpinner = new JLabel(tr("Interpolation: Maximum way length:"));
		spinner.getModel().setValue(Config.getPref().getDouble(Osm2XConstants.PREF_INTERP_DIST, Osm2XConstants.DEFAULT_INTERP_DIST));
		general.add(jLabelSpinner, GBC.std().insets(0, 5, 10, 0));
		general.add(spinner, GBC.eol().fill(GridBagConstraints.HORIZONTAL).insets(5, 5, 400, 5));

		// tags to use for barriers dataset (filter like syntax)
		general.add(new JSeparator(SwingConstants.HORIZONTAL), GBC.eol().fill(GridBagConstraints.HORIZONTAL));

		tagsBarrier.setText(Config.getPref().get(Osm2XConstants.PREF_BARRIER_FILTER, Osm2XConstants.DEFAULT_BARRIER_FILTER));
		tagsBarrier.setToolTipText("Search expression for creating the barrier dataset. Please refer to filter dialog for examples.");
		general.add(new JLabel(tr("Tags to include in barrier dataset:")), GBC.std().insets(0, 5, 10, 0));
		general.add(tagsBarrier, GBC.eol().fill(GridBagConstraints.HORIZONTAL).insets(5, 5, 200, 5));

		checkCopyAdr.setSelected(true);
		checkCopyAdr.setToolTipText("Choose whether to copy the addresses of buildings to the entrance nodes");
		general.add(checkCopyAdr, GBC.eol().insets(0, 0, 0, 0));

		// tags to use for highways dataset
		general.add(new JSeparator(SwingConstants.HORIZONTAL), GBC.eol().fill(GridBagConstraints.HORIZONTAL));
		JLabel jLabelHighwayTags = new JLabel(tr("Tags to include in highway dataset:"));
		tagsHighway.setText(Config.getPref().get(Osm2XConstants.PREF_HIGHWAY_FILTER, Osm2XConstants.DEFAULT_HIGHWAY_FILTER));
		tagsHighway.setToolTipText("Search expression for creating the barrier dataset. Please refer to filter dialog for examples.");
		general.add(jLabelHighwayTags, GBC.std().insets(0, 5, 10, 0));
		general.add(tagsHighway, GBC.eop().fill(GridBagConstraints.HORIZONTAL).insets(5, 5, 200, 5));

		// end of dialog, scroll bar
		general.add(Box.createVerticalGlue(), GBC.eol().fill(GridBagConstraints.VERTICAL));

		return general;
	}

	/**
	 * Creates a tab with all export settings
	 * @param gui preferences tab pane
	 * @return JPanel
	 */
	protected JPanel createExportSettings(PreferenceTabbedPane gui)
	{
		JPanel general = new JPanel();
		general.setName("Export");

		// read settings from file
		List<ExportRuleEntry> displayData = new ArrayList<>();

		try {
			displayData = prefIO.readFromFile();
		} catch (IOException ioe) {
			Logging.error(ioe);
			displayData.add(new ExportRuleEntry("<empty>", "<empty"));	// add empty entry
		}

		// create table and add to scrollpane
		exportRuleTable = new ExportRuleTable(displayData);
		JScrollPane scroll = new JScrollPane(exportRuleTable);
		general.add(scroll, GBC.eol().fill(GridBagConstraints.BOTH));
		scroll.setPreferredSize(new Dimension(400, 300));

		JPanel buttonPanel = new JPanel(new GridLayout(1, 6));	// up, down, add, remove, save, load
		// add button to shift row up
		JButton up = new JButton(tr("Up"), ImageProvider.get("dialogs/up", ImageProvider.ImageSizes.SMALLICON));
		buttonPanel.add(up, GBC.eol().fill(GridBagConstraints.HORIZONTAL));
		exportRuleTable.getSelectionModel().addListSelectionListener(event -> up.setEnabled(exportRuleTable.getSelectedRowCount() == 1 && exportRuleTable.getSelectedRow() > 0));
		up.addActionListener(e ->
		{
			int nextRow = exportRuleTable.getSelectedRow()-1;
			exportRuleTable.switchRows(exportRuleTable.getSelectedRow(), nextRow);
			exportRuleTable.changeSelection(nextRow, 0, false, false);
		});

		// add button to shift row down
		JButton down = new JButton(tr("Down"), ImageProvider.get("dialogs/down", ImageProvider.ImageSizes.SMALLICON));
		buttonPanel.add(down, GBC.eol().fill(GridBagConstraints.HORIZONTAL));
		exportRuleTable.getSelectionModel().addListSelectionListener(event -> down.setEnabled(exportRuleTable.getSelectedRowCount() == 1
				&& exportRuleTable.getSelectedRow() < (exportRuleTable.getRowCount()-1)));
		down.addActionListener(e ->
		{
			int nextRow = exportRuleTable.getSelectedRow()+1;
			exportRuleTable.switchRows(exportRuleTable.getSelectedRow(), nextRow);
			exportRuleTable.changeSelection(nextRow, 0, false, false);
		});

		// add button to add row
		JButton add = new JButton(tr("Add"), ImageProvider.get("dialogs/add", ImageProvider.ImageSizes.SMALLICON));
		buttonPanel.add(add, GBC.eol().fill(GridBagConstraints.HORIZONTAL));
		add.setToolTipText("Add new entry");
		add.addActionListener(e ->
		{
			exportRuleTable.addRow();
			exportRuleTable.changeSelection(exportRuleTable.getRowCount()-1, 0, false, false);
		});

		// add button to remove row
		JButton remove = new JButton(tr("Remove"), ImageProvider.get("dialogs/delete", ImageProvider.ImageSizes.SMALLICON));
		buttonPanel.add(remove);
		remove.setToolTipText("Remove selected items");
		remove.addActionListener(e -> exportRuleTable.removeRow(exportRuleTable.getSelectedItems()));

		// add button to export table
		JButton export = new JButton(tr("Export"), ImageProvider.get("save", ImageProvider.ImageSizes.SMALLICON));
		buttonPanel.add(export);
		export.setToolTipText("Export table");
		//		export.addActionListener(e -> exportSelectedToXML());

		// add button to import table from file
		JButton read = new JButton(tr("Read from file"), ImageProvider.get("open", ImageProvider.ImageSizes.SMALLICON));
		buttonPanel.add(read);
		read.setToolTipText("Read table from file");
		//		read.addActionListener(e -> readPreferencesFromXML());

		exportRuleTable.setRowSelectionInterval(0, 0); // select first row in table
		general.add(buttonPanel, GBC.eol());	// add all buttons to panel
		return general;
	}

	@Override
	public void addGui(PreferenceTabbedPane gui) {
		JPanel prefPanel = gui.createPreferenceTab(this);

		tabPane.add("General", createSettings());
		tabPane.add(createExportSettings(gui), GBC.eol().fill(GridBagConstraints.BOTH));

		prefPanel.add(tabPane, GBC.eol().fill(GridBagConstraints.BOTH));
	}

	@Override
	public boolean ok() {
		Config.getPref().putDouble(Osm2XConstants.PREF_INTERP_DIST, (Double)spinner.getModel().getValue());
		Config.getPref().put(Osm2XConstants.PREF_BARRIER_FILTER, tagsBarrier.getText());
		Config.getPref().putBoolean(Osm2XConstants.PREF_COPY_ADDRESS, checkCopyAdr.isSelected());
		Config.getPref().put(Osm2XConstants.PREF_HIGHWAY_FILTER, tagsHighway.getText());

		// save table to file
		try
		{
			prefIO.saveToFile(exportRuleTable.getItems());
		} catch (IOException e) {
			Logging.error(e);

			// warning dialog??

		}
		return false;
	}

}
