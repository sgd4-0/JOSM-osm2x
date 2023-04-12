// License: GPL. For details, see LICENSE file.
package de.sgd.josm.plugins.osm2x.preferences;

/**
 * Class to store key value pairs used in ExportPrefTable class
 */
public class ExportRuleEntry {
	private String expression;
	private String value;

	/**
	 * Constructs a new {@code PrefEntry}.
	 * @param key The preference key
	 * @param value The preference value
	 * @param defaultValue The preference default value
	 * @param isDefault determines if the current value is the default value
	 */
	public ExportRuleEntry(String expression, String value) {
		ensureParameterValid(expression);
		ensureParameterValid(value);
		this.expression = expression;
		this.value = value;
	}

	/**
	 * Returns the string used as filter.
	 * @return the string used as filter
	 */
	public String getFilter() {
		return expression;
	}

	/**
	 * Returns the value.
	 * @return the value
	 */
	public String getValue() {
		return value;
	}

	/**
	 * Set the filter expression
	 * @param key
	 */
	public void setFilter(String expression)
	{
		ensureParameterValid(expression);
		this.expression = expression;
	}

	/**
	 * Sets the preference value.
	 * @param value the preference value
	 */
	public void setValue(String value) {
		this.value = value;
	}

	private void ensureParameterValid(String parameter)
	{
		if (parameter == null || parameter.isEmpty())
		{
			throw new IllegalArgumentException("Parameter must not be null or emtpy");
		}
	}

	@Override
	public String toString()
	{
		return String.format("\"%s\":\"%s\"", expression.replace("\"", "\\\""), value.replaceAll("\"", "\\\""));
	}
}
