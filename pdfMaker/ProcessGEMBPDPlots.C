#include <iostream>
#include <list>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TCanvas.h>
#include <TObject.h>
#include <TGraph.h>
#include <TKey.h>
#include <TString.h>
#include <TStyle.h>

using namespace std;

//Set nice date format for x-axis.                                                                  
void SetDateFormat(TObject* object) {
  if (string(object->ClassName()) != "TGraph") {
    cout << "[ERROR] Tried to set date axis for a non-TGraph!" << endl;
    return;
  }

  TGraph* graph = static_cast<TGraph*>(object);
  graph->GetXaxis()->SetTimeDisplay(1);
  graph->GetXaxis()->SetNdivisions(503);
  graph->GetXaxis()->SetTimeFormat("%Y-%m-%d %H:%M");
  graph->GetXaxis()->SetTimeOffset(0,"gmt");

  return;
}

void ProcessGEMBPDPlots(const char* filename="../OfflineQA.root")
{
  vector<string> objectsToNotDraw {"TTree","TNtuple"}; 

  //Add histogram names with special draw options here. Key: Histogram
  //name. Value: Draw options.
  map<string,string> drawOptionsMap {
    {"TGraph","AP"},
      {"TGraphErrors","AP"},
	{"TGraphAsymmErrors","AP"},
	  {"TH2D","COLZ"}
  };
  //Add histogram names to be plotted with log X-scale here.
  vector<string> objectsWithLogX{
  };
  //Add histogram names to be plotted with log Y-scale here.
  vector<string> objectsWithLogY{
    "eventIdDeltaTs"
  };
  //Add histogram names to be plotted with log Z-scale here.
  vector<string> objectsWithLogZ{
    "GEMBPD1Occupancy",
    "GEMBPD1CoincidenceOccupancy",
    "GEMBPD2Occupancy",
    "GEMBPD2CoincidenceOccupancy",
    "GEMBPD3Occupancy",
    "GEMBPD3CoincidenceOccupancy"
  };
  map<string,double> graphMins {
    {"BPD1XMean",0},
    {"BPD1YMean",0},
    {"BPD2XMean",0},
    {"BPD2YMean",0},
    {"BPD3XMean",0},
    {"BPD3YMean",0},
    {"BPD1XWidth",0},
    {"BPD1YWidth",0},
    {"BPD2XWidth",0},
    {"BPD2YWidth",0},
    {"BPD3XWidth",0},
    {"BPD3YWidth",0},

    {"BPD1XMeanCoincidence",0},
    {"BPD1YMeanCoincidence",0},
    {"BPD2XMeanCoincidence",0},
    {"BPD2YMeanCoincidence",0},
    {"BPD3XMeanCoincidence",0},
    {"BPD3YMeanCoincidence",0},
    {"BPD1XWidthCoincidence",0},
    {"BPD1YWidthCoincidence",0},
    {"BPD2XWidthCoincidence",0},
    {"BPD2YWidthCoincidence",0},
    {"BPD3XWidthCoincidence",0},
    {"BPD3YWidthCoincidence",0},

      {"GEMBPD1Efficiency",0.},
	{"GEMBPD2Efficiency",0.},
	  {"GEMBPD3Efficiency",0.},

    {"eventTransferGraph",0.}
  };
  map<string,double> graphMaxes {
    {"BPD1XMean",5.},
    {"BPD1YMean",5.},
    {"BPD2XMean",5.},
    {"BPD2YMean",5.},
    {"BPD3XMean",5.},
    {"BPD3YMean",5.},
    {"BPD1XWidth",1.5},
    {"BPD1YWidth",1.5},
    {"BPD2XWidth",1.5},
    {"BPD2YWidth",1.5},
    {"BPD3XWidth",1.5},
    {"BPD3YWidth",1.5},

    {"BPD1XMeanCoincidence",5.},
    {"BPD1YMeanCoincidence",5.},
    {"BPD2XMeanCoincidence",5.},
    {"BPD2YMeanCoincidence",5.},
    {"BPD3XMeanCoincidence",5.},
    {"BPD3YMeanCoincidence",5.},
    {"BPD1XWidthCoincidence",1.5},
    {"BPD1YWidthCoincidence",1.5},
    {"BPD2XWidthCoincidence",1.5},
    {"BPD2YWidthCoincidence",1.5},
    {"BPD3XWidthCoincidence",1.5},
    {"BPD3YWidthCoincidence",1.5},

      
      {"GEMBPD1Efficiency",110},
	{"GEMBPD2Efficiency",110},
	  {"GEMBPD3Efficiency",110},

    {"eventTransferGraph",110}

  };


  //No stats.
  gStyle->SetOptStat(0);

  //Make sure file is open.  
  TFile* file = new TFile(filename) ;
  if (!file->IsOpen()) {
    exit(1) ;
  }

  //Get keys.
  TIter fileIter(file->GetListOfKeys());
  TKey* key ;

  //Loop through keys.
  while ((key = (TKey*)fileIter())) {
    TObject* obj = key->ReadObj();

    const string name = obj->GetName();
    const string type = obj->ClassName();

    bool setLogX = false;
    bool setLogY = false;
    bool setLogZ = false;

    for (auto it = objectsWithLogX.begin(), itEnd = objectsWithLogX.end(); it != itEnd; ++it)
      if (name.find(*it) != string::npos)
	setLogX = true;
    for (auto it = objectsWithLogY.begin(), itEnd = objectsWithLogY.end(); it != itEnd; ++it)
      if (name.find(*it) != string::npos)
	setLogY = true;
    for (auto it = objectsWithLogZ.begin(), itEnd = objectsWithLogZ.end(); it != itEnd; ++it)
      if (name.find(*it) != string::npos)
	setLogZ = true;

    const string drawOptions = (drawOptionsMap.find(type) != drawOptionsMap.end()) ? 
      drawOptionsMap[type] : "";
    
    //Make sure we should draw this type of object.
    if (find(objectsToNotDraw.begin(),objectsToNotDraw.end(),type) != objectsToNotDraw.end())
      continue;
    
    //Set date format for TGraphs.
    if (type == "TGraph" && name != "eventIdVsTime") 
      SetDateFormat(obj);

    TCanvas c("QACanvas", "", 900, 600);

    //Draw with options. Include special options, if desired.
    obj->Draw(drawOptions.c_str());
    
    if (setLogX)
      c.SetLogx();
    if (setLogY)
      c.SetLogy();
    if (setLogZ)
      c.SetLogz();
    
    //Set limits for TGraphs, if defined.
    if (graphMins.find(name) != graphMins.end()) {
      TGraph* graph = static_cast<TGraph*>(obj);
      graph->SetMinimum(graphMins.at(name));
    }
    if (graphMaxes.find(name) != graphMaxes.end()) {
      TGraph* graph = static_cast<TGraph*>(obj);
      graph->SetMaximum(graphMaxes.at(name));
    }

    const TString saveName = "plots/" + string(obj->GetName()) + ".png";
    c.SaveAs(saveName.Data());
    c.Close();
  }
} 
