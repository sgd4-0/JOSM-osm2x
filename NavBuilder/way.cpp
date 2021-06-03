/*
 * File:   way.cpp
 * Author: Aike Banse
 *
 * Created on January 28, 2021, 11:07 AM
 * 
 * Erstellt eine navigationsf�hige Datei in der Nodes mit den n�chstliegenden verbundenen Nodes und deren Tags aufgelistet werden und pr�ft ob Referenzen doppelt sind.
 */

#include <fstream>
#include <string>

#include "way.h"

using namespace std;

// Leert die Navigationsdatei, schreibt Datei Anfang und Ende und ruft Funktionen auf
void navigationDatWay(string zielDat, string navigationDat) {
	
	fstream f;

	// Deklarieren und initialisieren von f�r das Programm wichtiger Variablen
	string nodeArray[100]	= {};
	string docAnfang		= "<nodelist id='1'>";
	string docEnde			= "</nodelist>";

	// Leeren der Navigationsdatei
	f.open(navigationDat, f.out | f.trunc);
	f.close();

	// Schreibt den Anfang in die Navigationsdatei
	f.open(navigationDat, f.app);
	f << docAnfang << "\n";
	f.close();

	// Aufruf der Funktionen
	nodesSuchen(zielDat, nodeArray);						// Sucht die Nodes und speichert diese in einem Array
	for (int k = 0; nodeArray[k] != ""; k++) {				// Ruft die Funktion auf bis nix mehr im Nodearray steht
		nodesSchreiben(zielDat, navigationDat, nodeArray, k);	// Schreibt die Nodes in die Zieldatei
	}

	// Schreibt das Ende in die Navigationsdatei
	f.open(navigationDat, f.app);
	f << docEnde;
	f.close();
}

// Sucht die Nodes und speichert diese in einem Array
void nodesSuchen(string zielDat, string nodeArray[]) {

	ifstream sourceDat(zielDat);

	// Deklarieren und initialisieren von f�r das Programm wichtiger Variablen
	int nodenr		= 0;
	int idAnfang	= 12;
	int idEnde		= 0;

	string zeile;
	string ID			= "";
	string idBegrenzung = "'";

	while (getline(sourceDat, zeile)) {				// Wird ausgef�hrt solange Zeilen in der gefilterten Datei sind
		if (zeile.find("<node") != string::npos) {		// Trifft zu wenn in der aktuellen Zeile "<node" steht
			idEnde = idAnfang;								// Speichert den Wert von "idAnfang" in "idEnde"
			while (zeile[idEnde] != idBegrenzung[0]) {		// Wird ausgef�hrt solange er nicht die begrenzung der ID findet
				idEnde++;										// Z�hlt "idEnde" hoch
			}
			ID.resize(idEnde - idAnfang);					// Ver�ndert die Gr��e von ID auf die Gr��e der aktuellen ID
			for (int i = 0; i < idEnde - idAnfang; i++) {	// L�uft solange i der ID-L�nge entspricht
				ID[i] = zeile[i + idAnfang];					// Speichert jede Stelle der ID in den ID-String
			}
			nodeArray[nodenr] = ID;							// Speichert die ID im Nodearray
			nodenr++;										// Z�hlt die gefundenen ID�s hoch
		}
	}
}

