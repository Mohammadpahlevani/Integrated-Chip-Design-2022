#ifndef SIMPLE_H
#define SIMPLE_H

#include <iostream>
#include <list>
using namespace std;
#include "atlasDB.h"

enum PlacerRunMode { none_pl_mode, read_file_mode, read_db_mode };

struct simplePoint  {
	float x;
	float y;
};

struct simpleRect  {
	float t;
	float l;
	float b;
	float r;
};
struct simpleNet {
	char 	*netName;
	int NetID;
	list <struct simpleInstance *> *instances;
	bool tag;
	simpleInstance *srcTerminal;
};

struct simpleInstance {
	int	InstID;
	char 	*InstName;
   	char 	*CellName;
    simplePoint TL;
    float 	Width, Height;
	simpleNet *net;
	int rowNo;
	bool isSelected;
};

struct PerturbLog {
	simpleInstance *inst1;
	simpleInstance *inst2;
};

struct simpleDumpInst
{
  string instName;
  simplePoint TL;
};

class SimplePlacer {
public:
	SimplePlacer();
	~SimplePlacer();
	int simPlacer();
	int readDB(atlasDB *);
	void updateDB(atlasDB *);
	void dumpPlacement ( char *dumpFile );
	void readDump (char *dumpFile);
private:
	simpleInstance *findInstance(string instName);
	list <simpleInstance *> *mList; // List of modules wherein design
	list <simpleNet *> *nList; // List of nets wherein design
	float rowHeight;
	unsigned maxlayoutW;
	unsigned maxlayoutH;
	int rowNumber;
	double THPWL();

};
#endif



