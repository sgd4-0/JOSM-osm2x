package de.sgd.josm.plugins.osm2x.gui;

import static org.openstreetmap.josm.tools.I18n.tr;

import java.awt.BorderLayout;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.ArrayList;
import java.util.List;

import javax.swing.AbstractAction;
import javax.swing.BorderFactory;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.SwingConstants;

import org.openstreetmap.josm.data.osm.Filter;
import org.openstreetmap.josm.data.osm.search.SearchSetting;
import org.openstreetmap.josm.gui.MainApplication;
import org.openstreetmap.josm.gui.dialogs.FilterTableModel;
import org.openstreetmap.josm.gui.util.GuiHelper;
import org.openstreetmap.josm.tools.GBC;
import org.openstreetmap.josm.tools.ImageProvider;
import org.openstreetmap.josm.tools.InputMapUtils;
import org.openstreetmap.josm.tools.Logging;

/**
 * Display the mesher checker window
 *
 */
public class MesherCheckerDialog extends JDialog {

	public static final int WIDTH_ATTRIBUTE = 0;
	public static final int AREA_IN_DATASET = 1;
	public static final int ENTRANCE_CONNECT = 2;
	public static final int NODE_OUT_DOWNLOAD = 3;
	public static final int ALL_NODES_CONNECT = 4;

	public static final int ICON_UNKNOWN = 0;
	public static final int ICON_FAILURE = 1;
	public static final int ICON_WARNING = 2;
	public static final int ICON_SUCCESS = 3;

	private List<JLabel> labels = new ArrayList<>();
	private List<ImageIcon> imIcons = new ArrayList<>();

	private JButton okButton;

	public MesherCheckerDialog() {
		super(MainApplication.getMainFrame(), tr("Mesher Checker"), ModalityType.MODELESS);

		this.setMinimumSize(new Dimension(500, 300));
		// set the maximum width to the current screen. If the dialog is opened on a
		// smaller screen than before, this will reset the stored preference.
		this.setMaximumSize(GuiHelper.getScreenSize());
		this.setLocation((GuiHelper.getScreenSize().width-this.getWidth())/2,
				(GuiHelper.getScreenSize().height-this.getHeight())/2);

		// create icons
		imIcons.add(ImageProvider.get("dialogs/unknown", ImageProvider.ImageSizes.HTMLINLINE));
		imIcons.add(ImageProvider.get("dialogs/error", ImageProvider.ImageSizes.LARGEICON));
		imIcons.add(ImageProvider.get("dialogs/warning", ImageProvider.ImageSizes.LARGEICON));
		imIcons.add(ImageProvider.get("dialogs/ok", ImageProvider.ImageSizes.LARGEICON));

		Container c = getContentPane();
		c.setLayout(new BorderLayout());
		c.add(buildGui(), BorderLayout.CENTER);

		c.add(buildActionPanel(), BorderLayout.SOUTH);
		addWindowListener(new WindowEventHandler());

		InputMapUtils.addEscapeAction(getRootPane(), new CancelAction());
	}

