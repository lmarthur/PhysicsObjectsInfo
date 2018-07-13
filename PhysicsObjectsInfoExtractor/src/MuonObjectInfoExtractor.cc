// -*- C++ -*-
//
// Package:    MuonObjectInfoExtractor
// Class:      MuonObjectInfoExtractor
// 
/**\class MuonObjectInfoExtractor MuonObjectInfoExtractor.cc PhysicsObjectsInfo/MuonObjectInfoExtractor/src/MuonObjectInfoExtractor.cc

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



//
// class declaration
//

class MuonObjectInfoExtractor : public edm::EDAnalyzer {
   public:
      explicit MuonObjectInfoExtractor(const edm::ParameterSet&);
      ~MuonObjectInfoExtractor();

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
      void analyzeMuons(const edm::Event& iEvent);
  //declare the input tag for the muons collection to be used (read from cofiguration)
  edm::InputTag muonsInput;
  
  //These variable will be global

  //Declare some variables for storage
  TFile* myfile;//root file
  TTree* mytree;//root tree

  //and declare variable that will go into the root tree
  int runno; //run number
  int evtno; //event number
  int nmu; //number of muons in the event
  std::vector<float> mu_e;
  std::vector<float> mu_pt;
  std::vector<float> mu_px;
  std::vector<float> mu_py;
  std::vector<float> mu_pz;
  std::vector<float> mu_eta;
  std::vector<float> mu_phi;
  std::vector<float> mu_ch;
  std::vector<float> mu_glbtrk_pt;
  std::vector<float> mu_glbtrk_eta;
  std::vector<float> mu_glbtrk_phi;

  

  
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
MuonObjectInfoExtractor::MuonObjectInfoExtractor(const edm::ParameterSet& iConfig)

{
  //This should match the configuration in the corresponding python file
  muonsInput = iConfig.getParameter<edm::InputTag>("InputCollection");

}


MuonObjectInfoExtractor::~MuonObjectInfoExtractor()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called for each event  ------------
void
MuonObjectInfoExtractor::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   using namespace edm;

   //get the global information first
   runno = iEvent.id().run();
   evtno  = iEvent.id().event();
   
   //Now, to keep it orderly, pass the collection to a subroutine that extracts
   //some of  the muon information
   //We do need to pass the event.  We could have also passed
   //the event setup if it were needed.  For example, if you need to 
   //store the trigger information you will need to follow
   //this example (https://github.com/cms-opendata-analyses/trigger_examples/tree/master/TriggerInfo/TriggerInfoAnalyzer) and check how to do it.
   analyzeMuons(iEvent);

   //Here, if one were to write a more general PhysicsObjectsInfoExtractor.cc
   //code, this is where the rest of the objects extraction will be, for exmaple:

   //analyzeElectrons(iEvent);
   //analyzeJets(iEvent, iSetup);
   //analyzeMet (iEvent, iSetup);
   //......

  

   //fill the root tree
   mytree->Fill();
   return;

}

// ------------ function to analyze muons
void 
MuonObjectInfoExtractor::analyzeMuons(const edm::Event& iEvent)
{
  //clear the storage containers for this objects in this event
  //these were declared above and are global
  nmu=0;
  mu_e.clear();
  mu_pt.clear();
  mu_px.clear();
  mu_py.clear();
  mu_pz.clear();
  mu_eta.clear();
  mu_phi.clear();
  mu_ch.clear();
  mu_glbtrk_pt.clear();
  mu_glbtrk_eta.clear();
  mu_glbtrk_phi.clear();

//Declare a container (or handle) where to store your muons.
   //https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideDataFormatRecoMuon
  edm::Handle<reco::MuonCollection> mymuons;

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


  //check if the collection is valid
  if(mymuons.isValid()){
      //get the number of muons in the event
      nmu=(*mymuons).size();
	//loop over all the muons in this event
	for (reco::MuonCollection::const_iterator recoMu = mymuons->begin(); recoMu!=mymuons->end(); ++recoMu){
      //find only globlal muons for this specific example
      //https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookMuonAnalysis?rev=88
	  //Note that this would be already a selection cut, i.e.
	  //requiring it to be global is a constrain on what kind of muon it is
      if(recoMu->isGlobalMuon()) {
	  mu_e.push_back(recoMu->energy());
	  mu_pt.push_back(recoMu->pt());
	  mu_px.push_back(recoMu->px());
	  mu_py.push_back(recoMu->py());
	  mu_pz.push_back(recoMu->pz());
	  mu_eta.push_back(recoMu->eta());
	  mu_phi.push_back(recoMu->phi());
	  mu_ch.push_back(recoMu->charge());
	  // get the track combinig the information from both the Tracker and the Spectrometer
	  reco::TrackRef recoCombinedGlbTrack = recoMu->combinedMuon();
	  mu_glbtrk_pt.push_back(recoCombinedGlbTrack->pt());
	  mu_glbtrk_eta.push_back(recoCombinedGlbTrack->eta());
	  mu_glbtrk_phi.push_back(recoCombinedGlbTrack->phi());

	  //here one could apply some identification
	  //cuts to show how to do particle id, and store
	  //refined variables
	}
      else{
	//Here I put default values for those muons that are not global
	//so the containers do not show up as empty. One could do
	//this in a smarter way though.
	mu_e.push_back(-999);
	mu_pt.push_back(-999);
	mu_px.push_back(-999);
	mu_py.push_back(-999);
	mu_pz.push_back(-999);
	mu_eta.push_back(-999);
	mu_phi.push_back(-999);
	mu_ch.push_back(-999);
	mu_glbtrk_pt.push_back(-999);
	mu_glbtrk_eta.push_back(-999);
	mu_glbtrk_phi.push_back(-999);
      }
      }
    }
  
}


// ------------ method called once each job just before starting event loop  ------------
void 
MuonObjectInfoExtractor::beginJob()
{
  //Define storage variables
  myfile = new TFile("MuonObjectInfo.root","RECREATE");
  mytree = new TTree("mytree","Rootuple with object information");
  //point root branches to the right place
  //this is a typical ROOT way of doing it
  //one could of course try to store the information in a different format
  //for exmaple json (nested) format or plain csv 
  //(that would be something nice to implement).
  mytree->Branch("runno",&runno,"runno/I");
  mytree->Branch("evtno",&evtno,"evtno/I");
  mytree->Branch("nmu",&nmu,"nmu/I");
  mytree->Branch("mu_e",&mu_e);
  mytree->Branch("mu_pt",&mu_pt);
  mytree->Branch("mu_px",&mu_px);
  mytree->Branch("mu_py",&mu_py);
  mytree->Branch("mu_pz",&mu_pz);
  mytree->Branch("mu_eta",&mu_eta);
  mytree->Branch("mu_phi",&mu_phi);
  mytree->Branch("mu_ch",&mu_ch);
  mytree->Branch("mu_glbtrk_pt",&mu_glbtrk_pt);
  mytree->Branch("mu_glbtrk_eta",&mu_glbtrk_eta);
  mytree->Branch("mu_glbtrk_phi",&mu_glbtrk_phi);


  
}

// ------------ method called once each job just after ending the event loop  ------------
void 
MuonObjectInfoExtractor::endJob() 
{

  //save file
  myfile->Write();

}

// ------------ method called when starting to processes a run  ------------
void 
MuonObjectInfoExtractor::beginRun(edm::Run const&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a run  ------------
void 
MuonObjectInfoExtractor::endRun(edm::Run const&, edm::EventSetup const&)
{
}

// ------------ method called when starting to processes a luminosity block  ------------
void 
MuonObjectInfoExtractor::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a luminosity block  ------------
void 
MuonObjectInfoExtractor::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
MuonObjectInfoExtractor::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(MuonObjectInfoExtractor);
