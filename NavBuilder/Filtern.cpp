/*
 * File:   filtern.cpp
 * Author: Aike Banse
 *
 * Created on January 7, 2021, 11:47 AM
 *
 * Dieses Programm liest eine .osm-Datei ein, sucht die wichtigen Passagen raus und schreibt diese in eine neue .osm-Datei. 
 * Nachdem alle Bestandteile übertragen wurden, wird nach Referenzen in diesen Bestandteilen geguckt und anhand dieser die 
 * dazugehörigen Bestandteile ebenfalls in die neue Datei übertragen.
*/

#include <fstream>
#include <string>

#include "Filtern.h"

using namespace std;

// Ruft die einzelnen filter auf, filtert die Quelldatei und schreibt die Zieldatei                                         
void filtern(string herkunftDat, string zielDat) {

    fstream f;

    // Deklarieren und initialisieren von für das Programm wichtiger Variablen
    const int refSize = 300;
    const int rahmenSize = 200;
    int rahmen[rahmenSize][2] = { 0 };
    int rahmenREF[rahmenSize][2] = { 0 };

    string refs[refSize] = {};
    string docEnde = "</osm>";

    f.open(zielDat, f.out | f.trunc);                                   // öffne die Zieldatei und lösche alles in dieser
    f.close();                                                          // schließe die Zieldatei

    // Führe die Funktionen aus und übergebe die benötigten Variablen
    SGDrahmenFinden(herkunftDat, rahmen);
    SGDSchreiben(herkunftDat, zielDat, rahmen);
    referenzSuchen(zielDat, refs);
    referenzAbgleich(herkunftDat, rahmen, rahmenREF, refs, refSize);
    referenzSchreiben(herkunftDat, rahmenREF, zielDat);

    f.open(zielDat, f.app);                                             // öffne und gehe zum letzten Punkt der Zieldatei
    f << docEnde;                                                       // schreibe das Dokumentende in die Zieldatei
    f.close();                                                          // schließe die Zieldatei
}

// Findet die zu dem Tag "SharedGuideDog" gehörenden Nodes und Ways in der Quelldatei und speichert die Zeilennummern in einem Array
void SGDrahmenFinden(string herkunftDat, int rahmen[][2]) {

    ifstream osmdatei(herkunftDat);     // öffnen der Quelldatei

    // Deklarieren und initialisieren von für das Programm wichtiger Variablen
    int SGDgefunden = 0;
    int zeilennr = 0;
    int anzahlSGD = 0;
    int anfang = 0;

    string zeile;
    string SGD = "SharedGuideDog";

    while (getline(osmdatei, zeile)) {                                                           // Läuft solange wie in der Quelldatei Zeilen sind, bewegt sich pro Aufruf von getline() eine Zeile weiter und speichert die aktuelle Zeile in "zeile"

        SGDgefunden = 0;                                                                            // setzt die Variable "SGDgefunden" auf 0
        zeilennr++;                                                                                 // setzt die Zeilenanzahl um 1 hoch

        if (zeile.find("<node") != string::npos || zeile.find("<way") != string::npos) {
            while (zeile.find("</node>") == string::npos && zeile.find("</way>") == string::npos) {       // Läuft solange bis in der aktuellen Zeile "</node>" oder "</way>" gefunden wird
                if (zeile.find("<node") != string::npos || zeile.find("<way") != string::npos) {              // wenn in der aktuellen Zeile "<node" oder "<way" steht dann
                    anfang = zeilennr;                                                                          // setze die Variable "anfang" gleich der Variable "zeilennr"
                }
                else if (zeile.find(SGD) != string::npos) {                                                        // Wenn in der aktuellen Zeile "SharedGuideDog" gefunden wird dann
                    SGDgefunden = 1;                                                                            // setzt die Variable "SGDgefunden" auf 1
                }
                getline(osmdatei, zeile);                                                                   // iteriert weiter durch die Quelldatei und gibt neue Zeilen zurück
                zeilennr++;                                                                                 // setzt die Zeilenanzahl um 1 hoch
            }
            if (SGDgefunden == 1) {                                                                       // wenn die Variable "SGDgefunden" = 1 ist dann
                rahmen[anzahlSGD][0] = anfang;                                                              // setze den Anfang des Rahmens auf die Zeilennummer bei der "<node" oder "<way" gefunden wurde,
                rahmen[anzahlSGD][1] = zeilennr;                                                            // setze das Ende des Rahmens auf die Zeilennummer bei der "</node>" oder "</way>" gefunden wurde,
                anzahlSGD++;                                                                                // und zähle die Anzahl die SGD nach oben    
            }
        }
    }
}

