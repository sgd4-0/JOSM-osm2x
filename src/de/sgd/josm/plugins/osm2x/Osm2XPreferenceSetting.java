package de.sgd.josm.plugins.osm2x;

import static org.openstreetmap.josm.tools.I18n.tr;

import java.awt.GridBagLayout;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSpinner;
import javax.swing.SpinnerNumberModel;

import org.openstreetmap.josm.gui.preferences.DefaultTabPreferenceSetting;
import org.openstreetmap.josm.gui.preferences.PreferenceTabbedPane;
import org.openstreetmap.josm.spi.preferences.Config;
import org.openstreetmap.josm.tools.GBC;

import de.sgd.josm.plugins.osm2x.helper.Osm2XConstants;

public class Osm2XPreferenceSetting extends DefaultTabPreferenceSetting {

	public JPanel prefPanel;

	private final JSpinner spinner = new JSpinner(new SpinnerNumberModel(5.0, 2.0, null, 0.1));

	public Osm2XPreferenceSetting() {
		super(Osm2XConstants.ICON_PREF_48,
				tr("Osm2X Preferences"),
				tr("Convert and export OSM data to be used on the Shared Guide Dog 4.0"));
	}

	protected JPanel createSettings() {
		JPanel general = new JPanel(new GridBagLayout());
		general.setBorder(BorderFactory.createEmptyBorder(0, 0, 0, 0));
		general.setName(tr("Osm2X preferences"));

		// options for interpolation
		JLabel jLabelSpinner = new JLabel(tr("Maximum way length:"));
		spinner.getModel().setValue(Config.getPref().getDouble(Osm2XConstants.PREF_INTERP_DIST, 5.0));
		general.add(jLabelSpinner, GBC.std().insets(0, 5, 10, 0));
		general.add(spinner, GBC.eol().fill(GBC.HORIZONTAL).insets(5, 5, 200, 5));

		// end of dialog, scroll bar
		general.add(Box.createVerticalGlue(), GBC.eol().fill(GBC.VERTICAL));

		return general;
	}

	@Override
	public void addGui(PreferenceTabbedPane gui) {
		prefPanel = gui.createPreferenceTab(this);

		JScrollPane scrollpane = new JScrollPane(createSettings());
		scrollpane.setBorder(BorderFactory.createEmptyBorder(0, 0, 0, 0));
		prefPanel.add(scrollpane, GBC.eol().fill(GBC.BOTH));
	}

	@Override
	public boolean ok() {
		Config.getPref().putDouble(Osm2XConstants.PREF_INTERP_DIST, (Double)spinner.getModel().getValue());
		return false;
	}

}
