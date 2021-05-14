/*
 * File:   main.cpp
 * Author: Aike Banse
 *
 * Created on January 7, 2021, 11:47 AM
 *
 * Dieses Programm liest eine .osm-Datei ein und filtert diese. Die Datei wird zuerst nach dem Tag "SharedGuideDog" durchsucht und schreibt die gefilterten Inhalte
 * in eine zweite .osm-Datei. Nachfolgend werden in den Nodes und Ways mit dem Tag "SharedGuideDog" die Referenzen zu anderen Nodes rausgefiltert und ebenfalls in die
 * Zieldatei geschrieben. Nachdem nun vollständig gefiltert wurde muss die Datei navigierbar gemacht werden und wird dafür umgeschrieben. Folgend muss die erstellte 
 * Datei in eine A*-Funktion eingelesen werden und ein Pfad mit der niedrigsten Gesamtsumme erstellt werden, welcher wieder in einer Datei festgehalten wird.
*/

// Einbinden von bereits vorhandenen Bibliotheken
#include <string>
#include <fstream>
#include <iostream>
#include <vector>

#include "include/navigation/rapidxml-1.13/rapidxml.hpp"

// Einbinden von erzeugten Header-Dateien
#include "Filtern.hpp"
#include "way.h"
#include "aStern.h"

namespace sgd_nav
{

// Hauptprogramm
int main() {
    /*
        1. Nutzen der API zum Download von Daten -> später
        2. prefilter mit C++ -> später
        3. JOSM starten -> Daten augmentieren -> Datei speichern -> später
        4. Warten, bis JOSM geschlossen wird -> nochmal filtern
        5. Augmentierte Datei zu nav file
        5.1 for all nodes
            if action='delete' then continue
            import node
        5.2 for all way
            if action='delete' then continue
            add nd to node, copy tags
    */

    // Einbinden von benötigten Variablen

    int s = 0;
    float StartZielKoords[4] = { 0.0, 0.0, 0.0, 0.0 };

    // Einbinden der Dateipfade der Quell-, Ziel- und Navigationsdatei
    string ursprungsDatei = "C:\\Users\\Aike\\Desktop\\Uni\\Semester 7\\VSCodeProjekte\\SharedGuideDog\\01_Lohmuehlenpark_augmentiert_way.osm";
    string gefiltertDatei = "C:\\Users\\Aike\\Desktop\\Uni\\Semester 7\\VSCodeProjekte\\SharedGuideDog\\10_FilterErgebnis.osm";
    string navigationsfaehigeDatei = "C:\\Users\\Aike\\Desktop\\Uni\\Semester 7\\VSCodeProjekte\\SharedGuideDog\\20_NavigationsFaehigeDaten.osm";
    string endDateiOSM = "C:\\Users\\Aike\\Desktop\\Uni\\Semester 7\\VSCodeProjekte\\SharedGuideDog\\30_EndNavigationOSM.osm";
    string endDateiNavigation = "C:\\Users\\Aike\\Desktop\\Uni\\Semester 7\\VSCodeProjekte\\SharedGuideDog\\31_EndNavigation.osm";

    // Abfrage welche Programmteile aufgerufen werden sollen
    cout << "Bitte geben Sie fuer das komplette Ausfuehren der Datei eine 0 ein oder 1, um nur die Navigationsdatei zu erstellen:" << "\n";
    cin >> s;

    // Erstellen neuer Filterdatei und erster navigationsfähiger Datei
    if (s == 0) {
        filtern(ursprungsDatei, gefiltertDatei);
        navigationDatWay(gefiltertDatei, navigationsfaehigeDatei);
        s = 1;
    }

    // Durchlaufen der ersten navigationsfähigen Datei mit dem A*-Algorithmus und erstellen der geforderten Dateien
    if (s == 1) {

        // Abfrage der Start- und Zielkoordinaten
        cout << "Bitte geben Sie die Start und Ziel Koordinaten ein. Benutzen Sie bitte die folgende Reihenfolge: Startlaenge, Startbreite, Ziellaenge, Zielbreite" << "\n";
        for (int i = 0; i < 4; i++) {
            cin >> StartZielKoords[i];
        }

        aStern(navigationsfaehigeDatei, StartZielKoords, gefiltertDatei, endDateiNavigation, endDateiOSM);
    }

    osm_to_nav("..\\Karten\\2_augmentiert.osm");

    return 0;
}

int osm_to_nav(std::string osm_file)
{
    rapidxml::xml_document<> doc;
    
    std::ifstream file(osm_file);
    std::vector<char> buffer((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    buffer.push_back('\0');
    // Parse the buffer using the xml file parsing library into doc 
    doc.parse<0>(&buffer[0]);
    // Find our root node
    rapidxml::xml_node<> *root_node = doc.first_node("osm");

    for (rapidxml::xml_node<>* nd = root_node->first_node("node") ;
        nd; nd = nd->next_sibling("node"))
    {
        rapidxml::xml_attribute<>* attr;
        if (attr = nd->first_attribute("action"))
        {
            if (attr->value() == "delete") continue;
        }
        // parse and save node
        nd->value();
    }
    
    for (rapidxml::xml_node<>* nd = root_node->first_node("way");
        nd; nd = nd->next_sibling("way"))
    {
        rapidxml::xml_attribute<>* attr;
        if (attr = nd->first_attribute("action"))
        {
            if (attr->value() == "delete") continue;
        }
        // parse and save way
    }
    return 0;
}

}