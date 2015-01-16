// -*- C++ -*-
//
// Xin Shi <Xin.Shi@cern.ch>
// Fri Dec 5 11:24:56 CET 2014
// 
// Analysis for the photon + jet
// 

#include <iostream>

#include "FWCore/FWLite/interface/AutoLibraryLoader.h"
#include "FWCore/PythonParameterSet/interface/MakeParameterSets.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "UserCode/llvv_fwk/interface/MacroUtils.h"
#include "UserCode/llvv_fwk/interface/SmartSelectionMonitor.h"
#include "UserCode/llvv_fwk/interface/PatUtils.h"

#include "TSystem.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"

SmartSelectionMonitor
initHistograms(){
  SmartSelectionMonitor mon;
  // pu control
  mon.addHistogram(new TH1F("nvtx", ";Vertices;Events", 50, 0, 50) ); 
  // photon control
  mon.addHistogram(new TH1F("rho", ";Average energy density (#rho);Events", 100, 0, 100) ); 
  mon.addHistogram(new TH1F("npho", ";Photons;Events", 20, 0, 20) ); 
  mon.addHistogram(new TH1F("phopt", ";Photon transverse momentum [GeV];Events", 100, 0, 1000) ); 
  mon.addHistogram(new TH1F("phoeta", ";Photon pseudo-rapidity;Events", 50, 0, 5) );
  // mon.addHistogram(new TH1F("phor9", ";Photon R9;Events", 10, 0, 1) );
  // mon.addHistogram(new TH1F("phoiso", ";Photon Iso;Events", 100, 0, 100) );
  mon.addHistogram(new TH1F("phohoe", ";Photon H/E;Events", 100, 0, 1) );
  mon.addHistogram(new TH1F("elevto", ";Electron Veto;Events", 2, 0, 1) );
  mon.addHistogram(new TH1F("sigietaieta", ";#sigma_{i#eta i#eta};Events", 100, 0, 0.1) );
  return mon; 
}


bool
passPhotonTrigger(fwlite::ChainEvent ev, float &triggerThreshold) {
  edm::TriggerResultsByName tr = ev.triggerResultsByName("HLT");
  if( !tr.isValid() ) return false;

  bool hasPhotonTrigger(false);
  float triggerPrescale(1.0); 
  // float triggerThreshold(0);
  triggerThreshold = 0.0;

  std::string successfulPath="";
  if( utils::passTriggerPatternsAndGetName(tr, successfulPath, "HLT_Photon300_*")){
    hasPhotonTrigger=true;
    triggerThreshold=300;
  }
  else if( utils::passTriggerPatternsAndGetName(tr, successfulPath, "HLT_Photon250_*")){
    hasPhotonTrigger=true;
    triggerThreshold=250;
  }
  else if( utils::passTriggerPatternsAndGetName(tr, successfulPath, "HLT_Photon160_*")){
    hasPhotonTrigger=true;
    triggerThreshold=160;
  }
  else if( utils::passTriggerPatternsAndGetName(tr, successfulPath, "HLT_Photon150_*")){
    hasPhotonTrigger=true;
    triggerThreshold=150;
  }
  else if( utils::passTriggerPatternsAndGetName(tr, successfulPath, "HLT_Photon135_*")){
    hasPhotonTrigger=true;
    triggerThreshold=135;
  }
  else if( utils::passTriggerPatternsAndGetName(tr, successfulPath, "HLT_Photon120_R9Id90_HE10_Iso40_EBOnly_*")){
    hasPhotonTrigger=true;
    triggerThreshold=120;
  }
  else if( utils::passTriggerPatternsAndGetName(tr, successfulPath, "HLT_Photon90_R9Id90_HE10_Iso40_EBOnly_*")){
    hasPhotonTrigger=true;
    triggerThreshold=92;
  }
  else if(utils::passTriggerPatternsAndGetName(tr, successfulPath, "HLT_Photon75_R9Id90_HE10_Iso40_EBOnly_*")){
    hasPhotonTrigger=true;
    triggerThreshold=77;
  }
  else if(utils::passTriggerPatternsAndGetName(tr, successfulPath, "HLT_Photon50_R9Id90_HE10_Iso40_EBOnly_*")){
    hasPhotonTrigger=true;
    triggerThreshold=50;
  }
  else if(utils::passTriggerPatternsAndGetName(tr, successfulPath, "HLT_Photon36_R9Id90_HE10_Iso40_EBOnly_*")){
    hasPhotonTrigger=true;
    triggerThreshold=36;
  }
  else if(utils::passTriggerPatternsAndGetName(tr, successfulPath, "HLT_Photon22_R9Id90_HE10_Iso40_EBOnly_*")){
    hasPhotonTrigger=true;
    triggerThreshold=22;
  }
      
  if(successfulPath!=""){ //get the prescale associated to it
    fwlite::Handle< pat::PackedTriggerPrescales > prescalesHandle;
    prescalesHandle.getByLabel(ev, "patTrigger");
    pat::PackedTriggerPrescales prescales = *prescalesHandle;
    const edm::TriggerResults& trResults =  prescales.triggerResults();
    prescales.setTriggerNames( ev.triggerNames(trResults) );
    triggerPrescale = prescales.getPrescaleForName(successfulPath);
  }

  return hasPhotonTrigger; 
}

