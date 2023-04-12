package de.sgd.josm.plugins.osm2x.modules;

import javax.swing.JOptionPane;

import org.openstreetmap.josm.data.osm.Node;
import org.openstreetmap.josm.data.osm.Way;
import org.openstreetmap.josm.gui.MainApplication;
import org.openstreetmap.josm.gui.layer.Layer;

import de.sgd.josm.plugins.osm2x.gui.MesherCheckerDialog;

/**
 * Check map before meshing
 *
 */
public class Osm2XMesherChecker {

	/**
	 * The Test could not be performed
	 */
	public static final int UNCHECKED = 0;

	/**
	 * The test failed and the error cannot be solved automatically
	 */
	public static final int FAILURE = 1;

	/**
	 * The test failed, but the error can be solved automatically
	 */
	public static final int WARNING = 2;

	/**
	 * The test succeeded
	 */
	public static final int SUCCESS = 3;

	/**
	 * Mesher checker dialog
	 */
	private MesherCheckerDialog mcd;

	public Osm2XMesherChecker()
	{
		// currently empty
	}

	/**
	 * Open mesher checker dialog and perform checks
	 * @return
	 */
	public int doChecks()
	{
		// check if a layer named 'highways' is present
		boolean hasHighwayLayer = false;
		for (Layer l : MainApplication.getLayerManager().getLayers())
		{
			if (l.getName().equalsIgnoreCase("highways"))
			{
				hasHighwayLayer = true;
				MainApplication.getLayerManager().setActiveLayer(l);
				break;
			}
		}

		if (!hasHighwayLayer)
		{
			// show warning dialog
			JOptionPane.showMessageDialog(MainApplication.getMainFrame(),
					"No layer named 'highways' present. Make sure that a layer is available with all the highways to be meshed.",
					"Missing highways layer", JOptionPane.WARNING_MESSAGE);
			return FAILURE;
		}

		// first: display dialog, then perform checks
		mcd = new MesherCheckerDialog();
		mcd.setVisible(true);

		if (MainApplication.getLayerManager().getActiveDataSet() != null)
		{
			int retCode = 3;
			// we want to perform 5 checks:
			// - width attribute
			int tmpRet = checkWidthAttribute();

			retCode = tmpRet < retCode ? tmpRet : retCode;

			// - no areas in dataset
			tmpRet = checkAreaInDataset();
			retCode = tmpRet < retCode ? tmpRet : retCode;

			// - all entrances connected
			tmpRet = checkEntrancesConnected();
			retCode = tmpRet < retCode ? tmpRet : retCode;

			// - no nodes outside downloaded area
			tmpRet = checkNodeOutsideDownload();

			retCode = tmpRet < retCode ? tmpRet : retCode;

			// - all nodes connected
			tmpRet = checkAllNodesConnected();
			mcd.setResult(MesherCheckerDialog.ALL_NODES_CONNECT, tmpRet);
			retCode = tmpRet < retCode ? tmpRet : retCode;

			if (retCode > 1)
				mcd.setTestsSuccessful();

			return retCode;
		}

		return FAILURE;
	}

	/**
	 * Checks if all ways have a width attribute
	 * @return
	 */
	private int checkWidthAttribute()
	{
		int counter = 0;
		for (Way w : MainApplication.getLayerManager().getActiveDataSet().getWays())
		{
			counter = (w.hasTag("sgd_width") && w.isUsable()) ? counter : counter+1;
		}

		if (counter > 0)
		{
			mcd.setResult(MesherCheckerDialog.WIDTH_ATTRIBUTE, WARNING, String.format("%d ways with missing 'sgd_width' tag", counter));
			return WARNING;
		}

		mcd.setResult(MesherCheckerDialog.WIDTH_ATTRIBUTE, SUCCESS);
		return SUCCESS;
	}

	/**
	 * Check if dataset contains areas
	 * @return
	 */
	private int checkAreaInDataset()
	{
		int counter = 0;
		for (Way w : MainApplication.getLayerManager().getActiveDataSet().getWays())
		{
			counter = w.isArea() ? counter+1 : counter;
		}

		if (counter > 0)
		{
			mcd.setResult(MesherCheckerDialog.AREA_IN_DATASET, WARNING, String.format("%d areas detected", counter));
			return WARNING;
		}
		mcd.setResult(MesherCheckerDialog.AREA_IN_DATASET, SUCCESS);
		return SUCCESS;
	}

	/**
	 * Check if every entrance is connected to at least one way
	 * @return
	 */
	private int checkEntrancesConnected()
	{
		int unconnEntr = 0;
		for (Node n : MainApplication.getLayerManager().getActiveDataSet().getNodes())
		{
			if (n.hasTag("entrance") && !n.isDeleted())
			{
				boolean onlyAreas = true;
				for (Way w : n.getParentWays())
				{
					onlyAreas &= w.isArea();
				}
				if (onlyAreas)
					unconnEntr++;
			}
		}

		if (unconnEntr > 0)
		{
			mcd.setResult(MesherCheckerDialog.ENTRANCE_CONNECT, FAILURE, String.format("%d unconnected entrances", unconnEntr));
			return FAILURE;
		}

		mcd.setResult(MesherCheckerDialog.ENTRANCE_CONNECT, SUCCESS);
		return SUCCESS;
	}

	/**
	 * Count nodes outside downloaded area
	 * @return
	 */
	private int checkNodeOutsideDownload()
	{
		int counter = 0;
		for (Node n : MainApplication.getLayerManager().getActiveDataSet().getNodes())
		{
			counter = (n.isOutsideDownloadArea() && !n.isDeleted()) ? counter+1 : counter;
		}

		if (counter > 0)
		{
			mcd.setResult(MesherCheckerDialog.NODE_OUT_DOWNLOAD, WARNING, String.format("%d nodes outside downloaded area", counter));
			return WARNING;
		}
		mcd.setResult(MesherCheckerDialog.NODE_OUT_DOWNLOAD, SUCCESS);
		return SUCCESS;
	}

	/**
	 * Check if all nodes are connected to each other (only one net)
	 * @return
	 */
	private int checkAllNodesConnected()
	{
		// check what to do here
		return SUCCESS;
	}
}
