package de.sgd.josm.plugins.osm2x.io;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import javax.xml.stream.XMLInputFactory;
import javax.xml.stream.XMLStreamConstants;
import javax.xml.stream.XMLStreamException;
import javax.xml.stream.XMLStreamReader;

import org.openstreetmap.josm.tools.Logging;

import de.sgd.josm.plugins.osm2x.preferences.ExportRuleEntry;

public class ExportPrefIO
{
	private File file;

	/**
	 * Reader / Writer for export rule table
	 * @param file file to read from / write to
	 */
	public ExportPrefIO(File file) {
		this.file = file;
	}

	/**
	 * Read export table preferences from file
	 * @return
	 * @throws IOException
	 * @throws XMLStreamException
	 * @throws FileNotFoundException
	 */
	public List<ExportRuleEntry> readFromFile() throws IOException
	{
		XMLInputFactory inputFactory = XMLInputFactory.newInstance();

		List<ExportRuleEntry> l = new ArrayList<>();
		try (FileReader reader = new FileReader(file)) {
			XMLStreamReader xmlReader = inputFactory.createXMLStreamReader(reader);

			while (xmlReader.hasNext())
			{
				int eventType = xmlReader.next();
				if (eventType == XMLStreamConstants.START_ELEMENT && xmlReader.getLocalName().equals("entry"))
				{
					// read entry
					ExportRuleEntry entry = new ExportRuleEntry("<empty>", "<empty>");
					entry.setFilter(xmlReader.getAttributeValue(null, "filter"));
					entry.setValue(xmlReader.getAttributeValue(null, "value"));
					l.add(entry);
				}
			}
			xmlReader.close();

		} catch (XMLStreamException xmle) {
			Logging.error(xmle);
		} catch (FileNotFoundException fnfe)
		{
			// do something
			Logging.error(fnfe);
		}
		return l;
	}

	/**
	 * Save a key value pair to file
	 * @param key
	 * @param value
	 * @throws IOException
	 */
	public void saveToFile(List<ExportRuleEntry> entries) throws IOException
	{
		try (FileWriter writer = new FileWriter(file))
		{
			writer.write("<exportPrefs>" + System.lineSeparator());
			for (ExportRuleEntry e : entries)
			{
				writer.write(String.format("\t<entry filter=\"%s\" value=\"%s\"/>" + System.lineSeparator(),
						e.getFilter().replace("\"", "&quot;"), e.getValue().replace("\"", "&quot;")));
			}
			writer.write("</exportPrefs>");
		}
	}

}
