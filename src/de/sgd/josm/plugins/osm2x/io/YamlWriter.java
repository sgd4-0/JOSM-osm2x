package de.sgd.josm.plugins.osm2x.io;

import java.io.Closeable;
import java.io.FileWriter;
import java.io.IOException;

import de.sgd.josm.plugins.osm2x.io.svg.SvgDocument;

public class YamlWriter implements Closeable {

	FileWriter writer;

	public YamlWriter(FileWriter writer) {
		this.writer = writer;
	}

	public void writeDataset(SvgDocument doc) throws IOException {
		writer.write("image: <path.png>  # convert the svg file to a pixel image and put path here\n");
		writer.write("# The resolution depends on the resolution of the image\n");
		writer.write(String.format("# resolution = %.3f m / image_width in px\n", Math.ceil(doc.getDocSize()[0])));
		writer.write("resolution:     # how much meters per pixel, recommended <= 0.1\n");
		double[] d = doc.getMapOrigin();
		writer.write(String.format("# Global coordinates of map origin: lat=%.7f, lon=%.7f\n", d[0], d[1]));
		writer.write("origin: [0.000000, 0.000000, 0.000000]    # origin is set to bottom left corner\n");
		writer.write("negate: 0\n");
		writer.write("occupied_thresh: 0.65\n");
		writer.write("free_thresh: 0.1");
	}

	@Override
	public void close() throws IOException {
		writer.close();
	}


}
