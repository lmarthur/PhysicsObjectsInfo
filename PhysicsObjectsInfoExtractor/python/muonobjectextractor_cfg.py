import FWCore.ParameterSet.Config as cms

process = cms.Process("muonext")

process.load("FWCore.MessageService.MessageLogger_cfi")

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(100) )

process.source = cms.Source("PoolSource",
    # replace 'myfile.root' with the source file you want to use
    fileNames = cms.untracked.vstring(
'root://eospublic.cern.ch//eos/opendata/cms/Run2011A/DoubleMu/AOD/12Oct2013-v1/10000/000D143E-9535-E311-B88B-002618943934.root',
        'root://eospublic.cern.ch//eos/opendata/cms/Run2011A/ElectronHad/AOD/12Oct2013-v1/20001/001F9231-F141-E311-8F76-003048F00942.root'
#        ' file:reco.root'
    )
)

process.muonextractor = cms.EDAnalyzer('MuonObjectInfoExtractor',
#change the input collection to other like cosmic muons, for instance
InputCollection = cms.InputTag("muons")
)


process.p = cms.Path(process.muonextractor)
