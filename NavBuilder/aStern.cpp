/*
 * File:   aStern.cpp
 * Author: Aike Banse
 *
 * Created on February 24, 2021, 11:00 AM
 * 
 * Ändert die dem Programm gegebenen Koordinaten in die Koordinaten des am nahe liegensten Nodes an und führt den A*-Algorithmus durch. 
 * Speichert das Ergebnis aus dem A*-Algorithmus in 2 Dateien. Einer navigationsfähigen und einer durch OSM lesbaren Datei.
 */

#include <fstream>
#include <string>
#include <math.h>

#include "aStern.h"

using namespace std;

// Ruft die Funktionen zum Annähern und für den A*-Algorithmus auf
void aStern(string navigationDat, float StartZielKoords[], string ursprungsDatei, string endDateiNavigation, string endDateiOSM) {
	
	string startZielIDs[2] = {};

	annaeherungKoords(navigationDat, StartZielKoords, startZielIDs);							// Aufruf der Annäherungsfunktion
	wegFinden(startZielIDs, navigationDat, ursprungsDatei, endDateiOSM, endDateiNavigation);	// Aufruf der A*-Funktion mit schreiben der beiden Dateien
}

// Nähert die gegebenen Koordinaten den Koordinaten des nächstliegenden Nodes an
void annaeherungKoords(string navigationDat, float StartZielKoords[], string startZielIDs[]) {

	ifstream navDat(navigationDat);

	double minDistant[2] = { 1000.0, 1000.0 };

	string zeile;
	string idlatlon = "id='lat='lon='";
	string zeilenEnde = "' ";

	while (getline(navDat, zeile)) {					// Läuft solange Zeilen in der Quelldatei sind
		if (zeile.find("<node ") != string::npos) {			// wenn "<node " gefunden wurde dann
			string lon;
			string lat;
			string id;
			for (int i = 6; i <= zeile.length(); i++) {			// sucht die ID, den lat-Wert und den lon-Wert der Node raus
				int k = 0;
				// Sucht und speichert die ID der Node
				if (zeile[i] == idlatlon[0] && zeile[i + 1] == idlatlon[1] && zeile[i + 2] == idlatlon[2] && zeile[i + 3] == idlatlon[3]) {	
					i = i + 4;
					for (int o = i; 1 == 1; o++) {		// suchen des Anfangs der ID
						if (zeile[o] == zeilenEnde[0]) {
							k = o - i;
							break;
						}
					}
					id.resize(k);						// Größe des strings anpassen
					for (int o = 0; o < k; o++) {		// speichern der ID
						id[o] = zeile[i];
						i++;
					}
				}
				// sucht und speichert den lat-Wert der Node
				else if (zeile[i] == idlatlon[4] && zeile[i + 1] == idlatlon[5] && zeile[i + 2] == idlatlon[6] && zeile[i + 3] == idlatlon[7] && zeile[i + 4] == idlatlon[8]) {	
					i = i + 5;
					for (int o = i; 1 == 1; o++) {		// suchen des Anfangs des lat-Wertes
						if (zeile[o] == zeilenEnde[0]) {
							k = o - i;
							break;
						}
					}
					lat.resize(k);						// Größe des strings anpassen
					for (int o = 0; o < k; o++) {		// speichern des lat-Wertes
						lat[o] = zeile[i];
						i++;
					}
				}
				// sucht und speichert den lon-Wert der Node
				else if (zeile[i] == idlatlon[9] && zeile[i + 1] == idlatlon[10] && zeile[i + 2] == idlatlon[11] && zeile[i + 3] == idlatlon[12] && zeile[i + 4] == idlatlon[13]) {
					i = i + 5;
					for (int o = i; 1 == 1; o++) {		// suchen des Anfangs des lon-Wertes
						if (zeile[o] == zeilenEnde[0]) {
							k = o - i;
							break;
						}
					}
					lon.resize(k);						// Größe des strings anpassen
					for (int o = 0; o < k; o++) {		// speichern des lon-Wertes
						lon[o] = zeile[i];
						i++;
					}
					break;
				}
			}
			float latKoord = stof(lat);		// Ändern des lat-Strings in einen float-Wert
			float lonKoord = stof(lon);		// Ändern des lon-Strings in einen float-Wert
			double distanz = sqrt(pow(StartZielKoords[0] - latKoord, 2.0) + pow(StartZielKoords[1] - lonKoord, 2.0));		// ermitteln der Distanz durch Satz des Pythagoras der Startkoordinaten
			if (distanz < minDistant[0]) {	// prüfen ob die erzeugte Distanz kleiner ist als die bisher ermittelte kleinste Distanz und gegebenenfall speichern
				minDistant[0] = distanz;
				startZielIDs[0] = id;
			}
			distanz = sqrt(pow(StartZielKoords[2] - latKoord, 2.0) + pow(StartZielKoords[3] - lonKoord, 2.0));				// ermitteln der Distanz durch Satz des Pythagoras der Zielkoordinaten
			if (distanz < minDistant[1]) {	// prüfen ob die erzeugte Distanz kleiner ist als die bisher ermittelte kleinste Distanz und gegebenenfall speichern
				minDistant[1] = distanz;
				startZielIDs[1] = id;
			}
		}
	}
}

