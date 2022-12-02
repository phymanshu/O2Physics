// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

///
/// \file   lfTPCPID.cxx
/// \author Nicolò Jacazio nicolo.jacazio@cern.ch
/// \since  2022-11-20
/// \brief  Task to produce the PID information for the TPC for the purpose of the Light flavor PWG
///

// ROOT includes
#include "TFile.h"
#include "TSystem.h"

// O2 includes
#include "CCDB/BasicCCDBManager.h"
#include "ReconstructionDataFormats/Track.h"
#include "CCDB/CcdbApi.h"
#include "PWGLF/DataModel/LFParticleIdentification.h"
#include "Common/DataModel/TrackSelectionTables.h"
#include "Framework/AnalysisTask.h"
#include "Framework/AnalysisDataModel.h"
#include "Framework/runDataProcessing.h"
#include "Framework/HistogramRegistry.h"
#include "Framework/RunningWorkflowInfo.h"
#include "Framework/StaticFor.h"
#include "DataFormatsTPC/BetheBlochAleph.h"
#include "TableHelper.h"

using namespace o2;
using namespace o2::framework;
using namespace o2::framework::expressions;
using namespace o2::track;

static constexpr int nSpecies = 9;
static constexpr int nParameters = 11;
static const std::vector<std::string> particleNames{"El", "Mu", "Pi", "Ka", "Pr", "De", "Tr", "He", "Al"};
static const std::vector<std::string> parameterNames{"Use default tiny",
                                                     "Use default full",
                                                     "Set parameters",
                                                     "bb1", "bb2", "bb3", "bb4", "bb5",
                                                     "MIP value", "Charge exponent", "Resolution"};
static constexpr float defaultParameters[nSpecies][nParameters]{};

/// Task to produce the response table
struct lfTpcPid {
  using Trks = soa::Join<aod::TracksIU, aod::TracksExtra>;
  using Coll = aod::Collisions;

  // Tables to produce
  Produces<o2::aod::pidTPCLfEl> tablePIDEl;
  Produces<o2::aod::pidTPCLfMu> tablePIDMu;
  Produces<o2::aod::pidTPCLfPi> tablePIDPi;
  Produces<o2::aod::pidTPCLfKa> tablePIDKa;
  Produces<o2::aod::pidTPCLfPr> tablePIDPr;
  Produces<o2::aod::pidTPCLfDe> tablePIDDe;
  Produces<o2::aod::pidTPCLfTr> tablePIDTr;
  Produces<o2::aod::pidTPCLfHe> tablePIDHe;
  Produces<o2::aod::pidTPCLfAl> tablePIDAl;

  Produces<o2::aod::pidTPCLfFullEl> tablePIDFullEl;
  Produces<o2::aod::pidTPCLfFullMu> tablePIDFullMu;
  Produces<o2::aod::pidTPCLfFullPi> tablePIDFullPi;
  Produces<o2::aod::pidTPCLfFullKa> tablePIDFullKa;
  Produces<o2::aod::pidTPCLfFullPr> tablePIDFullPr;
  Produces<o2::aod::pidTPCLfFullDe> tablePIDFullDe;
  Produces<o2::aod::pidTPCLfFullTr> tablePIDFullTr;
  Produces<o2::aod::pidTPCLfFullHe> tablePIDFullHe;
  Produces<o2::aod::pidTPCLfFullAl> tablePIDFullAl;

  // Input parameters
  Service<o2::ccdb::BasicCCDBManager> ccdb;

