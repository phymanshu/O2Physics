//  TrackTuner.cxx
//  Objective of this struct/file
//  -
//  - Input:   ?
//      -
//  - Output:  ?

//

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
#include <TGraph.h>

using namespace o2;
using namespace o2::framework;

//
struct trackTuner {

  TGraph* grDcaXYResVsPtPionCurrent;
  TGraph* grDcaXYResVsPtPionUpgrded;

  TGraph* grDcaZResVsPtPionCurrent;
  TGraph* grDcaZResVsPtPionUpgrded;

  TGraph* grDcaXYMeanVsPtPionCurrent;
  TGraph* grDcaXYMeanVsPtPionUpgrded;

  TGraph* grDcaZMeanVsPtPionCurrent;
  TGraph* grDcaZMeanVsPtPionUpgrded;

  TGraph* grOneOverPtPionCurrent;
  TGraph* grOneOverPtPionUpgrded;

  void getDcaGraphs(std::string pathCurrFile, std::string pathUpgrFile)
  {

    const TString fnameDCAFileCurr = pathCurrFile.data();
    const TString fnameDCAFileUpgr = pathUpgrFile.data();

    if ((fnameDCAFileCurr == "") | (fnameDCAFileUpgr == "")) {
      LOG(fatal) << "No Correction DCA files provided!";
      return;
    }

    std::unique_ptr<TFile> fileCurr(TFile::Open(fnameDCAFileCurr.Data(), "READ"));
    std::unique_ptr<TFile> fileUpgr(TFile::Open(fnameDCAFileUpgr.Data(), "READ"));

    grDcaXYResVsPtPionCurrent = (TGraph*)fileCurr->Get("D0RPResPi");
    grDcaXYResVsPtPionUpgrded = (TGraph*)fileUpgr->Get("D0RPResPi");

    grDcaZResVsPtPionCurrent = (TGraph*)fileCurr->Get("D0ZResPi");
    grDcaZResVsPtPionUpgrded = (TGraph*)fileUpgr->Get("D0ZResPi");

    grOneOverPtPionCurrent = (TGraph*)fileCurr->Get("Pt1ResPi");
    grOneOverPtPionUpgrded = (TGraph*)fileUpgr->Get("Pt1ResPi");
  }

  // template <typename T>
  // void init(T ccdb){
  //     // what should go here
  //     // reads the inputs objects and initiations e.g. graphs,...
  // }

