#pragma once
#include "Node.h"
using namespace std;

class Way
{
	int id;
	Node start, end;
	double weight;

public:
	Way(Node start, Node end) {
		this->id = start.getID();
		this->start = start;
		this->end = end;
		this->weight = start.distanceToPosition(end);
	};
public:
	int getID() {
		return this->id;
	};
	double getWeight() {
		return this->weight;
	};
	Node getStart() {
		return this->start;
	};
	Node getEnd() {
		return this->end;
	};
};
