#include <iostream>
#include <set>
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

void DrawWithUncertaintyEnvelope(TObject* object) {
  //This must be a TGraphErrors*!
  if (string(object->ClassName()) != string("TGraphErrors")) {
    cout << "[ERROR] Tried to fit for Max ADC on a non-TGraphErrors object!" << endl;
  }
  
  cout << "Making nice envelope plot for object " << object->GetName() << endl;

  TGraphErrors* graph = static_cast<TGraphErrors*>(object);

  const int color = graph->GetMarkerColor();

  graph->SetLineColor(color);
  graph->SetLineWidth(4);
  graph->SetFillColor(color);
  graph->SetFillColorAlpha(color,0.5);
  // graph->Draw("a3");
  // graph->Draw("LX SAME");
  graph->Draw("alp");
  // graph->Draw("LX SAME");
}

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

  //Objects for which to draw uncertainty envelope. For scalers
  //graphs.
  set<string> uncertaintyEnvelopeObjects {
    "T2T1Ratio",
      "T4T3Ratio",
      "T1T3Ratio",
      "T1S1Ratio",
      "T3S1Ratio",
      "S1Intensity"
      };

  //Add object names with special draw options here. Key: Histogram
  //name. Value: Draw options.
  map<string,string> specialDrawOptionsMap {
    {"runNumberGraph","AL"},
    {"eventIdGraph","AL"}
  };
  
  //Default special draw options for plot types.
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

    //Determine draw options.
    string drawOptions = "";
    if (specialDrawOptionsMap.find(name) != specialDrawOptionsMap.end()) {
      drawOptions = specialDrawOptionsMap[name];
    }
    else if (drawOptionsMap.find(type) != drawOptionsMap.end()) {
      drawOptions = drawOptionsMap[type];
    }
    
    //Make sure we should draw this type of object.
    if (find(objectsToNotDraw.begin(),objectsToNotDraw.end(),type) != objectsToNotDraw.end())
      continue;
    
    TCanvas c("QACanvas", "", 900, 600);

    //Draw with options. Include special options, if desired.
    if (uncertaintyEnvelopeObjects.find(name) != uncertaintyEnvelopeObjects.end())
      DrawWithUncertaintyEnvelope(obj);
    else
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
