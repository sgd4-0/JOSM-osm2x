/*
 * File:   aStern.h
 * Author: Aike Banse
 *
 * Created on February 24, 2021, 10:54 AM
 */

#include <string>

using namespace std;

#ifndef ASTERN_H
#define ASTERN_H

// Initialisierung für Funktionen
void aStern(string navigationDat, float StartZielKoords[], string ursprungsDatei, string endDateiNavigation, string endDateiOSM);
void annaeherungKoords(string navigationDat, float StartZielKoords[], string startZielIDs[]);
void wegFinden(string startZielIDs[], string navigationDat, string ursprungsDatei, string endDateiOSM, string endDateiNavigation);
void erstellenNavigationsDatei(string navigationDat, string endDateiNavigation, string nodes[], int finalerWeg[]);
void erstellenOSMDatei(string ursprungsDatei, string endDateiOSM, string nodes[], int finalerWeg[]);

#endif 