  // Parameters setting from json
  Configurable<LabeledArray<float>> bbParameters{"bbParameters",
                                                 {defaultParameters[0], nSpecies, nParameters, particleNames, parameterNames},
                                                 "Bethe Bloch parameters"};
  // Parameter setting from input file (including the ccdb)
  Configurable<std::string> fileParamBbEl{"filePparamBbEl",
                                          "",
                                          "Parameters for the Bethe-Bloch parametrization for electrons. Input file, if empty using the default values, priority over the json configuration. Can be a CCDB path if the string starts with ccdb://"};
  Configurable<std::string> fileParamBbMu{"filePparamBbMu",
                                          "",
                                          "Parameters for the Bethe-Bloch parametrization for muons. Input file, if empty using the default values, priority over the json configuration. Can be a CCDB path if the string starts with ccdb://"};
  Configurable<std::string> fileParamBbPi{"filePparamBbPi",
                                          "",
                                          "Parameters for the Bethe-Bloch parametrization for pions. Input file, if empty using the default values, priority over the json configuration. Can be a CCDB path if the string starts with ccdb://"};
  Configurable<std::string> fileParamBbKa{"filePparamBbKa",
                                          "",
                                          "Parameters for the Bethe-Bloch parametrization for kaons. Input file, if empty using the default values, priority over the json configuration. Can be a CCDB path if the string starts with ccdb://"};
  Configurable<std::string> fileParamBbPr{"filePparamBbPr",
                                          "",
                                          "Parameters for the Bethe-Bloch parametrization for protons. Input file, if empty using the default values, priority over the json configuration. Can be a CCDB path if the string starts with ccdb://"};
  Configurable<std::string> fileParamBbDe{"filePparamBbDe",
                                          "",
                                          "Parameters for the Bethe-Bloch parametrization for deuterons. Input file, if empty using the default values, priority over the json configuration. Can be a CCDB path if the string starts with ccdb://"};
  Configurable<std::string> fileParamBbTr{"filePparamBbTr",
                                          "",
                                          "Parameters for the Bethe-Bloch parametrization for tritons. Input file, if empty using the default values, priority over the json configuration. Can be a CCDB path if the string starts with ccdb://"};
  Configurable<std::string> fileParamBbHe{"filePparamBbHe",
                                          "",
                                          "Parameters for the Bethe-Bloch parametrization for helium3. Input file, if empty using the default values, priority over the json configuration. Can be a CCDB path if the string starts with ccdb://"};
  Configurable<std::string> fileParamBbAl{"filePparamBbAl",
                                          "",
                                          "Parameters for the Bethe-Bloch parametrization for helium4. Input file, if empty using the default values, priority over the json configuration. Can be a CCDB path if the string starts with ccdb://"};

  Configurable<std::string> url{"ccdb-url", "http://alice-ccdb.cern.ch", "url of the ccdb repository"};
  Configurable<std::string> ccdbPath{"ccdbPath", "Analysis/PID/TPC/Response", "Path of the TPC parametrization on the CCDB"};
  Configurable<int64_t> ccdbTimestamp{"ccdb-timestamp", -1, "timestamp of the object used to query in CCDB the detector response"};

  struct bbParams {
    float bb1 = 0.03209809958934784f;    // Aleph Bethel Bloch parameter 1
    float bb2 = 19.9768009185791f;       // Aleph Bethe Bloch parameter 2
    float bb3 = 2.5266601063857674e-16f; // Aleph Bethe Bloch parameter 3
    float bb4 = 2.7212300300598145f;     // Aleph Bethe Bloch parameter 4
    float bb5 = 6.080920219421387f;      // Aleph Bethe Bloch parameter 5
    float mip = 50.f;                    // MIP value
    float exp = 2.299999952316284f;      // Exponent of the charge factor
    float res = 0.002f;                  // Resolution

    bool setValues(std::vector<float> v)
    {
      if (v.size() != 8) {
        LOG(error) << "The vector of Bethe-Bloch parameters has the wrong size";
        return false;
      }
      LOG(info) << "Before: set of parameters -> bb1: " << bb1 << ", bb2: " << bb2 << ", bb3: " << bb3 << ", bb4: " << bb4 << ", bb5: " << bb5 << ", mip: " << mip << ", exp: " << exp << ", " << res;
      bb1 = v[0];
      bb2 = v[1];
      bb3 = v[2];
      bb4 = v[3];
      bb5 = v[4];
      mip = v[5];
      exp = v[6];
      res = v[7];
      LOG(info) << "After: set of parameters -> bb1: " << bb1 << ", bb2: " << bb2 << ", bb3: " << bb3 << ", bb4: " << bb4 << ", bb5: " << bb5 << ", mip: " << mip << ", exp: " << exp << ", " << res;
      return true;
    }