// Schreibt die gefundenen Nodes und Ways mit dem Tag "SharedGuideDog" in die Zieldatei
void SGDSchreiben(string herkunftDat, string zielDat, int rahmen[][2]) {

    fstream f;

    // Deklarieren und initialisieren von für das Programm wichtiger Variablen
    int zeilennr = 0;
    int SGDnr = 0;

    string zeile;

    ifstream osmdatei2(herkunftDat);    // öffnen der Quelldatei

    while (getline(osmdatei2, zeile)) {                      // Läuft solange wie in der Quelldatei Zeilen sind, bewegt sich pro Aufruf von getline() eine Zeile weiter und speichert die aktuelle Zeile in "zeile"
        zeilennr++;                                             // setzt die Zeilenanzahl um 1 hoch
        if (zeilennr == 1 || zeilennr == 2 || zeilennr == 3) {    // wenn die Zeilennummer = 1, 2 oder 3 entspricht dann
            f.open(zielDat, f.app);                                 // öffne und gehe zum letzten Punkt der Zieldatei
            f << zeile << "\n";                                     // schreibe die aktuelle Zeile in die Zieldatei und beende die Zeile
            f.close();                                              // schließe die Zieldatei
        }
        else if (rahmen[SGDnr][0] == zeilennr) {                  // sonst wenn der Anfang der gefundenen Nodes und Ways der aktuellen Zeilennummer entrspricht dann
            for (int i = zeilennr; i <= rahmen[SGDnr][1]; i++) {      // solange i das Ende des Rahmens nicht erreicht hat
                f.open(zielDat, f.app);                                 // öffne und gehe zum letzten Punkt der Zieldatei
                f << zeile << "\n";                                     // schreibe die aktuelle Zeile in die Zieldatei und beende die Zeile
                f.close();                                              // schließe die Zieldatei
                if (i == rahmen[SGDnr][1]) {                              // wenn i dem Rahmenende entpricht dann mache nix
                }
                else {                                                   // sonst
                    getline(osmdatei2, zeile);                              // gehe eine Zeile weiter
                    zeilennr++;                                             // und setze die zeilennummer hoch
                }
            }
            SGDnr++;                                                // setzt die aktuelle Zeile des rahmen-Arrays hoch
        }
    }
}

