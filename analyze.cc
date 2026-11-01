#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TMath.h"
#include <iostream>
#include <map>
#include <string>
#include <vector>

struct HistConfig {
    int bins;
    double low;
    double high;
};

std::map<std::string, TH1D> createHistograms(){
    std::map<std::string, TH1D> histograms;

    const std::map<std::string, HistConfig> config = {
        {"hN_",   {400, 0.0, 400.0}},
        {"hPt_", {100, 0, 10}},
        {"hEta_", {100,-5,5}},
        {"hPhi_", {64,0,TMath::Pi()}}
    };

    const std::vector<std::pair<std::string, std::string>> categories = {
        {"all", "all particles"},
        {"pip", "#pi^{+}"},
        {"pim", "#pi^{-}"},
        {"Kp", "K^{+}"},
        {"Km", "K^{-}"},
        {"p", "p"},
        {"pbar", "#bar{p}"},
        {"ch", "charged particles"},
    };
    const std::vector<std::pair<std::string, std::string>> datatypes = {
        {"hN_", "Multiplicity"},
        {"hPt_", "pT"},
        {"hEta_", "#eta"},
        {"hPhi_", "#phi"},
    };


    for(const auto& datatype : datatypes){
        for (const auto& category : categories){
            std::string name = datatype.first + category.first;
            std::string title = datatype.second + " distribution of " + category.second;
            
            const HistConfig& cfg = config.at(datatype.first);

            TH1D histogram(
                name.c_str(),
                title.c_str(),
                cfg.bins,
                cfg.low,
                cfg.high);
            histogram.SetDirectory(nullptr);
            histograms.emplace(name, std::move(histogram));
        }
    }
    return histograms;
}

struct EventCounts {
    int nCh = 0;
    int nPip = 0;
    int nPim = 0;
    int nKp = 0;
    int nKm = 0;
    int nP = 0;
    int nPbar = 0;
};

void fill_histogram(
    std::map<std::string, TH1D>& histograms,
    EventCounts& counts,
    double pt,
    double eta,
    double phi,
    int pdgId,
    bool charged
){
    histograms["hPt_all"].Fill(pt);
    histograms["hEta_all"].Fill(eta);
    histograms["hPhi_all"].Fill(phi);

    if (charged) {
        ++counts.nCh;
        histograms["hPt_ch"].Fill(pt);
        histograms["hEta_ch"].Fill(eta);
        histograms["hPhi_ch"].Fill(phi);
    }

    if (pdgId == 211) {
        ++counts.nPip;
        histograms["hPt_pip"].Fill(pt);
        histograms["hEta_pip"].Fill(eta);
        histograms["hPhi_pip"].Fill(phi);
    } else if (pdgId == -211) {
        ++counts.nPim;
        histograms["hPt_pim"].Fill(pt);
        histograms["hEta_pim"].Fill(eta);
        histograms["hPhi_pim"].Fill(phi);
    } else if (pdgId == 321) {
        ++counts.nKp;
        histograms["hPt_Kp"].Fill(pt);
        histograms["hEta_Kp"].Fill(eta);
        histograms["hPhi_Kp"].Fill(phi);
    } else if (pdgId == -321) {
        ++counts.nKm;
        histograms["hPt_Km"].Fill(pt);
        histograms["hEta_Km"].Fill(eta);
        histograms["hPhi_Km"].Fill(phi);
    } else if (pdgId == 2212) {
        ++counts.nP;
        histograms["hPt_p"].Fill(pt);
        histograms["hEta_p"].Fill(eta);
        histograms["hPhi_p"].Fill(phi);
    } else if (pdgId == -2212) {
        ++counts.nPbar;
        histograms["hPt_pbar"].Fill(pt);
        histograms["hEta_pbar"].Fill(eta);
        histograms["hPhi_pbar"].Fill(phi);
    }
}
int main(){

    TFile in("event.root", "READ");

    TTree* tree = nullptr;
    in.GetObject("events", tree);

    if (!tree) {
        std::cerr << "Cannot find tree events\n";
        return 1;
    }

    std::vector<double>* pt = nullptr;
    std::vector<double>* eta = nullptr;
    std::vector<double>* phi = nullptr;
    std::vector<int>* pdgId = nullptr;
    std::vector<bool>* isCharged = nullptr;

    const char* requiredBranches[] = {"pt", "eta", "phi", "pdgID", "isCharged"};
    for (const char* name : requiredBranches) {
        if (!tree->GetBranch(name)) {
            std::cerr << "Missing branch " << name << "\n";
            return 1;
        }
    }

    if (tree->SetBranchAddress("pt", &pt) < 0
        || tree->SetBranchAddress("eta", &eta) < 0
        || tree->SetBranchAddress("phi", &phi) < 0
        || tree->SetBranchAddress("pdgID", &pdgId) < 0
        || tree->SetBranchAddress("isCharged", &isCharged) < 0) {
        std::cerr << "Cannot read branches from event.root\n";
        return 1;
    }

    Int_t n_entries = tree->GetEntries();

    TH1::AddDirectory(false);
    std::map<std::string, TH1D> histograms = createHistograms();

    for(Int_t i = 0; i < n_entries;i++){

        tree -> GetEntry(i);

        if (!pt || !eta || !phi || !pdgId || !isCharged
            || eta->size() != pt->size()
            || phi->size() != pt->size()
            || pdgId->size() != pt->size()
            || isCharged->size() != pt->size()) {
            std::cerr << "Inconsistent vectors in entry " << i << "\n";
            return 1;
        }

        EventCounts counts;

        for (Int_t j = 0; j < pt->size(); ++j) {
            fill_histogram(
                histograms,
                counts,
                pt->at(j),
                eta->at(j),
                phi->at(j),
                pdgId->at(j),
                isCharged->at(j) != 0);
        }

        histograms["hN_all"].Fill(pt->size());
        histograms["hN_ch"].Fill(counts.nCh);
        histograms["hN_pip"].Fill(counts.nPip);
        histograms["hN_pim"].Fill(counts.nPim);
        histograms["hN_Kp"].Fill(counts.nKp);
        histograms["hN_Km"].Fill(counts.nKm);
        histograms["hN_p"].Fill(counts.nP);
        histograms["hN_pbar"].Fill(counts.nPbar);
    }

    TFile out("analysis.root", "RECREATE");

    if (out.IsZombie()) {
        std :: cerr << "Cannot open analysis.root\n";
        return 1;
    }
    for (auto& entry : histograms) {
        entry.second.Write();
    }
    out.Close();

    return 0;
}