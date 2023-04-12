package de.sgd.josm.plugins.osm2x;

import java.io.File;
import java.io.IOException;

import org.openstreetmap.josm.actions.ExtensionFileFilter;
import org.openstreetmap.josm.gui.MapFrame;
import org.openstreetmap.josm.gui.preferences.PreferenceSetting;
import org.openstreetmap.josm.plugins.Plugin;
import org.openstreetmap.josm.plugins.PluginInformation;
import org.openstreetmap.josm.spi.preferences.Config;
import org.openstreetmap.josm.tools.Logging;

import de.sgd.josm.plugins.osm2x.helper.Osm2XConstants;
import de.sgd.josm.plugins.osm2x.io.NavExporter;
import de.sgd.josm.plugins.osm2x.io.SvgExporter;

public class Osm2XPlugin extends Plugin {

	//protected static Osm2XDialog osm2xDialog;

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
			newFrame.addToggleDialog(/*osm2xDialog = */new Osm2XDialog());
		}/* else {
			osm2xDialog = null;
		}*/
	}

	@Override
	public PreferenceSetting getPreferenceSetting()
	{
		// check if path to file is included in config
		String fileLoc = Config.getPref().get(Osm2XConstants.PREF_EXPORT_PREF_FILE);
		File file = new File(fileLoc.isEmpty() ? getPluginDirs().getPreferencesDirectory(false).getAbsolutePath() + "\\export_pref.xml" : fileLoc);

		if (!file.canRead())
		{
			// try to create new file
			Config.getPref().put(Osm2XConstants.PREF_EXPORT_PREF_FILE, file.getAbsolutePath());
			try {
				// create new file if file does not already exist
				file.createNewFile();
			} catch (IOException e) {
				Logging.error(e);
			}
		}

		return new Osm2XPreferenceSetting();
	}
}
