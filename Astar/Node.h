#pragma once
#include <math.h>

using namespace std;

class Node
{
	long id;
	double lat, lon;
	list<pair<long, double>> successors;

public:
	//Node() = default; // default constructor -> remove
	Node(long id, double lat, double lon) {
		this->id = id;
		this->lat = lat;
		this->lon = lon;
	}

	long getID() {
		return this->id;
	}

	double getLat() {
		return this->lat;
	}
	double getLon() {
		return this->lon;
	}
	void addSuccessor(long ID, double g)	{
		this->successors.push_back(make_pair(ID, g));
	}
	list<pair<long, double>> getSuccessors() {
		return this->successors;
	}
	double distanceToPosition(Node pos) {
		return (double)sqrt(pow(this->lat - pos.lat, 2.0) + pow(this->lon - pos.lon, 2.0));
	}
};