    bool setValues(const char* particle, const Configurable<LabeledArray<float>>& p)
    {
      if (p->get(particle, "Set parameters") < 1.5f) {
        LOG(info) << "Using default for " << particle << " input vector size " << p->get(particle, "Set parameters") << " < 1.5";
        return false;
      }
      std::vector<float> v{p->get(particle, "bb1"),
                           p->get(particle, "bb2"),
                           p->get(particle, "bb3"),
                           p->get(particle, "bb4"),
                           p->get(particle, "bb5"),
                           p->get(particle, "MIP value"),
                           p->get(particle, "Charge exponent"),
                           p->get(particle, "Resolution")};
      LOG(info) << "Setting custom Bethe-Bloch parameters for mass hypothesis " << particle;
      return setValues(v);
    }

    bool setValues(TH1F* h)
    {
      const int n = h->GetNbinsX();
      TAxis* axis = h->GetXaxis();
      std::vector<float> v{static_cast<float>(h->GetBinContent(axis->FindBin("bb1"))),
                           static_cast<float>(h->GetBinContent(axis->FindBin("bb2"))),
                           static_cast<float>(h->GetBinContent(axis->FindBin("bb3"))),
                           static_cast<float>(h->GetBinContent(axis->FindBin("bb4"))),
                           static_cast<float>(h->GetBinContent(axis->FindBin("bb5"))),
                           static_cast<float>(h->GetBinContent(axis->FindBin("MIP value"))),
                           static_cast<float>(h->GetBinContent(axis->FindBin("Charge exponent"))),
                           static_cast<float>(h->GetBinContent(axis->FindBin("Resolution")))};
      if (h->GetNbinsX() != n) {
        LOG(error) << "The input histogram of Bethe-Bloch parameters has the wrong size " << n << " while expecting " << h->GetNbinsX();
        return false;
      }
      LOG(info) << "Setting custom Bethe-Bloch parameters from histogram " << h->GetName();
      return setValues(v);
    }

    bool setValues(TFile* f)
    {
      TH1F* h = nullptr;
      f->GetObject("hpar", h);
      if (!h) {
        LOG(error) << "The input file does not contain the histogram hpar";
        return false;
      }
      LOG(info) << "Setting parameters from file " << f->GetName();
      return setValues(h);
    }

    bool setValues(const Configurable<std::string>& cfg, o2::framework::Service<o2::ccdb::BasicCCDBManager> ccdbObj)
    {
      if (cfg.value.size() <= 1) {
        return false;
      }
      LOG(info) << "Loading parameters " << cfg.name << " from file " << cfg.value;
      std::string s = cfg.value;
      if (s.rfind("ccdb://", 0) == 0) {
        s.replace(0, 7, "");
        TH1F* h = ccdbObj->get<TH1F>(s);
        return setValues(h);
      }
      return setValues(TFile::Open(cfg.value.c_str(), "READ"));
    }

  } bbEl, bbMu, bbPi, bbKa, bbPr, bbDe, bbTr, bbHe, bbAl;

  template <o2::track::PID::ID id, typename T>
  float BetheBlochLf(const T& track, const bbParams& params)
  {
    static constexpr float invmass = 1.f / o2::track::pid_constants::sMasses2Z[id];
    static constexpr float charge = o2::track::pid_constants::sCharges[id];
    return params.mip * o2::tpc::BetheBlochAleph(track.tpcInnerParam() * invmass, params.bb1, params.bb2, params.bb3, params.bb4, params.bb5) * std::pow(charge, params.exp);
  }