bool
passCutBasedPhotonID(SmartSelectionMonitor mon,
		     std::string label,
		     pat::Photon photon,
		     double rho) {
  // https://twiki.cern.ch/twiki/bin/viewauth/CMS/CutBasedPhotonIdentificationRun2
  // CSA14 selection, conditions: 25ns, better detector alignment. 
  // Used Savvas Kyriacou's slides, mailed from Ilya. 

  TString tag = "all";  
  double weight = 1.0; 

  // Electron Veto
  bool elevto = photon.hasPixelSeed();
  mon.fillHisto("elevto", tag, elevto, weight);
  
  // sigma ieta ieta
  // full5x5 is not ready in 720 yet 
  // float sigmaIetaIeta = photon.full5x5_sigmaIetaIeta();
  // taken from https://github.com/cms-sw/cmssw/blob/CMSSW_7_2_X/PhysicsTools/PatAlgos/plugins/PATPhotonSlimmer.cc#L119-L130
  
  // float sigmaIetaIeta = photon.sigmaIetaIeta(); 
  float sigmaIetaIeta = photon.userFloat("sigmaIetaIeta_NoZS"); 
  mon.fillHisto("sigietaieta", tag, sigmaIetaIeta, weight);

  // H/E 
  float hoe = photon.hadTowOverEm();
  mon.fillHisto("phohoe", tag, hoe, weight);

  // isolation
  bool passIso(true);
  double pt=photon.pt();
  double eta=photon.superCluster()->eta();

  float chIso = photon.chargedHadronIso(); 
  float chArea = utils::cmssw::getEffectiveArea(22,eta,3,"chIso"); 

  float nhIso = photon.neutralHadronIso();
  float nhArea = utils::cmssw::getEffectiveArea(22,eta,3,"nhIso");

  float gIso = photon.photonIso();
  float gArea = utils::cmssw::getEffectiveArea(22,eta,3,"gIso");

  // apply cuts 
  float max_hoe(0);
  float max_sigmaIetaIeta(0);
  float max_chIso(0); 
  float max_nhIso(0); 
  float max_gIso(0); 

  if (label == "Tight") {
    max_hoe = 0.012;
    max_sigmaIetaIeta = 0.0098;
    max_chIso = 1.91;
    max_nhIso = 2.55 + 0.0023*pt; 
    max_gIso  = 1.29 + 0.0004*pt; 
  }

  if ( elevto ) return false;
  if ( hoe > max_hoe) return false; 
  if ( sigmaIetaIeta > max_sigmaIetaIeta ) return false; 
  if ( TMath::Max(chIso-chArea*rho,0.0) > max_chIso ) return false; 
  if ( TMath::Max(nhIso-nhArea*rho,0.0) > max_nhIso ) return false; 
  if ( TMath::Max(gIso-gArea*rho,  0.0) > max_gIso ) return false; 

  return true;

}


pat::PhotonCollection
passPhotonSelection(SmartSelectionMonitor mon,
		    pat::PhotonCollection photons,
		    float triggerThreshold,
		    double rho){

  pat::PhotonCollection selPhotons;
  TString tag = "all";  
  double weight = 1.0;

  for(size_t ipho=0; ipho<photons.size(); ipho++) {
    pat::Photon photon = photons[ipho]; 
    double pt=photon.pt();
    double eta=photon.superCluster()->eta();
    mon.fillHisto("phopt", tag, pt, weight);
    mon.fillHisto("phoeta", tag, eta, weight);
    // mon.fillHisto("phor9", tag, photon.r9(), weight);
    // mon.fillHisto("phoiso", tag, photon.photonIso(), weight);

    if( pt < triggerThreshold || fabs(eta)>1.4442 ) continue;

    bool passPhotonSelection = passCutBasedPhotonID(mon, "Tight", photon, rho); 
    if(!passPhotonSelection) continue; 
    selPhotons.push_back(photon);
  }

  return selPhotons; 
}


