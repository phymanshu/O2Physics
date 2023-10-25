//  Name of this Task and  struct: Improver OR TrackCalibrator.cxx OR TrackTuner.cxx (TBD)
//  Objective of this struct/file
//  - This is a helper task of trackPropagation in case if we use improver function on MC
//  -
//  - Input:   ?
//      -
//  - Output:  ?

// Rename it: *.h

#include "Framework/AnalysisDataModel.h"
#include "Framework/AnalysisTask.h"
#include "Framework/runDataProcessing.h"
#include "Framework/RunningWorkflowInfo.h"
#include "Common/DataModel/TrackSelectionTables.h"
#include "Common/Core/trackUtilities.h"
#include "ReconstructionDataFormats/DCA.h"
#include "DetectorsBase/Propagator.h"
#include "DetectorsBase/GeometryManager.h"
#include "CommonUtils/NameConf.h"
#include "CCDB/CcdbApi.h"
#include "DataFormatsParameters/GRPMagField.h"
#include "CCDB/BasicCCDBManager.h"
#include "Framework/HistogramRegistry.h"
#include "DataFormatsCalibration/MeanVertexObject.h"
#include "CommonConstants/GeomConstants.h"

using namespace o2;
using namespace o2::framework;

//
struct trackTuner {
  // Service<o2::ccdb::BasicCCDBManager> ccdb;

  std::string fMcFileName; // this one?? ok
  TString fDataFileName;

  TString fMcGraphDcaMeanXY;
  TString fMcGraphDcaMeanZ;
  TString fDataGraphDcaMeanXY;
  TString fDataGraphDcaMeanZ;

  TString fMcGraphDcaRmsXY;
  TString fMcGraphDcaRmsZ;
  TString fDataGraphDcaRmsXY;
  TString fDataGraphDcaRmsZ;

  // Getters
  // void getMcFileName()    { return fMcFileName;}
  // void getDataFileName()  { return fDataFileName;}

  // // Setters
  // void setMcFileName(TString mcFileName)      { fMcFileName = mcFileName.Data();}
  // void setDataFileName(TString dataFileName)    { fDataFileName = dataFileName.Data();}

  // TFile* getMcFile(DcaObjectLoader& fileLoader) {
  //         TFile *file = TFile::Open(fileLoader.fMcFileName);
  //         return file;
  //     }
  //
  // TFile* getDataFile(DcaObjectLoader& fileLoader) {
  //         TFile *file = TFile::Open(fileLoader.fDataFileName);
  //         return file;
  //     }
  //
  // TGraphError *getGraphObject(TFile *file, TString objName) {
  //         TGraphError *gr = (TGraphError*)file->FindObject(objName.Data());
  //         return gr;
  //     }
  //
  // void init() {
  //         DcaFileLoader fileLoader;
  //
  //         TFile *mcFile = (TFile*) getFile(fileLoader.mcFileName);
  //         TFile *dataFile = (TFile*) getFile(fileLoader.dataFileName);
  //
  //     }
  template <typename T>
  void init(T ccdb)
  {
    // what should go here
    // reads the inputs objects and initiations e.g. graphs,...
  }
  // ...
  template <typename T1, typename T2, typename T3, typename T4>
  void tuneTrackParams(T1 mcparticle, T2 trackParCov, T3 matCorr, T4 dcaInfoCov)
  {
    // define
    // o2::dataformats::DCA dcaInfoCov;
    o2::dataformats::VertexBase vtxMC;

    vtxMC.setPos({mcparticle.vx(), mcparticle.vy(), mcparticle.vz()});
    vtxMC.setCov(1, 0, 1, 0, 0, 1); // ??? or All ZEROs // == 1 cm2? wrt prop point
    o2::base::Propagator::Instance()->propagateToDCABxByBz(vtxMC, trackParCov, 2.f, matCorr, dcaInfoCov);
  }
};

// 1)
//