	protected JPanel buildGui()
	{
		GridBagLayout layout = new GridBagLayout();
		JPanel pDialog = new JPanel(layout);

		// create labels
		labels.add(new JLabel("width attribute", imIcons.get(ICON_UNKNOWN), SwingConstants.LEFT));
		labels.add(new JLabel("no areas in dataset", imIcons.get(ICON_UNKNOWN), SwingConstants.LEFT));
		labels.add(new JLabel("all entrances connected", imIcons.get(ICON_UNKNOWN), SwingConstants.LEFT));
		labels.add(new JLabel("nodes outside downloaded area", imIcons.get(ICON_UNKNOWN), SwingConstants.LEFT));
		labels.add(new JLabel("all nodes connected", imIcons.get(ICON_UNKNOWN), SwingConstants.LEFT));

		// create filters
		List<FilterEntry> filters = new ArrayList<>();
		filters.add(new FilterEntry("sgd_width=*", false));
		filters.add(new FilterEntry("closed", true));
		filters.add(new FilterEntry("-entrance=* | ways:1-", false));
		filters.add(new FilterEntry("allindownloadedarea", false));
		filters.add(null);

		for (int i = 0; i < labels.size(); i++)
		{
			labels.get(i).setIconTextGap(10);
			pDialog.add(labels.get(i), lblGBC(i));

			if (filters.get(i) != null)
				pDialog.add(new JButton(new FilterAction(filters.get(i))), btnGBC(i));
		}

		pDialog.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));

		return pDialog;
	}

	/**
	 * Sets the icon for the label associated with the test
	 * @param checkType
	 * @param result
	 */
	public void setResult(int checkType, int result)
	{

		if (inRange(checkType, 0, labels.size()) && inRange(result, 0, imIcons.size()))
		{
			labels.get(checkType).setIcon(imIcons.get(result));
		} else
		{
			Logging.warn(String.format("Sent request to set result label but number is out of range: checkType: %d, result: %d", checkType, result));
		}
	}

	/**
	 * Sets the icon for the label associated with the test
	 * @param checkType
	 * @param result
	 * @param text Text to display in label
	 */
	public void setResult(int checkType, int result, String text)
	{
		if (inRange(checkType, 0, labels.size()) && inRange(result, 0, imIcons.size()))
		{
			labels.get(checkType).setIcon(imIcons.get(result));
			labels.get(checkType).setText(text);
		} else
		{
			Logging.warn(String.format("Sent request to set result label but number is out of range: checkType: %d, result: %d", checkType, result));
		}
	}

	/**
	 * Tells the mesher checker dialog that all tests completed with at least
	 * a warning and the user can proceed with meshing
	 */
	public void setTestsSuccessful()
	{
		okButton.setEnabled(true);
		okButton.setToolTipText("Continue with meshing");
	}

	protected JPanel buildActionPanel() {
		JPanel pnl = new JPanel(new GridBagLayout());

		JPanel btns = new JPanel(new FlowLayout(FlowLayout.CENTER));
		btns.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));

		okButton = new JButton(new OKAction());
		okButton.setEnabled(false);				// continue button is disabled until all checks are completed
		btns.add(okButton);
		btns.add(new JButton(new CancelAction()));
		btns.add(new JButton(new RefreshAction()));
		pnl.add(btns, GBC.std().fill(GridBagConstraints.HORIZONTAL));
		return pnl;
	}

	/**
	 * Checks if a number is greater than or equal to min and less than max
	 * @param number the number to check
	 * @param min lower bound (inclusive)
	 * @param max upper bound (exclusive)
	 * @return true if number is in interval [min, max)
	 */
	private static boolean inRange(int number, int min, int max)
	{
		return (number >= min) && (number < max);
	}

	/**
	 * Create constraints for buttons
	 * @param gridy
	 * @return
	 */
	private GridBagConstraints btnGBC(int gridy)
	{
		GridBagConstraints gbc = new GridBagConstraints();
		gbc.gridx = 1;
		gbc.gridy = gridy;

		return new GridBagConstraints(1, gridy, 1, 1, 0.0, 0.5, GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 0), 0, 0);
	}

	/**
	 * Create constraints for labels
	 * @param gridy
	 * @return
	 */
	private GridBagConstraints lblGBC(int gridy)
	{
		return new GridBagConstraints(0, gridy, 1, 1, 0.5, 0.5, GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 0), 0, 0);
	}

	class CancelAction extends AbstractAction {
		CancelAction() {
			putValue(NAME, tr("Cancel"));
			new ImageProvider("cancel").getResource().attachImageIcon(this);
			putValue(SHORT_DESCRIPTION, tr("Close this dialog and stop meshing"));
		}

		public void cancel() {
			dispose();
		}

		@Override
		public void actionPerformed(ActionEvent evt) {
			cancel();
		}
	}

	class OKAction extends AbstractAction {
		OKAction() {
			putValue(NAME, tr("Continue"));
			new ImageProvider("ok").getResource().attachImageIcon(this);
			putValue(SHORT_DESCRIPTION, tr("Meshing is not possible because one or more tests failed"));
		}

		@Override
		public void actionPerformed(ActionEvent evt) {
			dispose();
		}
	}

	class RefreshAction extends AbstractAction {
		RefreshAction() {
			putValue(NAME, tr("Refresh"));
			new ImageProvider("restart").getResource().attachImageIcon(this);
			putValue(SHORT_DESCRIPTION, tr("Rerun the tests"));
		}

		@Override
		public void actionPerformed(ActionEvent evt) {
			// TODO restart tests
		}
	}

	class FilterAction extends AbstractAction {
		private Filter filter;

		FilterAction(FilterEntry filter) {
			this.filter = new Filter(SearchSetting.readFromString("C " + filter.getFilter()));
			this.filter.inverted = filter.isInverted();

			putValue(NAME, tr("Set filter"));
			new ImageProvider("dialogs/filter").getResource().attachImageIcon(this);
			putValue(SHORT_DESCRIPTION, tr("Set a filter"));
		}

		@Override
		public void actionPerformed(ActionEvent evt) {
			// activate filter
			FilterTableModel ftm = MainApplication.getMap().filterDialog.getFilterModel();
			ftm.addFilter(filter);
			ftm.executeFilters();
		}
	}

	class FilterEntry {
		private String filter;
		private boolean inverted;

		public FilterEntry(String filter, boolean inverted) {
			this.filter = filter;
			this.inverted = inverted;
		}

		public String getFilter()
		{
			return filter;
		}

		public boolean isInverted()
		{
			return inverted;
		}
	}

	class WindowEventHandler extends WindowAdapter {
		@Override
		public void windowClosing(WindowEvent arg0) {
			new CancelAction().cancel();
		}
	}

}