int main(int argc, char* argv[])
{
  // check arguments
  if(argc<2){ std::cout << "Usage : " << argv[0] << " parameters_cfg.py" << std::endl; exit(0); }
  
  // load framework libraries
  gSystem->Load( "libFWCoreFWLite" );
  AutoLibraryLoader::enable();
  
  // configure the process
  const edm::ParameterSet &runProcess = edm::readPSetsFrom(argv[1])->getParameter<edm::ParameterSet>("runProcess");
  bool debug = runProcess.getParameter<bool>("debug");
  bool isMC = runProcess.getParameter<bool>("isMC");  
  double xsec = runProcess.getParameter<double>("xsec");
  int mctruthmode=runProcess.getParameter<int>("mctruthmode");

  std::vector<std::string> urls=runProcess.getUntrackedParameter<std::vector<std::string> >("input");
  // TString url = TString(argv[1]);
  // TString outFileUrl(gSystem->BaseName(url));
  // outFileUrl.ReplaceAll("_cfg.py","");
  // if(mctruthmode!=0) { outFileUrl += "_filt"; outFileUrl += mctruthmode; }
  // TString outdir=runProcess.getParameter<std::string>("outdir");
  // TString outUrl( outdir );
  // gSystem->Exec("mkdir -p " + outUrl);

  TString output=runProcess.getParameter<std::string>("output");
  
  // initiating histograms
  SmartSelectionMonitor mon = initHistograms();
  
  // get ready for the event loop
  fwlite::ChainEvent ev(urls);
  const size_t totalEntries= ev.size();

  //MC normalization (to 1/pb)
  double xsecWeight = xsec/totalEntries;

  if(!isMC) xsecWeight=1.0;
  if (debug) {
    printf("DEBUG: xsec= %f\n", xsec);
    printf("DEBUG: xsecWeight = %f\n", xsecWeight);
  }

  // make sure that histogram internally produced in 
  // lumireweighting are not destroyed when closing the file
  gROOT->cd();  

  // event loop
  // loop on all the events
  printf("Progressing Bar     :0%%       20%%       40%%       60%%       80%%       100%%\n");
  printf("Scanning the ntuple :");
  int treeStep(totalEntries/50);
  
  TString tag("all");
  double weight(1.0); // for testing now. 

  for( size_t iev=0; iev<totalEntries; iev++){
    if(iev%treeStep==0){printf(".");fflush(stdout);}
    // load the event content from the EDM file
    ev.to(iev);
    
    //apply trigger and require compatibilitiy of the event with the PD
    float triggerThreshold = 0.0; 
    bool hasPhotonTrigger = passPhotonTrigger(ev, triggerThreshold);

    // only run on the events that pass our triggers
    if( !hasPhotonTrigger ) continue; 
    
    //load all the objects we will need to access
    reco::VertexCollection vtx;
    fwlite::Handle< reco::VertexCollection > vtxHandle; 
    vtxHandle.getByLabel(ev, "offlineSlimmedPrimaryVertices");
    if (vtxHandle.isValid() ) { vtx = *vtxHandle; }
    mon.fillHisto("nvtx", "all", vtx.size(), weight);

    double rho = 0;
    fwlite::Handle< double > rhoHandle;
    rhoHandle.getByLabel(ev, "fixedGridRhoFastjetAll");
    if(rhoHandle.isValid()){ rho = *rhoHandle;}
    mon.fillHisto("rho", "all", rho, weight);
    
    pat::PhotonCollection photons;
    fwlite::Handle< pat::PhotonCollection > photonsHandle;
    photonsHandle.getByLabel(ev, "slimmedPhotons");
    if(photonsHandle.isValid()){ photons = *photonsHandle;}
    mon.fillHisto("npho", "all", photons.size(), weight);
  
    // below follows the analysis of the main selection with n-1 plots
    pat::PhotonCollection selPhotons = passPhotonSelection(mon, photons, triggerThreshold, rho);
    if ( selPhotons.size() == 0) continue;  

    tag = "sel";
    mon.fillHisto("npho", tag, selPhotons.size(), weight);
    mon.fillHisto("nvtx", tag, vtx.size(), weight);

    for(size_t ipho=0; ipho<selPhotons.size(); ipho++) {
      pat::Photon photon = selPhotons[ipho]; 
      mon.fillHisto("phopt", tag, photon.pt(), weight);
      mon.fillHisto("phoeta", tag, photon.superCluster()->eta(), weight);
      // mon.fillHisto("phor9", tag, photon.r9(), weight);
      // mon.fillHisto("phoiso", tag, photon.photonIso(), weight);
      mon.fillHisto("phohoe", tag, photon.hadTowOverEm(), weight);
      mon.fillHisto("elevto", tag, photon.hasPixelSeed(), weight);
      // mon.fillHisto("sigietaieta", tag, photon.sigmaIetaIeta(), weight);
      mon.fillHisto("sigietaieta", tag, photon.userFloat("sigmaIetaIeta_NoZS"), weight);
    }
    
  } // end event loop 
  printf(" done.\n"); 
  
  //save control plots to file
  // outUrl += "/";
  // outUrl += outFileUrl + ".root";
  // printf("Results saved in %s\n", outUrl.Data());
  // TFile *ofile=TFile::Open(outUrl, "recreate");

  printf("Results saved in %s\n", output.Data());
  TFile *ofile=TFile::Open(output, "recreate");

  mon.Write();
  ofile->Close();
}  






