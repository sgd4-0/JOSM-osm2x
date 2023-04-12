package de.sgd.josm.plugins.osm2x.io;

import static org.openstreetmap.josm.tools.I18n.tr;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

import org.openstreetmap.josm.actions.ExtensionFileFilter;
import org.openstreetmap.josm.gui.io.importexport.OsmExporter;
import org.openstreetmap.josm.gui.layer.OsmDataLayer;

public class NavExporter extends OsmExporter {

	public NavExporter() {
		super(new ExtensionFileFilter("nav", "nav", tr("SGD navigation files") + " (*.nav)"));
	}

	@Override
	protected void doSave(File file, OsmDataLayer layer) throws IOException {
		// Save map data to *.nav file and adresses to *.adr file
		String navPath = file.getAbsolutePath();
		if (!navPath.endsWith(".nav"))
		{
			navPath = navPath.concat(".nav");
		}

		try (FileWriter navWriter = new FileWriter(navPath);
				NavWriter w = new NavWriter(navWriter);)
		{
			layer.data.getReadLock().lock();
			try {
				w.writeDataset(layer.getDataSet());
			} finally {
				layer.data.getReadLock().unlock();
			}
		}

		String adrPath = navPath.replaceFirst(".nav", ".json");
		File adrFile = new File(adrPath);
		try (FileWriter adrWriter = new FileWriter(adrFile);
				AdrWriter w = new AdrWriter(adrWriter);)
		{
			layer.data.getReadLock().lock();
			try
			{
				w.writeDataset(layer.getDataSet());
			} finally {
				layer.data.getReadLock().unlock();
			}
		}
	}
}