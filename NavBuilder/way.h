/*
 * File:   way.h
 * Author: Aike Banse
 *
 * Created on January 28, 2021, 11:07 AM
 */

#include <string>

using namespace std;

#ifndef WAY_H
#define WAY_H

	// Initialisierung für Funktionen
	void navigationDatWay(string zielDat, string navigationDat);
	void nodesSuchen(string zielDat, string nodeArray[]);
	void nodesSchreiben(string zielDat, string navigationDat, string nodeArray[], int k);
	void nodeRefFinden(string zielDat, string nodeRefs[], string aktuelleNode);

#endif 