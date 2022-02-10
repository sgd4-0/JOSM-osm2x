package de.sgd.josm.plugins.osm2x;

import org.openstreetmap.josm.actions.ExtensionFileFilter;
import org.openstreetmap.josm.gui.MapFrame;
import org.openstreetmap.josm.gui.preferences.PreferenceSetting;
import org.openstreetmap.josm.plugins.Plugin;
import org.openstreetmap.josm.plugins.PluginInformation;

import de.sgd.josm.plugins.osm2x.io.NavExporter;
import de.sgd.josm.plugins.osm2x.io.SvgExporter;

public class Osm2XPlugin extends Plugin {

	//private IconToggleButton btn;
	protected static Osm2XDialog osm2xDialog;

	public Osm2XPlugin(PluginInformation info) {
		super(info);
		// init your plugin
		ExtensionFileFilter.addExporter(new NavExporter());
		ExtensionFileFilter.addExporter(new SvgExporter());
	}

	@Override
	public void mapFrameInitialized(MapFrame oldFrame, MapFrame newFrame) {
		// Plugins implementing a method mapFrameInitialized are notified about the change of the current map frame
		if (newFrame != null) {
			newFrame.addToggleDialog(osm2xDialog = new Osm2XDialog());
		} else {
			osm2xDialog = null;
		}
	}

	@Override
	public PreferenceSetting getPreferenceSetting() {
		return new Osm2XPreferenceSetting();
	}

	//public void addDownloadSelection(List<DownloadSelection> list) {
	//
	//}
}