  // tuneTrackParams() similar to -> SmearTrack()
  template <typename T1, typename T2, typename T3, typename T4>
  void tuneTrackParams(T1 mcparticle, T2& trackParCov, T3 matCorr, T4 dcaInfoCov)
  {
    /*
    // WIP
    // !!!!  WIP block start !!!!!!
    // ------------------------------
    // some important variables to be used for tuning

    // track obj for MC
    // e.g.   AliExternalTrackParam mct(mcx,mcp,mccv,mcc); in AliPhy
    //

    // Double_t ptMC = TMath::Abs(mcparticle.pt());
    // Double_t phiMC = mcparticle.phi();

    // Int_t phiBin = getPhiBin(phiMC);


    // bz ???? to be passed
    // Int_t MagFieldPolarity = 0;
    // if(bz < 0.) MagFieldPolarity = 0;
    // if(bz > 0.) MagFieldPolarity = 1;

    Double_t DcaXYResCurrent = 0.0;      // sd0rpo=0.;
    Double_t DcaZResCurrent = 0.0;       // sd0zo =0.;

    Double_t DcaXYResUpgrded = 0.0;      // sd0rpn=0.;
    Double_t DcaZResUpgrded = 0.0;       // sd0zn =0.;

    Double_t OneOverPtCurrent = 0.0;     // spt1o =0.;
    Double_t OneOverPtUpgrded = 0.0;     // spt1n =0.;

    Double_t DcaXYMeanCurrent = 0.0;     // sd0mrpo=0.;
    Double_t DcaXYMeanUpgrded = 0.0;     // sd0mrpn=0.;

    // Double_t correctionForPulls = 1.0;      // pullcorr=1.;

    DcaXYResCurrent  = EvalGraph(ptMC, grDcaXYResVsPtPionCurrent);
    DcaXYResUpgrded  = EvalGraph(ptMC, grDcaXYResVsPtPionUpgrded);
    DcaZResCurrent   = EvalGraph(ptMC, grDcaZResVsPtPionCurrent );
    DcaZResUpgrded   = EvalGraph(ptMC, grDcaZResVsPtPionUpgrded );
    OneOverPtCurrent = EvalGraph(ptMC, grOneOverPtPionCurrent );
    OneOverPtUpgrded = EvalGraph(ptMC, grOneOverPtPionUpgrded );

    // DcaXYMeanCurrent = EvalGraph(ptMC, grDcaXYMeanVsPtPionCurrent );
    // DcaXYMeanUpgrded = EvalGraph(ptMC, grDcaXYMeanVsPtPionUpgrded );

    //  Unit conversion, is it required ??
    DcaXYResCurrent*=1.e-4;
    DcaZResCurrent*=1.e-4;

    DcaXYResUpgrded*=1.e-4;
    DcaZResUpgrded*=1.e-4;

    DcaXYMeanCurrent*=1.e-4;
    DcaXYMeanUpgrded*=1.e-4;


    // Apply the smearing
    // ---------------------------------------------
    // Double_t d0zo  =param  [1];
    Double_t trackParDcaZCurrent = trackParCov.getZ();

    // Double_t d0zmc =parammc[1];
    Double_t trackParDcaZMC = 0.0;

    // Double_t d0rpo =param  [0];
    Double_t trackParDcaXYCurrent = trackParCov.getY();

    // Double_t d0rpmc=parammc[0];
    Double_t trackParDcaXYMC =  0.0;

    // Double_t pt1o  =param  [4];
    Double_t trackParOneOverPtCurrent =  trackParCov.getQ2Pt();

    // Double_t pt1mc =parammc[4];
    Double_t trackParOneOverPtMC = 0.0;

    // Double_t dd0zo =d0zo-d0zmc;
    Double_t diffDcaZFromMCCurent = trackParDcaZCurrent - trackParDcaZMC ;

    // Double_t dd0zn =dd0zo *(sd0zo >0. ? (sd0zn /sd0zo ) : 1.);
    Double_t diffDcaZFromMCUpgrded =  diffDcaZFromMCCurent *(DcaZResCurrent > 0. ? (DcaZResUpgrded / DcaZResCurrent ) : 1.);

    // Double_t d0zn  =d0zmc+dd0zn;
    Double_t trackParDcaZUpgrded = trackParDcaZMC + diffDcaZFromMCUpgrded;

    // Double_t dd0rpo=d0rpo-d0rpmc;
    Double_t diffDcaXYFromMCCurent = trackParDcaXYCurrent - trackParDcaXYMC;

    // Double_t dd0rpn=dd0rpo*(sd0rpo>0. ? (sd0rpn/sd0rpo) : 1.);
    Double_t diffDcaXYFromMCUpgrded = diffDcaXYFromMCCurent *(DcaXYResCurrent > 0. ? (DcaXYResUpgrded / DcaXYResCurrent ) : 1.);

    // Double_t dd0mrpn=TMath::Abs(sd0mrpn)-TMath::Abs(sd0mrpo);
    Double_t diffDcaXYMeanUpgMinusCur = TMath::Abs(DcaXYMeanUpgrded) - TMath::Abs(DcaXYMeanCurrent) ;

    // Double_t d0rpn =d0rpmc+dd0rpn-dd0mrpn;
    Double_t trackParDcaXYUpgrded =  diffDcaXYFromMCCurent + diffDcaXYFromMCUpgrded - diffDcaXYMeanUpgMinusCur;


    // option mimic data
    // ----------------------
    // if(fMimicData){
    //     // dd0mrpn=sd0mrpn-sd0mrpo;
    //     diffDcaXYMeanUpgMinusCur = DcaXYMeanUpgrded - DcaXYMeanCurrent;
    //     // d0rpn = d0rpmc+dd0rpn+dd0mrpn;
    //     trackParDcaXYUpgrded =  diffDcaXYFromMCCurent + diffDcaXYFromMCUpgrded + diffDcaXYMeanUpgMinusCur;
    // }


    // update q/pt
    // --------------------------------------
    // Double_t dpt1o =pt1o-pt1mc;
    Double_t diffOneOverPtFromMCCurent = trackParOneOverPtCurrent - trackParOneOverPtMC ;

    // Double_t dpt1n =dpt1o *(spt1o >0. ? (spt1n /spt1o ) : 1.);
    Double_t diffOneOverPtFromMCUpgrded = diffOneOverPtFromMCCurent * (OneOverPtCurrent > 0. ? (OneOverPtUpgrded / OneOverPtCurrent) : 1.);

    // Double_t pt1n  = pt1mc+dpt1n;
    Double_t trackParOneOverPtUpgrded = trackParOneOverPtMC + diffOneOverPtFromMCUpgrded;

    // std::cout << "Inside tuneTrackParams() before modifying trackParCov.getY(): " << trackParCov.getY() << " trackParOneOverPtMC = " << trackParOneOverPtMC << " diffOneOverPtFromMCUpgrded = "  << diffOneOverPtFromMCUpgrded <<  std::endl;

    // setting updated track parameters
    // --------------------------------
    // param[0]=d0rpn;
    Double_t oldDCAxyValue = trackParCov.getY();
    trackParCov.setY(trackParDcaXYUpgrded);
    trackParCov.setY(oldDCAxyValue);
    // param[1]=d0zn ;
    trackParCov.setZ(trackParDcaZUpgrded);
    // param[4]=pt1n ;
    trackParCov.setQ2Pt(trackParOneOverPtUpgrded);


    // update single track cov matrix

  //   if(fUpdateSTCovMatrix){ //
  //       if(sd0rpo>0.)            covar[0]*=(sd0rpn/sd0rpo)*(sd0rpn/sd0rpo);//yy
  //       if(sd0zo>0. && sd0rpo>0.)covar[1]*=(sd0rpn/sd0rpo)*(sd0zn/sd0zo);//yz
  //       if(sd0zo>0.)             covar[2]*=(sd0zn/sd0zo)*(sd0zn/sd0zo);//zz
  //       if(sd0rpo>0.)            covar[3]*=(sd0rpn/sd0rpo);//yl
  //       if(sd0zo>0.)             covar[4]*=(sd0zn/sd0zo);//zl
  //       if(sd0rpo>0.)            covar[6]*=(sd0rpn/sd0rpo);//ysenT
  //       if(sd0zo>0.)             covar[7]*=(sd0zn/sd0zo);//zsenT
  //       if(sd0rpo>0. && spt1o>0.)covar[10]*=(sd0rpn/sd0rpo)*(spt1n/spt1o);//ypt
  //       if(sd0zo>0. && spt1o>0.) covar[11]*=(sd0zn/sd0zo)*(spt1n/spt1o);//zpt
  //       if(spt1o>0.)             covar[12]*=(spt1n/spt1o);//sinPhipt
  //       if(spt1o>0.)             covar[13]*=(spt1n/spt1o);//tanTpt
  //       if(spt1o>0.)             covar[14]*=(spt1n/spt1o)*(spt1n/spt1o);//ptpt
  // }


    // !!!!  WIP block finish !!!!!!
    // ------------------------------
    */
    // define
    // o2::dataformats::DCA dcaInfoCov;
    std::cout << "Inside tuneTrackParams() before modifying trackParCov.getY(): " << trackParCov.getY() << " Pt before: " << 1. / trackParCov.getQ2Pt() << std::endl;
    o2::dataformats::VertexBase vtxMC;
    // vtxMC.setPos({mcparticle.vx(), mcparticle.vy(), mcparticle.vz()});
    // vtxMC.setCov(1, 0, 1, 0, 0, 1); // ??? or All ZEROs // == 1 cm2? wrt prop point
    // o2::base::Propagator::Instance()->propagateToDCABxByBz(vtxMC, trackParCov, 2.f, matCorr, dcaInfoCov);
    // trackParCov.setY(trackParCov.getY()*1.01-0.006);
    trackParCov.setZ(trackParCov.getZ() + 0.05);
    trackParCov.setY(trackParCov.getY() + 0.05);
    std::cout << "Inside tuneTrackParams() after modifying trackParCov.getY(): " << trackParCov.getY() << " Pt after:  " << 1. / trackParCov.getQ2Pt() << std::endl;
  }