// Durchführung des A*-Algorithmus und schreiben der beiden Dateien
void wegFinden(string startZielIDs[], string navigationDat, string ursprungsDatei, string endDateiOSM, string endDateiNavigation) {

	ifstream navDat(navigationDat);

	const int zeilenAnzahl = 100;
	int NodeListenNr = 0;

	string zeile;
	string nodes[zeilenAnzahl];
	string anfangDaten = "='";

	int verknuepft[zeilenAnzahl][4];
	float latlonWerte[zeilenAnzahl][2] = { 0.0 };

	// setzt alle Werte im verknuepft-Array auf 1000
	for (int y = 0; y < zeilenAnzahl; y++) {
		for (int w = 0; w < 4; w++) {
			verknuepft[y][w] = 1000;
		}
	}

	// Filtert die ID, die lat-Werte und lon-Werte aus den Nodes und wandelt die lat- und lon-Werte in floats
	while (getline(navDat, zeile)) {
		if (zeile.find("<node ") != string::npos) {		// wenn "<node " in der zeile gefunden wird

			int k = 0;
			
			string node = "              ";
			string latDat = "              ";
			string lonDat = "              ";

			for (int i = 0; i <= zeile.length(); i++) {		// durchläuft die Zeile
				if (zeile[i] == anfangDaten[0] && zeile[i + 1] == anfangDaten[1]) {	// prüft ob nachfolgend eine ID, lat oder lon kommt
					i = i + 2;
					int a = 0;
					for (int o = 0; zeile[i] != anfangDaten[1]; o++) {
						if (k == 0) {			// speichert die node-ID
							node[o] = zeile[i];
						}
						else if (k == 1) {		// speichert den node-lat-wert
							latDat[o] = zeile[i];
						}
						else if (k == 2) {		// speichert den node-lon-wert
							lonDat[o] = zeile[i];
						}
						a = o+1;
						i++;
					}
					if (k == 0) {		// passt die Größe des Strings an
						node.resize(a);
					}
					else if (k == 1) {	// passt die Größe des Strings an
						latDat.resize(a);
					}
					else if (k == 2) {	// passt die Größe des Strings an
						lonDat.resize(a);
					}
					k++;
					if (k == 3) {		// wenn k 3 erreicht springe aus der for-Schleife
						break;
					}
				}
			}
			nodes[NodeListenNr] = node;						// Speichern der ID
			latlonWerte[NodeListenNr][0] = stof(latDat);	// Speichern des lat-Werts als float
			latlonWerte[NodeListenNr][1] = stof(lonDat);	// Speichern des lon-Werts als float
		}
		else if (zeile.find("</node>") != string::npos){	// wenn das Ende eines Nodes detektiert wird setze die nodelistennummer hoch
			NodeListenNr++;
		}
	}

	ifstream navDat1(navigationDat);

	int y = 0;
	int referenzNr = 0;
	
	// Speichert die Referenz-ID´s der einzelnen Nodes in ein Array
	while (getline(navDat1, zeile)) {
		if (zeile.find("<nd ") != string::npos) {		// wenn eine Referenz gefunden wurde

			string referenz = "             ";

			for (int i = 0; i <= zeile.length(); i++) {			// Durchsucht die Zeile nach der ID
				if (zeile[i] == anfangDaten[0] && zeile[i + 1] == anfangDaten[1]) {
					i = i + 2;
					int u = 0;
					for (int o = 0; zeile[i] != anfangDaten[1]; o++) {	// Schreibt die ID in das Referenz-Array
						referenz[o] = zeile[i];
						i++;
						u++;
					}
					referenz.resize(u);				// Passt die Größe des Strings an
					break;
				}
			}
			int r = 0;
			while (true) {
				if (nodes[r] == referenz) {		// sucht die Zeile im Nodes-Array welche der referenz entspricht
					break;
				}
				r++;
			}
			verknuepft[y][referenzNr] = r;	// speichert die Zeilennummer der Referenz aus dem Nodes-Array
			referenzNr++;
		}
		else if (zeile.find("</node>") != string::npos) {	// wenn das Nodeende detektiert wird setze y hoch und die referenznr = 0
			y++;
			referenzNr = 0;
		}
	}

	int nochoffen[zeilenAnzahl][2];
	float nochoffenDistanz[zeilenAnzahl];
	int gecheckt[zeilenAnzahl][2];
	float gechecktDistanz[zeilenAnzahl];
	int nodePosition = 0;
	int anfang = 0;
	int ende = 0;
	int finalerWeg[zeilenAnzahl] = {};

	// Setzt die Werte in den aufgerufenen Arrays
	for (int y = 0; y < zeilenAnzahl; y++) {
		for (int w = 0; w < 2; w++) {
			gecheckt[y][w] = -1;
			nochoffen[y][w] = -1;
		}
		gechecktDistanz[y] = 1000;
		nochoffenDistanz[y] = 1000;
		finalerWeg[y] = -1;
	}

	// Speichern des Starts des A*-Algorithmus
	for (int i = 0; i < zeilenAnzahl; i++) {
		if (nodes[i] == startZielIDs[0]) {
			nochoffen[0][0] = i;
			nochoffen[0][1] = -1;
			nochoffenDistanz[0] = 0.0;
			anfang = i;
			break;
		}
	}

	// Speichern des Ziels des A*-Algorithmus
	for (int i = 0; i < zeilenAnzahl; i++) {
		if (nodes[i] == startZielIDs[1]) {
			ende = i;
			break;
		}
	}

	// Durchführung des A*-Algorithmus
	while (true) {
		
		float minDistanz = 1000.0;
		int q = 0;

		// entscheiden welcher Node die kürzeste Distanz hat und als nächstes dran ist
		for (int p = 0; p < zeilenAnzahl; p++) {
			if (nochoffenDistanz[p] < minDistanz) {
				minDistanz = nochoffenDistanz[p];
				nodePosition = nochoffen[p][0];
				q = p;
			}
		}

		// prüfen ob das Ziel erreicht ist und gegebenenfalls den Weg speichern
		if (nochoffen[q][0] == ende){
			int z = 0;
			while (true) {
				if (gecheckt[z][0] == -1) {
					gecheckt[z][0] = nochoffen[q][0];
					gecheckt[z][1] = nochoffen[q][1];
					gechecktDistanz[z] = nochoffenDistanz[q];
					break;
				}
				z++;
			}
			finalerWeg[0] = gecheckt[z][0];
			for (int s = 1; finalerWeg[s - 1] != anfang; s++) {
				int h = 0;
				while (true) {
					if (gecheckt[z][1] == gecheckt[h][0]) {
						finalerWeg[s] = gecheckt[h][0];
						z = h;
						break;
					}
					h++;
				}
			}
			break;
		}

		int k = 0;
		int enthalten = 0;

		// prüfen ob das zu übertragende Objekt aus der nochoffen-Liste bereits in der gecheckt-Liste vorkommt und gegebenenfalls den Zeiger und Distanz ändern
		while (gecheckt[k][0] >= 0) {
			if (gecheckt[k][0] == nochoffen[q][0]) {
				if (gechecktDistanz[k] > minDistanz) {
					gecheckt[k][1] = nochoffen[q][1];
					gechecktDistanz[k] = minDistanz;
				}
				nochoffen[q][0] = -1;
				nochoffen[q][1] = -1;
				nochoffenDistanz[q] = 1000.0;
				enthalten = 1;
				break;
			}
			k++;
		}

		// wenn nicht enthalten ist dann neu reinschreiben
		if (enthalten == 0) {
			gecheckt[k][0] = nochoffen[q][0];
			gecheckt[k][1] = nochoffen[q][1];
			gechecktDistanz[k] = nochoffenDistanz[q];
			nochoffen[q][0] = -1;
			nochoffen[q][1] = -1;
			nochoffenDistanz[q] = 1000.0;
		}

		// prüfen ob die Node-Kombination der nächsten Nodes in der nochoffen-Liste oder in der gecheckt-Liste bereits notiert sind, wenn das der Fall ist wird nix neues eingetragen
		for (int i = 0; verknuepft[nodePosition][i] != 1000; i++) {
			
			int l = 0;
			int vorhanden = 0;

			for (int r = 0; r < zeilenAnzahl; r++) {
				if ((verknuepft[nodePosition][i] == gecheckt[r][0] && nodePosition == gecheckt[r][1]) || (verknuepft[nodePosition][i] == gecheckt[r][1] && nodePosition == gecheckt[r][0])) {
					vorhanden = 1;
					break;
				}
			}

			for (int r = 0; r < zeilenAnzahl; r++) {
				if ((verknuepft[nodePosition][i] == nochoffen[r][0] && nodePosition == nochoffen[r][1]) || (verknuepft[nodePosition][i] == nochoffen[r][1] && nodePosition == nochoffen[r][0])) {
					nochoffen[r][0] = -1;
					nochoffen[r][1] = -1;
					nochoffenDistanz[r] = 1000.0;
					vorhanden = 1;
					break;
				}
			}
			
			while (vorhanden == 0) {
				if (nochoffen[l][0] == -1) {
					nochoffen[l][0] = verknuepft[nodePosition][i];
					nochoffen[l][1] = nodePosition;
					nochoffenDistanz[l] = gechecktDistanz[k] + sqrt(pow(latlonWerte[nodePosition][0] - latlonWerte[verknuepft[nodePosition][i]][0], 2.0) + pow(latlonWerte[nodePosition][1] - latlonWerte[verknuepft[nodePosition][i]][1], 2.0));
					break;
				}
				l++;
			}
		}
	}

	// Aufruf der Funktionen zum Erstellen der gesuchten navigationsfähigen und durch OSM lesbaren Datei
	erstellenNavigationsDatei(navigationDat, endDateiNavigation, nodes, finalerWeg);
	erstellenOSMDatei(ursprungsDatei, endDateiOSM, nodes, finalerWeg);
}

