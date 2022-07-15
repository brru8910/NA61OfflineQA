#include <iostream>
#include <list>
#include <vector>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TCanvas.h>
#include <TObject.h>
#include <TKey.h>
#include <TString.h>
#include <TStyle.h>
#include <TLatex.h>
#include <TFitResult.h>
#include <TGraph.h>
#include <algorithm>

using namespace std;

void FitForMaxADC(TObject* object) {
  TLatex label; //for MaxADC
  label.SetTextSize(0.1);
  
  //This must be a TH1D*!
  if (string(object->ClassName()) != string("TH1D")) {
    cout << "[ERROR] Tried to fit for Max ADC on a non-TH1D object!" << endl;
  }
  
  TH1D* histogram = static_cast<TH1D*>(object);

  //Get max value to fit around. Fit near max with Landau.
  double maxVal = histogram->GetXaxis()->GetBinCenter(histogram->GetMaximumBin());
  TFitResultPtr r = histogram->Fit("landau", "SI", "", maxVal - 20, maxVal + 40);

  //Draw result.  
  int fitStatus = r;
  if (fitStatus >= 0) {
    double maxMaxADC = r->Parameter(1);
    label.DrawLatexNDC(0.38, 0.75, Form("MaxADC_{max}#approx%0.1f", 
					maxMaxADC));
    histogram->GetXaxis()->SetRange(0, 130);
  }
}

void ProcessPlots(const char* filename="../OfflineQA.root")
{
  vector<string> objectsToNotDraw {"TTree","TNtuple"}; 

  //Add histogram names with special draw options here. Key: Histogram
  //name. Value: Draw options.
  map<string,string> drawOptionsMap {
    {"TGraph","AP"},
      {"TGraphErrors","AP"},
	{"TGraphAsymmErrors","AP"},
	  {"TH2D","COLZ"},
	    {"TH2F","COLZ"}
  };
  //Add histogram names to be plotted with log X-scale here.
  vector<string> objectsWithLogX{
  };
  //Add histogram names to be plotted with log Y-scale here.
  vector<string> objectsWithLogY{
  };
  //Add histogram names to be plotted with log Z-scale here.
  vector<string> objectsWithLogZ{
    "SecondaryVerticesXZ","SecondaryVerticesZY","SecondaryVerticesZX","Cl_XY","Cl_ZX","Cl_ZY"
  };
  map<string,double> graphMins {
    {"T2T1Ratio",0},
      {"T4T3Ratio",0},
	{"T1T3Ratio",0},
	  {"T1S1Ratio",0},
	    {"T3S1Ratio",0},
	      {"S1Counts",0}
  };
  map<string,double> graphMaxes {
    {"T2T1Ratio",1.1},
      {"T4T3Ratio",1.1},
	{"T1T3Ratio",1.1},
	  {"T1S1Ratio",1.1},
	    {"T3S1Ratio",1.1}
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

    //Fit for maxADC.
    if (name.find("MaxADC") != string::npos)
      FitForMaxADC(obj);

    const TString saveName = "plots/" + string(obj->GetName()) + ".png";
    c.SaveAs(saveName.Data());
    c.Close();
  }
} 