  // to be declared
  // ---------------
  Int_t getPhiBin(Double_t phi) const
  {
    Double_t pi = TMath::Pi();
    if (phi > 2. * pi || phi < 0.)
      return -1;
    if ((phi <= (pi / 4.)) || (phi > 7. * (pi / 4.)))
      return 0;
    if ((phi > (pi / 4.)) && (phi <= 3. * (pi / 4.)))
      return 1;
    if ((phi > 3. * (pi / 4.)) && (phi <= 5. * (pi / 4.)))
      return 2;
    if ((phi > (5. * pi / 4.)) && (phi <= 7. * (pi / 4.)))
      return 3;

    return -1;
  }

  Double_t EvalGraph(Double_t x, const TGraph* graph) const
  {
    //
    // Evaluates a TGraph without linear extrapolation. Instead the last
    // valid point of the graph is used when out of range.
    // The function assumes an ascending order of X.
    //

    if (!graph) {
      printf("\tEvalGraph fails !\n");
      return 0.;
    }

    Int_t nPoints = graph->GetN();
    Double_t xMin = graph->GetX()[0];
    Double_t xMax = graph->GetX()[nPoints - 1];
    if (x > xMax)
      return graph->Eval(xMax);
    if (x < xMin)
      return graph->Eval(xMin);

    return graph->Eval(x);
  }
};