// erstellen der geforderten navigationsfähigen Datei
void erstellenNavigationsDatei(string navigationDat, string endDateiNavigation, string nodes[], int finalerWeg[]) {
	
	fstream f;

	f.open(endDateiNavigation, f.out | f.trunc);                        // öffne die Zieldatei und lösche alles in dieser
	f.close();                                                          // schließe die Zieldatei

	ifstream sourceDat(navigationDat);

	string zeile;
	int zeilennr = 0;

	while (getline(sourceDat, zeile)) {                      // Läuft solange wie in der Quelldatei Zeilen sind, bewegt sich pro Aufruf von getline() eine Zeile weiter und speichert die aktuelle Zeile in "zeile"
		if (zeilennr == 0) {									// wenn die Zeilennummer = 0 entspricht dann
			f.open(endDateiNavigation, f.app);                      // öffne und gehe zum letzten Punkt der Zieldatei
			f << zeile << "\n";                                     // schreibe die aktuelle Zeile in die Zieldatei und beende die Zeile
			f.close();                                              // schließe die Zieldatei
			zeilennr++;                                             // setzt die Zeilenanzahl um 1 hoch
		}
		else if (zeile.find("<node ") != string::npos) {     // sonst wenn "<node " gefunden wird dann
			for (int i = 0; finalerWeg[i] != -1; i++) {			// solange an der i-ten Stelle der Variable nicht "-1" steht
				if (zeile.find(nodes[finalerWeg[i]]) != string::npos) {
					while (zeile.find("</node>") == string::npos) {
						if (zeile.find("<node ") != string::npos) {
							f.open(endDateiNavigation, f.app);                                 // öffne und gehe zum letzten Punkt der Zieldatei
							f << zeile << "\n";                                     // schreibe die aktuelle Zeile in die Zieldatei und beende die Zeile
							f.close();                                              // schließe die Zieldatei
						}
						else if (zeile.find("<nd ") != string::npos) {
							for (int k = 0; finalerWeg[k] != -1; k++) {      // solange k das Ende des Rahmens nicht erreicht hat
								if (zeile.find(nodes[finalerWeg[k]]) != string::npos) {
									f.open(endDateiNavigation, f.app);                                 // öffne und gehe zum letzten Punkt der Zieldatei
									f << zeile << "\n";                                     // schreibe die aktuelle Zeile in die Zieldatei und beende die Zeile
									f.close();                                              // schließe die Zieldatei
								}
							}
						}
						else {
							f.open(endDateiNavigation, f.app);                      // öffne und gehe zum letzten Punkt der Zieldatei
							f << zeile << "\n";                                     // schreibe die aktuelle Zeile in die Zieldatei und beende die Zeile
							f.close();                                              // schließe die Zieldatei
						}
						getline(sourceDat, zeile);                              // gehe eine Zeile weiter
					}
					f.open(endDateiNavigation, f.app);                                 // öffne und gehe zum letzten Punkt der Zieldatei
					f << zeile << "\n";                                     // schreibe die aktuelle Zeile in die Zieldatei und beende die Zeile
					f.close();                                              // schließe die 
				}
			}
		}
	}
	f.open(endDateiNavigation, f.app);                                             // öffne und gehe zum letzten Punkt der Zieldatei
	f << "</nodelist>";                                                       // schreibe das Dokumentende in die Zieldatei
	f.close();                                                          // schließe die Zieldatei
}

