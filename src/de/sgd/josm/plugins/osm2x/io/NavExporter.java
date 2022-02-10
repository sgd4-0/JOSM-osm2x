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
		String nav_path = file.getAbsolutePath();
		if (!nav_path.endsWith(".nav"))
		{
			nav_path = nav_path.concat(".nav");
		}

		try (FileWriter navWriter = new FileWriter(nav_path);
				NavWriter w = new NavWriter(navWriter);)
		{
			layer.data.getReadLock().lock();
			try {
				w.writeDataset(layer.getDataSet());
			} finally {
				layer.data.getReadLock().unlock();
			}
		}

		String adr_path = nav_path.replaceFirst(".nav", ".adr");
		File adr_file = new File(adr_path);
		try (FileWriter adrWriter = new FileWriter(adr_file);
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