/**
	*********************************************************
	In this file, basic functions of parser are implemented.
	*********************************************************
**/

#include <stdlib.h>
#include <string.h>
#include <placer.h>
#include <stdio.h>
#include <math.h>

#include <iostream>
#include <list>
#include <vector>
#include <map>


using namespace std;
#include <atlasDB.h>
unsigned layoutH, layoutW, cellHeight;

#define SRC 101
#define SNK 102

#define __whiteSpace 1.1

#define __whiteSpace 1.1
#define DBU 2000

//-----------------------------------------------------------
// Constructors
//-----------------------------------------------------------
SimplePlacer::SimplePlacer()
{
	mList = new list <simpleInstance *>;
	nList = new list <simpleNet *>;
}

//-----------------------------------------------------------
// Destructors
//-----------------------------------------------------------
SimplePlacer::~SimplePlacer()
{
}

//-----------------------------------------------------------
// Read DB information
//-----------------------------------------------------------
int SimplePlacer::readDB ( atlasDB *db )
{
	TInstData *dbInst = new TInstData;
	list <simpleInstance *>::iterator iter;
	simpleInstance *anInst;
	TNetData *dbNet = new TNetData;
	TPinData *dbPin = new TPinData;
	TPointData *pinXY = new TPointData;
	float layoutArea = 0, Dim, modDim;
	int cnt;
	printf ( "\tReading %d cells ... \n", db->getInstancesNum() );
	//------------------
	// Reading Instances
	//------------------
	db->resetInstListPos();
	cnt = 0;
	while ( db->getInstData ( dbInst ) == true )
	{
		//--------------------------
		// Creating a new instance
		//--------------------------
		anInst = new simpleInstance;
		anInst->rowNo = 0;
		anInst -> TL.x = 0;
		anInst -> TL.y = 0;
		anInst -> InstID = cnt + 1;
		anInst -> InstName = new char[100];
		strcpy ( anInst -> InstName, dbInst->instName.c_str() );
		anInst -> CellName = new char[100];
		strcpy ( anInst -> CellName, dbInst->macroName.c_str() );
		anInst -> Width = dbInst->width;
		anInst -> Height = dbInst->height;
		rowHeight = anInst->Height;
		mList->push_back ( anInst );
		db->instGoForward();
		cnt ++;
	}
	cellHeight = rowHeight;

	printf ( "\tReading and elaborating %d nets ... ", db->getNetsNum() );
	unsigned nn = 0;
	db->resetNetListPos();
	while ( db->getNetData ( dbNet ) == true )
	{
		simpleNet *aNet;
		aNet = new simpleNet;
		aNet -> netName = new char[100];
		aNet->instances = new list <struct simpleInstance *>;
		aNet->srcTerminal = NULL;
		strcpy ( aNet -> netName, dbNet -> name . c_str() );
/*		if ( ( nn%100 ) == 0 )
			printf ( "\t\t%d / %d\n", nn, db->getNetsNum() );*/
		nn ++;
		if ( dbNet -> name == "POWR" )
		{
			printf ( "\t\tNet Power is ignored.\n" );
			db->NetDataGoForward();
			continue;
		}
		if ( dbNet -> name == "GRND" )
		{
			printf ( "\t\tNet Ground is ignored.\n" );
			db->NetDataGoForward();
			continue;
		}
		db->resetPinOfNetListPos();
		while ( db->getPinOfNetData ( dbPin ) == true )
		{
			db->getPinParentPos ( pinXY );
			db->getPinParent ( dbInst );
			simpleInstance *anInst =  findInstance ( dbInst->instName );
			if ( anInst == NULL )
			{
				printf ( "Netlist annotation error.\n" );
			}
			else
			{
				aNet->instances->push_back ( anInst );
				if ( dbPin->dir == output )
					aNet->srcTerminal = anInst;
			}
			db->PinOfNetGoForward();
		}
		nList->push_back ( aNet );
		db->NetDataGoForward();
	}
	printf ( "OK.\n" );

	//----------------------------
	// Estimate layout dimension
	//----------------------------
	for ( iter = mList->begin(); iter != mList->end(); iter ++ )
	{
		anInst = ( simpleInstance * ) ( *iter );
		layoutArea += anInst->Width * anInst->Height;
	}

	Dim = floor ( sqrt ( layoutArea * __whiteSpace ) );
	modDim = Dim - ( int ) Dim % ( int ) rowHeight;
	maxlayoutW = ( unsigned ) ( modDim );
	maxlayoutH = ( unsigned ) ( modDim );
	printf ( "\tRead DB: %d insts are read.\n", mList->size() );
	printf ( "\tRead DB: %d nets are read.\n" , nList->size() );
	
	return 0;
}

//-----------------------------------
// Read Placed Netlist from file
//-----------------------------------
void SimplePlacer::readDump(char *dumpFile)
{
	FILE *fp;
	char iName[64];
	float iLeft, iTop;
	unsigned int cnt = 0, iNumber = 0; 
	
	fp = fopen(dumpFile, "r+t");
	if (fp == NULL)
	{
	    printf("Cannot read dump file %s\n", dumpFile);
	    exit(1);
	}

	fscanf(fp,"%d %d %d %d\n", &iNumber, &maxlayoutW, &maxlayoutH, &rowNumber);
// 	printf("#cells = %d, W = %d, H = %d, Row number = %d\n", iNumber, maxlayoutW, maxlayoutH, rowNumber);

	for(unsigned i = 0;i < iNumber;i ++)
	{
	    fscanf(fp,"%s %f %f \n",iName, &iLeft, &iTop);
// 	    printf("i = %d, name = %s , x = %f , y = %f \n",i, iName, iLeft, iTop);
	    simpleInstance *iPtr = findInstance(iName);
	    if (iPtr == NULL)
	    {
		printf("Instance %s is not found in instance list.\n");
		exit(1);
	    }
	    else
	    {
		iPtr -> TL.x = iLeft;
		iPtr -> TL.y = iTop;
		cnt ++;
	    }
	}
	printf("%d in stances are loaded from dump\n", cnt);
}

