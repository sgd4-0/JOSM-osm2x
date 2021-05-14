// Astar.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programms.
//

#include <iostream>
#include <stack>
#include <set>
#include <unordered_map>
#include <string>

#include "Node.h"
#include <unordered_set>
#include <queue>

#include <iostream>
#include <fstream>

// A C++ program to implement A* algorithm
using namespace std;

//constexpr auto ROW = 9;
//constexpr auto COL = 10;

// Creaing a shortcut for int, int pair type
// openList pair <Node.ID, f value>
typedef pair<long, double> olPair;
// closedList pair <Node.ID, parentNode.ID>
typedef pair<long, long> clPair;
typedef pair<double, pair<int, int>> pPair;
typedef pair<int, pair<double, double>> node;
// typedef pair<int, double> way;

// A structure to hold the necessary paramters
struct olData
{
    // ROW and COLUMN index of its parents
    int id;
    int parent_id;
    double f, g, h;
};

class myComparator
{
public:
    int operator() (const olData& p1, const olData& p2)
    {
        return p1.f > p2.f;
    }
};

// import xml file
void importXml() {
    string inFilename = "C:\\Users\\Stahr\\Documents\\Blindenhund-4.0\\OSM Daten\\test.xml";

    ifstream infile;
    infile.open(inFilename, ios::in);
    string line;
    while (getline(infile, line))
    {


        cout << line;
    }
    infile.close();
}

// Check whether destination cell has been reached or not
bool isDestination(Node pos, Node dest)
{
    // Besser über Lat und Lon
    return (pos.getID() == dest.getID());
}

// Trace path from source to destination
void tracePath(unordered_map<long,long> closedList, Node dest)
{
    printf("\nThe path is");
    
    stack<int> path;
    Node pos = dest;
    path.push(pos.getID());

    int id, pid;
    id = dest.getID();
    while ((pid = closedList.at(id)) > 0)
    {
        path.push(pid);
        id = pid;
    }

    while (!path.empty())
    {
        int p = path.top();
        path.pop();
        printf("-> (%d) ", p);
    }
    return;
}

void aStarSearch(unordered_map<long, Node> graph, Node src, Node dest)
{
    
    // Destination is the same as source
    if (isDestination(src, dest))
    {
        printf("We are already at the destination\n");
        return;
    }
    unordered_map<long, long> closedList;
    priority_queue<olData, vector<olData>, myComparator> openList;

    // Initialising the parameters of the starting node
    long i = src.getID();
    olData d;
    d.id = src.getID();
    d.parent_id = -1;
    d.f = 0.0;
    d.g = 0.0;
    d.h = 0.0;
    openList.push(d);

    // We set this boolean value as false as initially 
    // the destination is not reached. 
    bool foundDest = false;

    while (!openList.empty())
    {
        // access element with smallest f value
        olData olistData = openList.top();
        Node pos = graph.at(olistData.id);

        // Remove this node from the open list 
        openList.pop();

        // Check if destination is reached
        if (isDestination(pos, dest))
        {
            closedList.insert(make_pair(olistData.id, olistData.parent_id));
            printf("The destination cell is found\n");
            foundDest = true;
            tracePath(closedList, dest);
            return;
        }

        list<pair<long, double>> successors = pos.getSuccessors();
        list<pair<long, double>>::iterator it;
        for (it = successors.begin(); it != successors.end(); ++it) {
            // successor<id, distance>
            pair<long, double> np = *it;
            
            // If the successor is already on the closed 
            // list ignore it.
            if (closedList.find(np.first) != closedList.end()) {
                continue;
            }
            // Else do the following
            
            Node nextPos = graph.at(np.first);

            olData d;
            d.id = np.first;
            d.parent_id = olistData.id;
            d.g = olistData.g + np.second;
            d.h = nextPos.distanceToPosition(src);
            d.f = d.g + d.h;
            openList.push(d);
        }
        closedList.insert(make_pair(olistData.id, olistData.parent_id));
    }

    // When the destination cell is not found and the open 
    // list is empty, then we conclude that we failed to 
    // reach the destiantion cell. This may happen when the 
    // there is no way to destination cell (due to blockages) 
    if (foundDest == false)
        printf("Failed to find the Destination Cell\n");
    return;
}

void Summe(double x, double y, double *sum) {
    *sum = x + y;
}

int main()
{
    double a1 = 1.2; double b1 = 2.3;
    double s;

    Summe(a1, b1, &s);
    cout << a1 << "+" << b1 << "=" << s;






    return 0;


    importXml();

    unordered_map<int, Node> nodes;

    Node a(123456, 0.0, 0.0);
    Node b(123457, 8.0, 0.0);
    Node c(123458, 0.0, 8.0);
    Node d(123459, 8.0, 8.0);
    Node e(123450, 12.0, 12.0);
    
    nodes.insert(make_pair(a.getID(), a));
    nodes.insert(make_pair(b.getID(), b));
    nodes.insert(make_pair(c.getID(), c));
    nodes.insert(make_pair(d.getID(), d));
    nodes.insert(make_pair(e.getID(), e));

    a.addSuccessor(b.getID(), a.distanceToPosition(b));
    a.addSuccessor(c.getID(), a.distanceToPosition(c));
    b.addSuccessor(a.getID(), b.distanceToPosition(a));
    b.addSuccessor(c.getID(), b.distanceToPosition(c));
    b.addSuccessor(d.getID(), b.distanceToPosition(d));
    b.addSuccessor(e.getID(), b.distanceToPosition(e));
    c.addSuccessor(b.getID(), c.distanceToPosition(b));
    c.addSuccessor(d.getID(), c.distanceToPosition(d));
    c.addSuccessor(e.getID(), c.distanceToPosition(e));
    e.addSuccessor(b.getID(), e.distanceToPosition(b));
    
    unordered_map<long, Node> graph;                 // change to set
    graph.insert(make_pair(a.getID(), a));
    graph.insert(make_pair(b.getID(), b));
    graph.insert(make_pair(c.getID(), c));
    graph.insert(make_pair(d.getID(), d));
    graph.insert(make_pair(e.getID(), e));

    Node src = a;
    Node dest = e;

    aStarSearch(graph, src, dest);

    return(0);
}


