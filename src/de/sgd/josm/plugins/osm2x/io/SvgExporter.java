package de.sgd.josm.plugins.osm2x.io;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

import org.openstreetmap.josm.actions.ExtensionFileFilter;
import org.openstreetmap.josm.gui.io.importexport.OsmExporter;
import org.openstreetmap.josm.gui.layer.OsmDataLayer;

public class SvgExporter extends OsmExporter {

	public SvgExporter() {
		super(new ExtensionFileFilter("svg", "svg", "Scalable Vector Graphics (SGD) (*.svg)"));
	}

	@Override
	protected void doSave(File file, OsmDataLayer layer) throws IOException {
		// Save map data to *.svg file and create .yaml config file
		String adr_path = file.getAbsolutePath();
		adr_path = adr_path.replaceFirst(".svg", ".yaml");
		File adr_file = new File(adr_path);

		try (FileWriter writer = new FileWriter(file);
				SvgWriter w = new SvgWriter(writer);)
		{
			layer.data.getReadLock().lock();
			try {
				w.writeDataset(layer.getDataSet());
			} finally {
				layer.data.getReadLock().unlock();
			}
		}

		/*
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
		}*/
	}
}
