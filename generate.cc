#include "Pythia8/Pythia.h"
#include "TFile.h"
#include "TTree.h"
#include <cmath>
#include <vector>
#include <iostream>
#include <string>
using namespace Pythia8;

int main(){

    int n_events = 100000;

    std::vector<double> pt;
    std::vector<double> eta;
    std::vector<double> phi;
    std::vector<bool> isCharged;
    std::vector<int> pdgId;

    Pythia pythia;

    pythia.readString("Beams:idA = 2212");
    pythia.readString("Beams:idB = 2212");
    pythia.readString("Beams:eCM = 13600");
    pythia.readString("SoftQCD:all = on");
    pythia.readString("Random:setSeed = on");
    pythia.readString("Random:seed = 100");

    if (!pythia.init()) {
        std::cerr << "Pythia initialization failed\n";
        return 1;
    }

    TFile out("event.root", "RECREATE");
    if (out.IsZombie()) {
        std::cerr << "Cannot create event.root\n";
        return 1;
    }

    TTree tree("events","pp_collisions_data");
    tree.Branch("pt",&pt);
    tree.Branch("eta",&eta);
    tree.Branch("phi",&phi);
    tree.Branch("isCharged", &isCharged);
    tree.Branch("pdgID", &pdgId);

    for(int i = 0;i < n_events;i++){

        if(pythia.next()) {
            pt.clear();
            eta.clear();
            phi.clear();
            isCharged.clear();
            pdgId.clear();
            
            int n_particles = pythia.event.size();

            for(int j = 1;j < n_particles;j++){
                const Particle& p = pythia.event[j];

                if(!p.isFinal()) continue;

                pdgId.push_back(p.id());
                pt.push_back(p.pT());
                eta.push_back(p.eta());
                phi.push_back(std::fabs(p.phi()));
                isCharged.push_back(p.isCharged());
                
            }

            tree.Fill();
        }

    }

    out.Write();
    out.Close();

    return 0;
}