simpleInstance *SimplePlacer::findInstance ( string instName )
{
	list <simpleInstance *>::iterator iter;
	simpleInstance *anInst;

	for ( iter = mList->begin(); iter != mList->end(); iter ++ )
	{
		anInst = ( simpleInstance * ) ( * iter );
		if ( strcmp ( anInst-> InstName, instName.c_str() ) == 0 )
			return anInst;
	}
	return NULL;
}

int SimplePlacer::simPlacer()
{
	list <simpleInstance *>::iterator iter, iter1;
	simpleInstance *anInst, *anInst1;
	int rowNo, i;
	float rowWidth, mWidth;
	
	rowNo = 0;
	rowWidth = 0;

	for ( iter = mList->begin(); iter != mList->end(); iter ++ )
	{	
		mWidth = 0; 
		for ( iter1 = mList->begin(); iter1 != mList->end(); iter1 ++ )
		{
			anInst = ( simpleInstance * ) ( * iter1 );
			if (anInst->Width > mWidth && anInst->isSelected != true)
			{
				mWidth = anInst->Width;
				anInst1 = anInst;
			}
		}
		anInst1->isSelected = true;
		if ( rowWidth < maxlayoutW )
		{
			anInst1->rowNo = rowNo;
			anInst1->TL.x = rowWidth;
			anInst1->TL.y = rowNo * rowHeight;
			rowWidth += anInst1->Width;
		}
		else
		{
			rowNo ++;
			anInst1->rowNo = rowNo;
			anInst1->TL.x = 0;
			anInst1->TL.y = rowNo * rowHeight;
			rowWidth = anInst1->Width;
		}
	}
	rowNumber = rowNo;
	layoutW = maxlayoutW;
	layoutH = maxlayoutH;
	return 1;
}

void SimplePlacer::updateDB ( atlasDB *db )
{
	list <simpleInstance *>::iterator iter;
	simpleInstance *iPtr;
	unsigned maxX = 0, maxY = 0;;
	int i;
	int totalCellNo = 0;

	for ( iter = mList->begin(); iter != mList->end(); iter ++ )
	{
		iPtr = ( simpleInstance * ) ( *iter );

		if ( db->setInstTopLeft ( iPtr->InstName, ( long int ) ( iPtr->TL.y ), ( long int ) ( iPtr->TL.x ) ) == false )
		{
			printf ( "Update DB violation!\n" );
			exit ( 1 );
		}
		totalCellNo ++;
		if ( ( iPtr->TL.x + iPtr->Width ) > maxX )
			maxX = iPtr->TL.x + iPtr->Width;

		if ( ( iPtr->TL.y + iPtr->Height ) > maxY )
			maxY = iPtr->TL.y + iPtr->Height;

		maxlayoutW = maxX;
		maxlayoutH = maxY;
	}

	printf ( "\tUpdate: %d instances are updated in DB\n", totalCellNo );
	printf ( "\tUpdate: layout area: w = %d , h = %d\n", maxlayoutW, maxlayoutH );

	//	set layout dimensions
	db->setNetlistPlaceResults ( ( long int ) ( maxX ), ( long int ) ( maxY ), rowNumber, ( long int ) ( rowHeight ) );
}

void SimplePlacer::dumpPlacement ( char *dumpFile )
{
	list <simpleInstance *>::iterator iter1;
	simpleInstance *anInst;
	FILE *fp;
    

	fp = fopen(dumpFile, "w+t");
	if (fp == NULL)
	{
	    printf("Cannot write dump file %s\n", dumpFile);
	    exit(1);
	}
	fprintf(fp,"%d %d %d %d\n",mList->size(), (unsigned int) (maxlayoutW), (unsigned int)(maxlayoutH), rowNumber);

	for ( iter1 = mList->begin();iter1 != mList->end();iter1 ++ )
	{
	    anInst = ( simpleInstance * ) ( * iter1 );
	    fprintf(fp,"%s %f %f\n", anInst->InstName, (float)(anInst->TL.x), (float)(anInst->TL.y) );
	}
	fclose(fp);
}


double SimplePlacer::THPWL()
{
	list <simpleNet *>::iterator iter1;
	list <simpleInstance *>::iterator iter2;
	simpleNet *aNet;
	simpleInstance *anInst;
	long int wl, minx, miny, maxx, maxy;
	double THPWL = 0;

	for ( iter1 = nList->begin();iter1 != nList->end();iter1 ++ )
	{
		aNet = ( simpleNet * ) ( * iter1 );
		wl = 0;
		minx = 0x7FFFFFFF;
		miny = 0x7FFFFFFF;
		maxx = -1;
		maxy = -1;
		for ( iter2 = aNet->instances->begin();iter2 != aNet->instances->end();iter2 ++ )
		{
			anInst = ( simpleInstance * ) ( * iter2 );
			if ( anInst->TL.x < minx )
				minx = anInst->TL.x;
			if ( anInst->TL.x > maxx )
				maxx = anInst->TL.x;
			if ( anInst->TL.y < miny )
				miny = anInst->TL.y;
			if ( anInst->TL.y > maxy )
				maxy = anInst->TL.y;
			wl += ( maxy - miny ) + ( maxx - minx );
		}
		THPWL += wl;
	}
	THPWL /= DBU;
// 	printf("\t\tTotal half perimeter wirelength = %lf um\n", THPWL);
	return THPWL;
}