// Finden der zu den Referenzen gehörenden Nodes und Ways
void referenzSuchen(string zielDat, string* refs) {

    // Deklarieren und initialisieren von für das Programm wichtiger Variablen
    int refgefunden = 0;
    int anzahlREF = 0;

    string zeile;
    string aktuelleZeile;
    string id;
    string ref = "ref='";
    string zeilenEnde = "/>";
    string idEnde = "'";

    ifstream testErgebnis(zielDat);    // öffnen der Zieldatei

    while (getline(testErgebnis, zeile)) {                                               // Läuft solange wie in der Zieldatei Zeilen sind, bewegt sich pro Aufruf von getline() eine Zeile weiter und speichert die aktuelle Zeile in "zeile"
        aktuelleZeile = "";                                                                 // Setzt den String "aktuelleZeile" zurück
        if (zeile.find(ref) != string::npos) {                                                // wenn in der aktuellen zeile der String "ref" gefunden wird dann
            refgefunden = 1;                                                                    // Variable gleich 1 setzen
            aktuelleZeile = zeile;                                                              // Kopiert die Aktuelle Zeile in einen String
            for (int i = 0; i <= 100; i++) {                                                      // Solange i kleiner gleich 100 ist führe aus
                if (aktuelleZeile[i] == zeilenEnde[0] && aktuelleZeile[i + 1] == zeilenEnde[1]) {      // wenn das zeilenende der aktuellen zeile zu erkennen ist dann
                    break;                                                                              // springe aus der For-Schleife
                }                                                                                   // sonst wenn die Textpassage aus der aktuellen Zeile = "ref" ist dann
                else if (aktuelleZeile[i] == ref[0] && aktuelleZeile[i + 1] == ref[1] && aktuelleZeile[i + 2] == ref[2] && aktuelleZeile[i + 3] == ref[3] && aktuelleZeile[i + 4] == ref[4]) {
                    i = i + 5;                                                                          // Zähle i + 5
                    int b = 0;                                                                          // Initialisieren und deklarieren der Variable "b"
                    while (aktuelleZeile[i] != idEnde[0]) {                                               // Läuft solange die aktuelle Zeile an der Stelle i nicht dem Ende der ID entspricht
                        i++;                                                                                // Zähle i hoch
                        b++;                                                                                // Zähle b hoch
                    }
                    i = i - b;                                                                          // Setze i um b zurück
                    id.resize(b);                                                                       // Erzeugt eine neue Größe des Strings "id" mit der Größe "b"
                    int a = 0;                                                                          // Initialisieren und deklarieren der Variable "a"
                    while (aktuelleZeile[i] != idEnde[0]) {                                               // Läuft solange die aktuelle Zeile an der Stelle i nicht dem Ende der ID entspricht
                        id[a] = aktuelleZeile[i];                                                           // Speichert die aktuelle ID-Stelle in den Sring "id"
                        i++;                                                                                // Zähle i hoch
                        a++;                                                                                // Zähle a hoch
                    }
                }
            }
            for (int i = 0; i < anzahlREF; i++) {                                                 // Solange die Laufvariable "i" kleiner als die Anzahl der Referenzen ist
                if (refs[i] == id) {                                                                  // wenn die gefunden ID der aktuellen Zeile des Referenzen-Array entspricht
                    refgefunden = 0;                                                                    // Setze Variable = 0
                    break;                                                                              // Spring aus der Schleife
                }
            }
            if (refgefunden == 1) {                                                               // wenn die Variable = 1 ist dann
                refs[anzahlREF] = id;                                                               // speicher die neue ID im Referenzen-Array
                anzahlREF++;                                                                        // Zähle die Zeilen hoch
            }
        }
    }
}

