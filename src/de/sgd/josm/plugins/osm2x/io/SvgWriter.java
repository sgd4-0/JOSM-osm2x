package de.sgd.josm.plugins.osm2x.io;

import java.io.Closeable;
import java.io.FileWriter;
import java.io.IOException;

import org.openstreetmap.josm.data.coor.LatLon;

import de.sgd.josm.plugins.osm2x.io.svg.SvgDocument;

public class SvgWriter implements Closeable {

	FileWriter writer;
	LatLon origin;

	public SvgWriter(FileWriter writer) {
		this.writer = writer;
	}

	public void writeDataset(SvgDocument doc) throws IOException {
		if (doc != null)
		{
			doc.writeToFile(writer);
		}
	}

	@Override
	public void close() throws IOException {
		// TODO Auto-generated method stub
		writer.close();
	}
}
