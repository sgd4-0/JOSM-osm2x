/*
 * File:   filtern.h
 * Author: Aike Banse
 *
 * Created on January 7, 2021, 11:52 AM
 */

#ifndef FILTERN_H
#define FILTERN_H

#include <string>

using namespace std;

	// Initialisierung für Funktionen
	void SGDrahmenFinden(string herkunftDat, int rahmen[][2]);
	void SGDSchreiben(string herkunftDat, string zielDat, int rahmen[][2]);
	void referenzSuchen(string zielDat, string* refs);
	void referenzAbgleich(string herkunftDat, int rahmen[][2], int rahmenREF[][2], string* refs, int refSize);
	void referenzSchreiben(string herkunftDat, int rahmenREF[][2], string zielDat);
	void filtern(string herkunftDat, string zielDat);

#endif 