// Überprüft ob die gefundenen Referenzen schon in der Zieldatei sind
void referenzAbgleich(string herkunftDat, int rahmen[][2], int rahmenREF[][2], string* refs, int refSize) {

    // Deklarieren und initialisieren von für das Programm wichtiger Variablen
    int zeilennr = 0;
    int anfang = 0;
    int refgefunden = 0;
    int anzahlREF = 0;
    int o = 0;

    string zeile;

    ifstream osmdatei3(herkunftDat);    // öffnen der Quelldatei

    while (getline(osmdatei3, zeile)) {                                                                                                      // Läuft solange wie in der Quelldatei Zeilen sind, bewegt sich pro Aufruf von getline() eine Zeile weiter und speichert die aktuelle Zeile in "zeile"

        zeilennr++;                                                                                                                             // Zählt die Zeilennummer hoch

        if (zeile.find("<node") != string::npos || zeile.find("<way") != string::npos || zeile.find("<relation") != string::npos) {               // wenn in der aktuellen Zeile "<node", "<way" oder "<relation" gefunden wird dann
            if (zeilennr == rahmen[o][0]) {                                                                                                           // wenn die aktuelle Zeilennummer dem aktuellen Rahmenanfang entspricht dann
                for (int i = 0; i < rahmen[o][1] - rahmen[o][0]; i++) {                                                                                   // solange i kleiner der rahmengröße ist 
                    getline(osmdatei3, zeile);                                                                                                              // gehe eine Zeile weiter 
                    zeilennr++;                                                                                                                             // und setze die Zeilennummer hoch
                }
                o++;                                                                                                                                    // Zähle "o" hoch
            }
            else {                                                                                                                                  // sonst
                anfang = zeilennr;                                                                                                                      // Speicher die aktuelle Zeilennummer
                while (zeile.find("</node>") == string::npos && zeile.find("</way>") == string::npos && zeile.find("</relation>") == string::npos) {      // Solange das Ende von eines Nodes, eines Ways oder einer Relation nicht gefunden wurde
                    if (zeile.find("<node") != string::npos || zeile.find("<way") != string::npos || zeile.find("<relation") != string::npos) {               // wenn der Anfang einer Node, einem Way oder einer relation gefunden wurde dann
                        int i = 0;                                                                                                                              // initialisiere und deklariere eine Laufvariable
                        while (1) {                                                                                                                               // unendlichschleife
                            if (refs[i] == "" || refs[i] == refs[refSize]) {                                                                                          // wenn keine referenzen mehr im Array stehen dann
                                break;                                                                                                                                  // spring aus der unendlichschleife
                            }
                            else if (zeile.find(refs[i]) != string::npos) {                                                                                          // sonst wenn in der aktuellen Zeile die Referenz übereinstimmt 
                                refgefunden = 1;                                                                                                                        // setze die Variable auf 1
                                break;                                                                                                                                  // spring aus der unendlichschleife
                            }
                            i++;                                                                                                                                    // Zähle i hoch
                        }
                        if (zeile.find("' />") != string::npos) {                                                                                               // wenn in der aktuellen Zeile der String zu finden ist dann
                            break;                                                                                                                                  // Spring aus der unterliegenden While-Schleife
                        }
                    }
                    getline(osmdatei3, zeile);                                                                                                              // gehe eine Zeile weiter
                    zeilennr++;                                                                                                                             // und setze die Zeilennummer hoch
                }
            }
            if (refgefunden == 1) {                                                                                                                   // wenn eine Referenz gefunden wurde dann
                rahmenREF[anzahlREF][0] = anfang;                                                                                                       // Speichert den anfang der Referenz
                rahmenREF[anzahlREF][1] = zeilennr;                                                                                                     // Speichert das Ende der Referenz
                anzahlREF++;                                                                                                                            // Zählt die Variable hoch
            }
        }
        refgefunden = 0;                                                                                                                        // setzt die Variable auf 0
    }
}

// Schreibt die Referenzen in die Zieldatei
void referenzSchreiben(string herkunftDat, int rahmenREF[][2], string zielDat) {

    fstream f;

    // Deklarieren und initialisieren von für das Programm wichtiger Variablen
    int zeilennr = 0;
    int SGDnr = 0;

    string zeile;

    ifstream osmdatei4(herkunftDat);    // öffnen der Quelldatei

    while (getline(osmdatei4, zeile)) {                      // Läuft solange wie in der Quelldatei Zeilen sind, bewegt sich pro Aufruf von getline() eine Zeile weiter und speichert die aktuelle Zeile in "zeile"
        zeilennr++;                                             // Zählt die Variable hoch
        if (rahmenREF[SGDnr][0] == zeilennr) {                    // wenn der Anfang des referenzrahmens der Zeilennummer entspricht dann
            for (int i = zeilennr; i <= rahmenREF[SGDnr][1]; i++) {   // solange i kleiner des Endes der referenzrahmens ist
                f.open(zielDat, f.app);                                 // öffne und gehe zum letzten Punkt der Zieldatei
                f << zeile << "\n";                                     // schreibe die aktuelle Zeile in die Zieldatei und beende die Zeile
                f.close();                                              // schließe die Zieldatei                                             
                if (i == rahmenREF[SGDnr][1]) {                           // wenn i der Zeilennummer des Endes des Referenznahmens entspricht mach nix
                }
                else {                                                   // sonst
                    getline(osmdatei4, zeile);                              // gehe eine Zeile weiter 
                    zeilennr++;                                             // und setze die zeilennummer hoch 
                }
            }
            SGDnr++;                                            // setzt die aktuelle Zeile des rahmen-Arrays hoch 
        }
    }
}