// schreibt die Nodes in die Zieldatei, sucht die damit verkn�pften Nodes und schreibt diese in die Zieldatei
void nodesSchreiben(string zielDat, string navigationDat, string nodeArray[], int k) {

	fstream f;
	
	ifstream sourceDat(zielDat);
	
	// Deklarieren und initialisieren von f�r das Programm wichtiger Variablen
	int nodenr = 0;

	string zeile;
	string koords;
	string nodeRefs[10]		= {};
	string nodeAnfang		= "<node id='";
	string koordsAnfang		= "lat='";
	const char koordsEnde	= ' ';
	const char nodeEnde		= '>';

	
	while (getline(sourceDat, zeile)) {									// Durchl�uft die Datei Zeile f�r Zeile
		if (zeile.find("<node") != string::npos) {							// wenn in der Zeile "<node" gefunden wird
			if (zeile.find(nodeArray[k]) != string::npos) {						// wenn in der Zeile die aktuelle die in nodearray stehende ID gefunden wird
				for (int i = 0; i <= zeile.length(); i++) {							// solange i kleiner ist als die Anzahl der chars in der Zeile
					if (zeile[i] == koordsAnfang[0] && zeile[i + 1] == koordsAnfang[1] && zeile[i + 2] == koordsAnfang[2] && zeile[i + 3] == koordsAnfang[3] && zeile[i + 4] == koordsAnfang[4]) {
						int b = 22;
						for (int s = b; 1 == 1; s++) {									// sucht die Anf�nge in der zeile von der lat- und lon-Werte
							if (zeile[i + s] == koordsEnde || zeile[i + s] == nodeEnde) {
								b = s;
								break;
							}
						}
						koords.resize(b);												// passt die Gr��e des strings koords an
						for (int a = 0; a <= b; a++) {									// schreibt die lat und lon-Werte in den koords String
							koords[a] = zeile[i];
							i++;
						}
					}
				}
				f.open(navigationDat, f.app);										// Schreibt die neue Zeile in die Zieldatei mit ID, lat und lon
				f << "  " << nodeAnfang << nodeArray[k] << "' " << koords << nodeEnde << "\n";
				f.close();
				nodeRefFinden(zielDat, nodeRefs, nodeArray[k]);						// findet die Referenzen zu der aktuellen Node
				int b = 0;
				while (nodeRefs[b] != "") {											// Schreibt die gefundenen Referenzen in die Zieldatei
					f.open(navigationDat, f.app);
					f << nodeRefs[b] << "\n";
					f.close();
					b++;
				}
				getline(sourceDat, zeile);											// geht eine Zeile weiter
				while (zeile.find("<tag") != string::npos) {						// �bertr�gt gefundene Tags in die Zieldatei
					f.open(navigationDat, f.app);
					f << zeile << "\n";
					f.close();
					getline(sourceDat, zeile);
				}
				f.open(navigationDat, f.app);										// Schlie�t den aktuellen Node aufruf in der Zieldatei
				f << "  </node>" << "\n";
				f.close();
			}
		}
	}
}

// findet die mit dem aktuell behandelten Node verkn�pften Nodes
void nodeRefFinden(string zielDat, string nodeRefs[], string aktuelleNode) {

	ifstream sourceDat1(zielDat);

	// Deklarieren und initialisieren von f�r das Programm wichtiger Variablen
	int refnr = 0;
	int doppelt = 0;

	string aktuellezeile;
	string letzteZeile;
	string ref[2];

	while (getline(sourceDat1, aktuellezeile)) {				// solange Zeilen in der Quellldatei sind
		if (aktuellezeile.find("<way") != string::npos) {			// wenn "<way" gefunden wird in der Zeile
			getline(sourceDat1, aktuellezeile);							// zeile weiter gehen
			while (aktuellezeile.find("<nd ref='") != string::npos) {	// solange "<nd ref='" in der zeile gefunden wird
				letzteZeile = aktuellezeile;								// speichert die zeile
				getline(sourceDat1, aktuellezeile);							// zeile weiter gehen
				if (letzteZeile.find(aktuelleNode) != string::npos) {		// wenn in der gespeicherten Zeile die aktuell behandelte Node steht, speicher eine Referenz
					ref[0] = aktuellezeile;
					break;
				}
				else if (aktuellezeile.find(aktuelleNode) != string::npos) {// sonst wenn die aktuelle Zeile der aktuellen Node entspricht, speicher die letzte Zeile und die n�chste Zeile als Referenz
					ref[0] = letzteZeile;
					getline(sourceDat1, aktuellezeile);
					if (aktuellezeile.find("<nd ref='") != string::npos) {
						ref[1] = aktuellezeile;
					}
					break;
				}
			}
		}
		for (int o = 0; o < 2; o++) {								// pr�ft ob die gefundenen Referenzen doppelt vorkommen
			doppelt = 0;
			if (ref[o] != "") {
				for (int i = 0; i < 10; i++) {
					if (nodeRefs[i] == ref[o]) {
						doppelt = 1;
					}
				}
				if (doppelt != 1) {
					nodeRefs[refnr] = ref[o];
					refnr++;
				}
			}
		}
	}
}