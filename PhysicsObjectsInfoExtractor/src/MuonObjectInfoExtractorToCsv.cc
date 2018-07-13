// -*- C++ -*-
//
// Package:    MuonObjectInfoExtractorToCsv
// Class:      MuonObjectInfoExtractorToCsv
// 
/**\class MuonObjectInfoExtractorToCsv MuonObjectInfoExtractorToCsv.cc PhysicsObjectsInfo/MuonObjectInfoExtractorToCsv/src/MuonObjectInfoExtractorToCsv.cc

 Description: [Example on how to extract physics information from a CMS EDM Muon Collection]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Edgar F. Carrera Jarrin (ecarrera@cern.ch)
//         Created:  Wed Jul  4 13:38:41 CEST 2018
// $Id$
//
// Notes:
// 
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

//classes included to extract muon information
#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/MuonReco/interface/MuonFwd.h" 

//classes included to extract tracking for the muons
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"

//additional classes for storage, containers and operations
#include<vector>
#include<string>
#include "TFile.h"
#include "TTree.h"
#include <stdlib.h>
#include<iostream>
#include<fstream>
#include<sstream>



//
// class declaration
//

class MuonObjectInfoExtractorToCsv : public edm::EDAnalyzer {
   public:
      explicit MuonObjectInfoExtractorToCsv(const edm::ParameterSet&);
      ~MuonObjectInfoExtractorToCsv();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);


   private:
      virtual void beginJob() ;
      virtual void analyze(const edm::Event&, const edm::EventSetup&);
      virtual void endJob() ;

      virtual void beginRun(edm::Run const&, edm::EventSetup const&);
      virtual void endRun(edm::Run const&, edm::EventSetup const&);
      virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&);
      virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&);
 
 //declare a function to do the muon analysis
      void analyzeMuons(const edm::Event& iEvent, const edm::Handle<reco::MuonCollection> &muons);
  //function to store info in csv
  void dumpMuonsToCsv();
  //declare the input tag for the muons collection to be used (read from cofiguration)
  edm::InputTag muonsInput;
  int maxNumObjt;

  //Declare some variables for storage
  std::ofstream myfile;
  int maxpart;
  std::ostringstream oss;
  std::string theHeader;


  //and declare variable that will go into the root tree
  int runno; //run number
  int evtno; //event number
  int nmu;//number of muons in the event
  std::string mu_partype; //type of particle
  std::vector<float> mu_e;
  std::vector<float> mu_pt;
  std::vector<float> mu_px;
  std::vector<float> mu_py;
  std::vector<float> mu_pz;
  std::vector<float> mu_eta;
  std::vector<float> mu_phi;
  std::vector<float> mu_ch;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
MuonObjectInfoExtractorToCsv::MuonObjectInfoExtractorToCsv(const edm::ParameterSet& iConfig)

{
  //This should match the configuration in the corresponding python file
  muonsInput = iConfig.getParameter<edm::InputTag>("InputCollection");
  maxNumObjt = iConfig.getUntrackedParameter<int>("maxNumberMuons",5);

}


MuonObjectInfoExtractorToCsv::~MuonObjectInfoExtractorToCsv()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called for each event  ------------
void
MuonObjectInfoExtractorToCsv::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   using namespace edm;

   //get the global information first
   runno = iEvent.id().run();
   evtno  = iEvent.id().event();

   //Declare a container (or handle) where to store your muons.
   //https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideDataFormatRecoMuon
   Handle<reco::MuonCollection> mymuons;

   //This is where the information gets extracted from the EDM file
   //Essentially, this corresponds to the information stored in a specific
   //branch within the EDM files.  For example, for recoMuons one could get
   //just "muons", which would be the most used reco muons collection,
   //but also "muonsFromCosmics", which could be used to study fakes.
   //If you explore the branches in the EDM file with a ROOT TBrowser, 
   //you can indeed find a
   //"recoMuons_muons__RECO" branch and a "recoMuons_muonsFromCosmics__RECO" one.
   //Therefore, following the documentation
   //(https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideEDMGetDataFromEvent?rev=20),
   //one could simply write "muons" instead of 
   //the muonsInput variable, which is extracted from
   //the configuration above.  However, using such a configuration variable
   //allows one to access a different branch or type of muons, some cosmic muons
   //for example, without having to recompile the code.
   iEvent.getByLabel(muonsInput, mymuons); 

   //Now, to keep it orderly, pass the collection to a subroutine that extracts
   //some of  the muon information
   //We do need to pass the event.  We could have also passed
   //the event setup if it were needed.
   analyzeMuons(iEvent,mymuons);
   dumpMuonsToCsv();
   return;

}

// ------------ function to analyze muons
void 
MuonObjectInfoExtractorToCsv::analyzeMuons(const edm::Event& iEvent, const edm::Handle<reco::MuonCollection> &muons)
{
  //clear the storage containers for this objects in this event
  nmu=0;
  mu_e.clear();
  mu_px.clear();
  mu_py.clear();
  mu_pz.clear();
  mu_pt.clear();
  mu_eta.clear();
  mu_phi.clear();
  mu_ch.clear();

  //check if the collection is valid
  if(muons.isValid()){
    //get the number of muons in the event
	//loop over all the muons in this event
    int idx = 0;
	for (reco::MuonCollection::const_iterator recoMu = muons->begin(); recoMu!=muons->end(); ++recoMu){
      //find only globlal muons for this specific example
      //https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookMuonAnalysis?rev=88
	  if(recoMu->isGlobalMuon()) {
	    mu_partype = "G"; 
	    mu_e.push_back(recoMu->energy());
	    mu_pt.push_back(recoMu->pt());
	    mu_px.push_back(recoMu->px());
	    mu_py.push_back(recoMu->py());
	    mu_pz.push_back(recoMu->pz());
	    mu_eta.push_back(recoMu->eta());
	    mu_phi.push_back(recoMu->phi());
	    mu_ch.push_back(recoMu->charge());
	    ++idx;
	  }
	}
	nmu = idx;
  }
  
}

// ------------ function to analyze muons
void MuonObjectInfoExtractorToCsv::dumpMuonsToCsv()
{
  unsigned int maxnumobjt = maxNumObjt;
  if(nmu>0){
  oss.str("");oss.clear();oss<<runno;
  myfile<<oss.str();
  oss.str("");oss.clear();oss<<evtno;
  myfile<<","<<oss.str();
    for (unsigned int j=0;j<maxnumobjt;j++){
      oss.str("");oss.clear();oss<<mu_partype;
      myfile<<","<<oss.str();
      oss.str("");oss.clear();oss<<mu_e[j];
      j<mu_e.size() ? myfile<<","<<oss.str():myfile<<",0.0";
      //      std::cout<<maxnumobjt<<"\t"<<nmu<<"\t"<<mu_e.size()<<"\t"<<mu_e[j]<<"\t"<<oss.str()<<std::endl;
      oss.str("");oss.clear();oss<<mu_px[j];
      j<mu_px.size() ? myfile<<","<<oss.str():myfile<<",0.0";
      oss.str("");oss.clear();oss<<mu_py[j];
      j<mu_py.size() ? myfile<<","<<oss.str():myfile<<",0.0";
      oss.str("");oss.clear();oss<<mu_pz[j];
      j<mu_pz.size() ? myfile<<","<<oss.str():myfile<<",0.0";
      oss.str("");oss.clear();oss<<mu_pt[j];
      j<mu_pt.size() ? myfile<<","<<oss.str():myfile<<",0.0";
      oss.str("");oss.clear();oss<<mu_eta[j];
      j<mu_eta.size() ? myfile<<","<<oss.str():myfile<<",0.0";
      oss.str("");oss.clear();oss<<mu_phi[j];
      j<mu_phi.size() ? myfile<<","<<oss.str():myfile<<",0.0";
      oss.str("");oss.clear();oss<<mu_ch[j];
      j<mu_ch.size() ? myfile<<","<<oss.str():myfile<<",0.0";
    }
  myfile<<"\n";
  }
}


// ------------ method called once each job just before starting event loop  ------------
void 
MuonObjectInfoExtractorToCsv::beginJob()
{
  //Define storage
  myfile.open("MuonObjectInfo.csv");
  //Write the header.
  //create the header string accordingly
  theHeader = "Run,Event";
  for(int j =1;j<maxNumObjt+1;j++){
    oss.str(""); oss<<j;
    std::string idxstr = oss.str();
    theHeader += ",type"+idxstr+",E"+idxstr+",px"+idxstr+",py"+idxstr+",pz"+idxstr+",pt"+idxstr+",eta"+idxstr+",phi"+idxstr+",Q"+idxstr;
  }
  
  myfile<<theHeader<<"\n";

}

// ------------ method called once each job just after ending the event loop  ------------
void 
MuonObjectInfoExtractorToCsv::endJob() 
{

  //save file
  myfile.close();

}

// ------------ method called when starting to processes a run  ------------
void 
MuonObjectInfoExtractorToCsv::beginRun(edm::Run const&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a run  ------------
void 
MuonObjectInfoExtractorToCsv::endRun(edm::Run const&, edm::EventSetup const&)
{
}

// ------------ method called when starting to processes a luminosity block  ------------
void 
MuonObjectInfoExtractorToCsv::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a luminosity block  ------------
void 
MuonObjectInfoExtractorToCsv::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
MuonObjectInfoExtractorToCsv::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(MuonObjectInfoExtractorToCsv);std::ostringstream oss;
