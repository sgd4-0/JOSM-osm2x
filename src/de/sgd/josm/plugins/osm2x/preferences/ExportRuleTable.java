// License: GPL. For details, see LICENSE file.
package de.sgd.josm.plugins.osm2x.preferences;

import java.awt.Component;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.stream.Collectors;

import javax.swing.JComponent;
import javax.swing.JTable;
import javax.swing.table.AbstractTableModel;

/**
 * Component for editing list of preferences as a table.
 * @since 6021
 */
public class ExportRuleTable extends JTable {
	private final ExportRuleTableModel model;

	/**
	 * Constructs a new {@code PreferencesTable}.
	 * @param displayData The list of preferences entries to display
	 */
	public ExportRuleTable(List<ExportRuleEntry> displayData) {
		model = new ExportRuleTableModel(displayData);
		setModel(model);
		putClientProperty("terminateEditOnFocusLost", Boolean.TRUE);

		addMouseListener(new MouseAdapter() {
			@Override
			public void mouseClicked(MouseEvent e) {
				if (e.getClickCount() == 2) {
					editPreference(ExportRuleTable.this);
				}
			}
		});
	}

	/**
	 * This method should be called when displayed data was changed form external code
	 */
	public void fireDataChanged() {
		model.fireTableDataChanged();
	}

	/**
	 * The list of currently selected rows
	 * @return newly created list of ExportRuleEntry
	 */
	public List<ExportRuleEntry> getSelectedItems() {
		return Arrays.stream(getSelectedRows()).mapToObj(row -> (ExportRuleEntry) model.getValueAt(row, -1)).collect(Collectors.toList());
	}

	public List<ExportRuleEntry> getItems()
	{
		List<ExportRuleEntry> l = new ArrayList<>();
		for (int i = 0; i < model.getRowCount(); i++)
		{
			l.add((ExportRuleEntry) model.getValueAt(i, -1));
		}
		return l;
	}

	/**
	 * Add a new empty row
	 */
	public void addRow()
	{
		model.addRow();
	}

	/**
	 * Remove specified objects from table
	 * @param l list with objects to remove
	 */
	public void removeRow(List<ExportRuleEntry> l)
	{
		model.removeRow(l);
	}

	/**
	 * Switch positions of two rows
	 * @param selectedRow the selected row
	 * @param nextRow The row to which the row is to be shifted
	 */
	public void switchRows(int selectedRow, int nextRow)
	{
		model.switchRows(selectedRow, nextRow);
	}

	/**
	 * Call this to edit selected row in preferences table
	 * @param gui - parent component for messagebox
	 * @return true if editing was actually performed during this call
	 */
	public boolean editPreference(final JComponent gui) {
		if (getSelectedRowCount() != 1) {
			return false;
		}

		editCellAt(getSelectedRow(), 1);
		Component editor = getEditorComponent();
		if (editor != null) {
			editor.requestFocus();
			return true;
		}
		return false;
	}

	/**
	 * Table model used for ExportRuleTable
	 *
	 */
	final class ExportRuleTableModel extends AbstractTableModel {

		private String[] columnNames = {"Filter expression", "Default value"};
		private ArrayList<ExportRuleEntry> rowEntries;

		ExportRuleTableModel(Collection<ExportRuleEntry> c) {
			// create new list and read file
			rowEntries = new ArrayList<>(c);
		}

		@Override
		public boolean isCellEditable(int row, int column) {
			return true;
		}

		@Override
		public int getRowCount() {
			return rowEntries.size();
		}

		@Override
		public Object getValueAt(int row, int column) {
			if (column == -1)
				return rowEntries.get(row);
			else if (column == 0)
				return rowEntries.get(row).getFilter();
			else
				return rowEntries.get(row).getValue();
		}

		@Override
		public void setValueAt(Object o, int row, int column) {
			if (column == 0) {
				rowEntries.get(row).setFilter((String) o);
			} else {
				// TODO check input
				rowEntries.get(row).setValue((String) o);
			}
		}

		@Override
		public String getColumnName(int col)
		{
			if (col >= 0 && col < 2)
			{
				return columnNames[col];
			}
			return "";
		}

		@Override
		public int getColumnCount() {
			return 2;
		}

		/**
		 * Add new row
		 */
		public void addRow()
		{
			ExportRuleEntry ere = new ExportRuleEntry("expression", "[R|W|D]#.#");
			rowEntries.add(ere);
			// update table
			model.fireTableRowsInserted(rowEntries.size()-2, rowEntries.size()-1);
		}

		public void removeRow(List<ExportRuleEntry> l)
		{
			rowEntries.removeAll(l);
			model.fireTableDataChanged();
		}

		/**
		 * Switches the contents of row1 and row2
		 * @param row1
		 * @param row2
		 */
		public void switchRows(int row1, int row2)
		{
			ExportRuleEntry row1Entry = rowEntries.get(row1);
			rowEntries.set(row1, rowEntries.get(row2));
			rowEntries.set(row2, row1Entry);
			model.fireTableDataChanged();
		}

		/**
		 * Get all items from the table
		 * @return all items from the table
		 */
		public List<ExportRuleEntry> getItems()
		{
			return rowEntries;
		}
	}
}