  template <typename T>
  float BetheBlochEl(const T& track)
  {
    return BetheBlochLf<o2::track::PID::Electron>(track, bbEl);
  }
  template <typename T>
  float BetheBlochMu(const T& track)
  {
    return BetheBlochLf<o2::track::PID::Muon>(track, bbMu);
  }
  template <typename T>
  float BetheBlochPi(const T& track)
  {
    return BetheBlochLf<o2::track::PID::Pion>(track, bbPi);
  }
  template <typename T>
  float BetheBlochKa(const T& track)
  {
    return BetheBlochLf<o2::track::PID::Kaon>(track, bbKa);
  }
  template <typename T>
  float BetheBlochPr(const T& track)
  {
    return BetheBlochLf<o2::track::PID::Proton>(track, bbPr);
  }
  template <typename T>
  float BetheBlochDe(const T& track)
  {
    return BetheBlochLf<o2::track::PID::Deuteron>(track, bbDe);
  }
  template <typename T>
  float BetheBlochTr(const T& track)
  {
    return BetheBlochLf<o2::track::PID::Triton>(track, bbTr);
  }
  template <typename T>
  float BetheBlochHe(const T& track)
  {
    return BetheBlochLf<o2::track::PID::Helium3>(track, bbHe);
  }
  template <typename T>
  float BetheBlochAl(const T& track)
  {
    return BetheBlochLf<o2::track::PID::Alpha>(track, bbAl);
  }

  template <o2::track::PID::ID id, typename T>
  float BetheBlochResolutionLf(const T& track, const bbParams& params)
  {
    static constexpr float invmass = 1.f / o2::track::pid_constants::sMasses[id];
    static constexpr float charge = o2::track::pid_constants::sCharges[id];

    const float dEdx = BetheBlochLf<id, T>(track, params);
    const float deltaP = params.res * std::sqrt(dEdx);
    const float bgDelta = track.tpcInnerParam() * (1.f + deltaP) * invmass;
    const float dEdx2 = params.mip * o2::tpc::BetheBlochAleph(bgDelta, params.bb1, params.bb2, params.bb3, params.bb4, params.bb5) * std::pow(charge, params.exp);
    return std::abs(dEdx2 - dEdx);
  }

  template <typename T>
  float BetheBlochResEl(const T& track)
  {
    return BetheBlochResolutionLf<o2::track::PID::Electron>(track, bbEl);
  }
  template <typename T>
  float BetheBlochResMu(const T& track)
  {
    return BetheBlochResolutionLf<o2::track::PID::Muon>(track, bbMu);
  }
  template <typename T>
  float BetheBlochResPi(const T& track)
  {
    return BetheBlochResolutionLf<o2::track::PID::Pion>(track, bbPi);
  }
  template <typename T>
  float BetheBlochResKa(const T& track)
  {
    return BetheBlochResolutionLf<o2::track::PID::Kaon>(track, bbKa);
  }
  template <typename T>
  float BetheBlochResPr(const T& track)
  {
    return BetheBlochResolutionLf<o2::track::PID::Proton>(track, bbPr);
  }
  template <typename T>
  float BetheBlochResDe(const T& track)
  {
    return BetheBlochResolutionLf<o2::track::PID::Deuteron>(track, bbDe);
  }
  template <typename T>
  float BetheBlochResTr(const T& track)
  {
    return BetheBlochResolutionLf<o2::track::PID::Triton>(track, bbTr);
  }
  template <typename T>
  float BetheBlochResHe(const T& track)
  {
    return BetheBlochResolutionLf<o2::track::PID::Helium3>(track, bbHe);
  }
  template <typename T>
  float BetheBlochResAl(const T& track)
  {
    return BetheBlochResolutionLf<o2::track::PID::Alpha>(track, bbAl);
  }

