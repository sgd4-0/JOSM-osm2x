# Shared Guide Dog 4.0 Map Creator Plugin for [JOSM](https://josm.openstreetmap.de)

[![license: GPLv2 or later](https://img.shields.io/badge/license-GPLv2_or_later-blue.svg?style=flat-square&maxAge=7200)](/GPL-v2.0.txt)

A plugin that prepares and exports OSM data for use on the [Shared Guide Dog 4.0](https://www.haw-hamburg.de/forschung/forschungsprojekte-detail/project/project/show/shared-guide-dog-40/).


* Maintainers:
  * Pascal Stahr <pascal.stahr@haw-hamburg.de>
* License: [GPL v2 or later](./LICENSE.md)


# Einleitung

Das JOSM Plugin *Shared Guide Dog 4.0 Map Creator* unterstützt bei der Verarbeitung von OpenStreetMap Karten für den Shared Guide Dog 4.0 (SGD4.0). Das Plugin ermöglicht den Export von Hinderniskarten und bei der Generierung und dem Export von Wegenetzen. Die exportierten Daten können direkt auf dem SGD4.0 genutzt werden.

Nach dem Laden einer OSM-Karte kann das Plugin aus der linken Toolbar aufgerufen werden. Auf der rechten Seite erscheint das Hauptfenster des SGD4.0 Map Creator Plugins. Das Fenster teilt sich in eine Anzeigefläche und drei Buttons. In der Anzeigefläche werden Daten zur geladenen Karte angezeigt. Mit den Buttons können die Funktionen des Plugins aufgerufen werden.

![Main window of the Shared Guide Dog 4.0 Map Creator plugin](/doc/main_window.png)

Funktionen der Buttons:
- Count Nodes: Berechnet die angezeigten Statistiken neu
- Split: Teilt die Karte in eine Hinderniskarte und eine Highway-Karte
- Mesh: Vernetzt die Highwaykarte (siehe [Meshing](Readme.md#generierung-und-export-eines-wegenetzes))

# Export von Hinderniskarten

## Vorbereitungen

Durch einen Klick auf den Split-Button wird eine Hinderniskarte und eine Wegkarte erstellt. Die Hinderniskarte enthält alle Hindernisse, wie beispielsweise Gebäude, Poller, Schranken, Wände und vieles mehr. Grundlage für die Aufteilung in Hindernis und Wege sind Filter, die über die Einstellungen angepasst werden können. 

![Preferences window](/doc/preferences_barrier.png)

Der Standard-Filter für die Hinderniskarte lautet: `barrier=* | natural=* | building=*`
Der Filter kann je nach Bedürfnis angepasst werden. Es wird die gleiche Syntax wie für den JOSM Filter verwendet.
Die Checkbox _Copy address to node_ ermöglicht, dass die Attribute der Gebäude zu den Entrance-Nodes kopiert werden. Im Normalfall ist dieses Verhalten erwünscht, da die Adressen für die Wegplanung benötigt werden und dann einzelnen Knoten zugeordnet sein müssen.

## Export

Die folgende Grafik zeigt die Hinderniskarte in JOSM nach dem Splitting. Vor dem Export muss die Karte in den meisten Fällen noch angepasst werden. Zu beachten ist, dass zum Beispiel Baumreihen, die durch einen Weg in OSM dargestellt werden, als Wand exportiert werden würden. Hier müssen demnach der Weg entfernt und einzelne Nodes gesetzt werden.

![Barrier map in JOSM](/doc/barrier_map.png)

Ist die Karte fertig bearbeitet, erfolgt der Export als Scalable Vector Graphic (SVG). Damit geht keine Genauigkeit durch eine Rasterisierung verloren und das Bild kann mit jedem üblichen Vektoreditor bearbeitet werden. Die Karte nach dem Export ist in der folgenden Grafik dargestellt. Zu beachten ist die Baumreihe auf der rechten Seite, die nun durch einen Strich dargestellt ist.

![Barrier map after exporting to svg](/doc/barrier_map_svg.png)


## Datenformat

Als Datenformat für den Export wird das Scalable Vector Graphics (SVG) Format verwendet. Es ermöglicht die effiziente Speicherung von großen Karten und die einfache Bearbeitung und Anzeige durch verschiedenste Programme.

Die Karte für den Shared Guide Dog 4.0 besteht aus den svg Elementen _circle_ und _path_. Neben den Positionsinformationen erhält jedes Objekt eine Klasse zugewiesen. Die Klasse leitet sich aus den OSM-Attributen ab. So wird beispielsweise aus einem Baum mit dem Attribut _natural=tree_ die Klasse _natural-tree_. Diese Klassen werden vom Shared Guide Dog 4.0 genutzt, um die Objekte zu klassifizieren. Zusätzlich wird über CSS Styles die Darstellung im Editor gesetzt. Dabei handelt es sich lediglich um eine graphische Hilfe, die Styles werden nicht vom Shared Guide Dog 4.0 verwendet.

![SVG export format](/doc/svg_export.png)

Zusätzlich zur Bilddatei wird eine yaml-Datei erstellt, die weitere Daten über das Bild enthält. Diese Daten werden vom Map Server des SGD4.0 verwendet.

# Generierung und Export eines Wegenetzes


# Known limitations and bugs