// erstellen der OSM-Datei zur Visualisierung des Ergebnisses
void erstellenOSMDatei(string ursprungsDatei, string endDateiOSM, string nodes[], int finalerWeg[]) {

	fstream f;

	f.open(endDateiOSM, f.out | f.trunc);                               // öffne die Zieldatei und lösche alles in dieser
	f.close();                                                          // schließe die Zieldatei

	ifstream sourceDat(ursprungsDatei);

	string zeile;

	int zeilennr = 0;

	while (getline(sourceDat, zeile)) {                      // Läuft solange wie in der Quelldatei Zeilen sind, bewegt sich pro Aufruf von getline() eine Zeile weiter und speichert die aktuelle Zeile in "zeile"
		if (zeilennr == 0 || zeilennr == 1 || zeilennr == 2) {    // wenn die Zeilennummer = 1, 2 oder 3 entspricht dann
			f.open(endDateiOSM, f.app);                                 // öffne und gehe zum letzten Punkt der Zieldatei
			f << zeile << "\n";                                     // schreibe die aktuelle Zeile in die Zieldatei und beende die Zeile
			f.close();                                              // schließe die Zieldatei
			zeilennr++;                                             // setzt die Zeilenanzahl um 1 hoch
		}
		else if (zeile.find("<node ") != string::npos) {                  // sonst wenn der Anfang der gefundenen Nodes und Ways der aktuellen Zeilennummer entrspricht dann
			for (int i = 0; finalerWeg[i] != -1; i++) {      // solange i das Ende des Rahmens nicht erreicht hat
				if (zeile.find(nodes[finalerWeg[i]]) != string::npos) {
					if (zeile.find("<node ") != string::npos && zeile.find(" />") != string::npos) {
						f.open(endDateiOSM, f.app);                                 // öffne und gehe zum letzten Punkt der Zieldatei
						f << zeile << "\n";                                     // schreibe die aktuelle Zeile in die Zieldatei und beende die Zeile
						f.close();                                              // schließe die Zieldatei
						break;
					}
					else {
						while (zeile.find("</node>") == string::npos) {
							f.open(endDateiOSM, f.app);                                 // öffne und gehe zum letzten Punkt der Zieldatei
							f << zeile << "\n";                                     // schreibe die aktuelle Zeile in die Zieldatei und beende die Zeile
							f.close();                                              // schließe die Zieldatei
							getline(sourceDat, zeile);                              // gehe eine Zeile weiter
						}
						f.open(endDateiOSM, f.app);                                 // öffne und gehe zum letzten Punkt der Zieldatei
						f << zeile << "\n";                                     // schreibe die aktuelle Zeile in die Zieldatei und beende die Zeile
						f.close();                                              // schließe die Zieldatei
					}
				}
			}
		}
	}
	f.open(endDateiOSM, f.app);                                             // öffne und gehe zum letzten Punkt der Zieldatei
	f << "  <way id='10000000' action='modify' timestamp='2020-12-13T11:31:12Z' uid='10000000' user='1' visible='true' version='2' changeset='1'>" << "\n";   // schreibe einen Way in die Zieldatei
	f.close();                                                          // schließe die Zieldatei

	int k = 0;

	while (finalerWeg[k] != -1) {
		f.open(endDateiOSM, f.app);                                         // öffne und gehe zum letzten Punkt der Zieldatei
		f << "    <nd ref='" << nodes[finalerWeg[k]] << "' />" << "\n";     // schreibe Nodereferenzen in die Zieldatei
		f.close();                                                          // schließe die Zieldatei
		k++;
	}

	f.open(endDateiOSM, f.app);                                         // öffne und gehe zum letzten Punkt der Zieldatei
	f << "  </way>" << "\n";                                            // schreibe das Dokumentende in die Zieldatei
	f.close();                                                          // schließe die Zieldatei

	f.open(endDateiOSM, f.app);                                         // öffne und gehe zum letzten Punkt der Zieldatei
	f << "</osm>";                                                      // schreibe das Dokumentende in die Zieldatei
	f.close();                                                          // schließe die Zieldatei
}