  void init(o2::framework::InitContext& initContext)
  {
    if (doprocessEl || doprocessFullEl) {
      LOG(info) << "Enabling Electron";
      bbEl.setValues("El", bbParameters);
      bbEl.setValues(fileParamBbEl, ccdb);
    } else {
      LOG(info) << "Skipping Electron";
      if (isTableRequiredInWorkflow(initContext, "pidTPCLfEl") || isTableRequiredInWorkflow(initContext, "pidTPCLfFullEl")) {
        LOG(fatal) << "Requested Electron table but not enabled in configuration";
      }
    }

    if (doprocessMu || doprocessFullMu) {
      LOG(info) << "Enabling Muon";
      bbMu.setValues("Mu", bbParameters);
      bbMu.setValues(fileParamBbMu, ccdb);
    } else {
      LOG(info) << "Skipping Muon";
      if (isTableRequiredInWorkflow(initContext, "pidTPCLfMu") || isTableRequiredInWorkflow(initContext, "pidTPCLfFullMu")) {
        LOG(fatal) << "Requested Muon table but not enabled in configuration";
      }
    }

    if (doprocessPi || doprocessFullPi) {
      LOG(info) << "Enabling Pion";
      bbPi.setValues("Pi", bbParameters);
      bbPi.setValues(fileParamBbPi, ccdb);
    } else {
      LOG(info) << "Skipping Pion";
      if (isTableRequiredInWorkflow(initContext, "pidTPCLfPi") || isTableRequiredInWorkflow(initContext, "pidTPCLfFullPi")) {
        LOG(fatal) << "Requested Pion table but not enabled in configuration";
      }
    }

    if (doprocessKa || doprocessFullKa) {
      LOG(info) << "Enabling Kaon";
      bbKa.setValues("Ka", bbParameters);
      bbKa.setValues(fileParamBbKa, ccdb);
    } else {
      LOG(info) << "Skipping Kaon";
      if (isTableRequiredInWorkflow(initContext, "pidTPCLfKa") || isTableRequiredInWorkflow(initContext, "pidTPCLfFullKa")) {
        LOG(fatal) << "Requested Kaon table but not enabled in configuration";
      }
    }

    if (doprocessPr || doprocessFullPr) {
      LOG(info) << "Enabling Proton";
      bbPr.setValues("Pr", bbParameters);
      bbPr.setValues(fileParamBbPr, ccdb);
    } else {
      LOG(info) << "Skipping Proton";
      if (isTableRequiredInWorkflow(initContext, "pidTPCLfPr") || isTableRequiredInWorkflow(initContext, "pidTPCLfFullPr")) {
        LOG(fatal) << "Requested Proton table but not enabled in configuration";
      }
    }

    if (doprocessDe || doprocessFullDe) {
      LOG(info) << "Enabling Deuteron";
      bbDe.setValues("De", bbParameters);
      bbDe.setValues(fileParamBbDe, ccdb);
    } else {
      LOG(info) << "Skipping Deuteron";
      if (isTableRequiredInWorkflow(initContext, "pidTPCLfDe") || isTableRequiredInWorkflow(initContext, "pidTPCLfFullDe")) {
        LOG(fatal) << "Requested Deuteron table but not enabled in configuration";
      }
    }

    if (doprocessTr || doprocessFullTr) {
      LOG(info) << "Enabling Triton";
      bbTr.setValues("Tr", bbParameters);
      bbTr.setValues(fileParamBbTr, ccdb);
    } else {
      LOG(info) << "Skipping Triton";
      if (isTableRequiredInWorkflow(initContext, "pidTPCLfTr") || isTableRequiredInWorkflow(initContext, "pidTPCLfFullTr")) {
        LOG(fatal) << "Requested Triton table but not enabled in configuration";
      }
    }

    if (doprocessHe || doprocessFullHe) {
      LOG(info) << "Enabling Helium3";
      bbHe.setValues("He", bbParameters);
      bbHe.setValues(fileParamBbHe, ccdb);
    } else {
      LOG(info) << "Skipping Helium3";
      if (isTableRequiredInWorkflow(initContext, "pidTPCLfHe") || isTableRequiredInWorkflow(initContext, "pidTPCLfFullHe")) {
        LOG(fatal) << "Requested Helium3 table but not enabled in configuration";
      }
    }

    if (doprocessAl || doprocessFullAl) {
      bbAl.setValues("Al", bbParameters);
      bbAl.setValues(fileParamBbAl, ccdb);
    } else {
      LOG(info) << "Skipping Alpha";
      if (isTableRequiredInWorkflow(initContext, "pidTPCLfAl") || isTableRequiredInWorkflow(initContext, "pidTPCLfFullAl")) {
        LOG(fatal) << "Requested Alpha table but not enabled in configuration";
      }
    }

    // Get the parameters
    const std::string path = ccdbPath.value;
    const auto ts = ccdbTimestamp.value;
    ccdb->setURL(url.value);
    ccdb->setCaching(true);
    ccdb->setLocalObjectValidityChecking();
    ccdb->setCreatedNotAfter(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    if (ts != 0) {
      LOGP(info, "Initialising LF TPC PID response for fixed timestamp {}:", ts);
      ccdb->setTimestamp(ts);
    }
  }

#define makeProcess(Particle)                                                                                                               \
  void process##Particle(Coll const& collisions,                                                                                            \
                         soa::Join<Trks, aod::pidTPC##Particle> const& tracks,                                                              \
                         aod::BCsWithTimestamps const&)                                                                                     \
  {                                                                                                                                         \
    LOG(debug) << "Filling table for particle: " << #Particle;                                                                              \
    tablePID##Particle.reserve(tracks.size());                                                                                              \
    if (bbParameters->get(#Particle, "Use default tiny") >= 1.5f) {                                                                         \
      for (auto const& trk : tracks) {                                                                                                      \
        tablePID##Particle(trk.tpcNSigmaStore##Particle());                                                                                 \
      }                                                                                                                                     \
    } else {                                                                                                                                \
      for (auto const& trk : tracks) {                                                                                                      \
        aod::pidutils::packInTable<aod::pidtpc_tiny::binning>((trk.tpcSignal() - BetheBloch##Particle(trk)) / BetheBlochRes##Particle(trk), \
                                                              tablePID##Particle);                                                          \
      }                                                                                                                                     \
    }                                                                                                                                       \
  }                                                                                                                                         \
  PROCESS_SWITCH(lfTpcPid, process##Particle, "Produce a table for the " #Particle " hypothesis", false);

  makeProcess(El);
  makeProcess(Mu);
  makeProcess(Pi);
  makeProcess(Ka);
  makeProcess(Pr);
  makeProcess(De);
  makeProcess(Tr);
  makeProcess(He);
  makeProcess(Al);

#undef makeProcess

// Full tables
#define makeProcess(Particle)                                                                                 \
  void processFull##Particle(Coll const& collisions,                                                          \
                             soa::Join<Trks, aod::pidTPCFull##Particle> const& tracks,                        \
                             aod::BCsWithTimestamps const&)                                                   \
  {                                                                                                           \
    LOG(debug) << "Filling table for particle: " << #Particle;                                                \
    tablePIDFull##Particle.reserve(tracks.size());                                                            \
    if (bbParameters->get(#Particle, "Use default full") >= 1.5f) {                                           \
      for (auto const& trk : tracks) {                                                                        \
        tablePIDFull##Particle(trk.tpcExpSigma##Particle(), trk.tpcNSigma##Particle());                       \
      }                                                                                                       \
    } else {                                                                                                  \
      for (auto const& trk : tracks) {                                                                        \
        tablePIDFull##Particle(BetheBlochRes##Particle(trk),                                                  \
                               (trk.tpcSignal() - BetheBloch##Particle(trk)) / BetheBlochRes##Particle(trk)); \
      }                                                                                                       \
    }                                                                                                         \
  }                                                                                                           \
  PROCESS_SWITCH(lfTpcPid, processFull##Particle, "Produce a full table for the " #Particle " hypothesis", false);

  makeProcess(El);
  makeProcess(Mu);
  makeProcess(Pi);
  makeProcess(Ka);
  makeProcess(Pr);
  makeProcess(De);
  makeProcess(Tr);
  makeProcess(He);
  makeProcess(Al);

#undef makeProcess
};

WorkflowSpec defineDataProcessing(ConfigContext const& cfgc)
{
  return WorkflowSpec{adaptAnalysisTask<lfTpcPid>(cfgc)};
}
