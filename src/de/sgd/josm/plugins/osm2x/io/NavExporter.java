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
		String adr_path = file.getAbsolutePath();
		adr_path = adr_path.replaceFirst(".nav", ".adr");
		File adr_file = new File(adr_path);

		try (FileWriter writer = new FileWriter(file);
				NavWriter w = new NavWriter(writer);)
		{
			layer.data.getReadLock().lock();
			try {
				w.writeDataset(layer.getDataSet());
			} finally {
				layer.data.getReadLock().unlock();
			}
		}

		try (FileWriter writer = new FileWriter(adr_file);
				AdrWriter w = new AdrWriter(writer);)
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