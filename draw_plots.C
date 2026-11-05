#include "TCanvas.h"
#include "TFile.h"
#include "TGaxis.h"
#include "TGraph.h"
#include "TH1.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TLegendEntry.h"
#include "TROOT.h"
#include "TString.h"
#include "TStyle.h"
#include "TSystem.h"
#include <cstdio>
#include <cstring>

const char* kCategoryNames[] = {
    "all", "ch", "pip", "pim", "Kp", "Km", "p", "pbar"
};
const int kCategoryColors[] = {
    kBlack,      // all
    kGreen + 2,  // ch
    kRed + 1,    // pip
    kViolet,     // pim
    kBlue,       // Kp
    kOrange + 7, // Km
    kGray + 2,   // p
    kAzure + 7   // pbar
};
const int nCategoryPalette = sizeof(kCategoryNames) / sizeof(kCategoryNames[0]);

/**
    * @brief Determines the line color based on the category encoded in a histogram name.
    *
    * Extracts the category from the substring following the first underscore
    * in the histogram name and matches it against the predefined category palette.
    *
    * @param histName Name of the histogram, expected to contain a category
    *                 identifier after the first underscore.
    * @return Color_t Corresponding color from the category palette if a match
    *                 is found; otherwise, returns kBlack.
    */
Color_t category_line_color(const char* histName){

    const char* underscore = std::strchr(histName, '_');
    if (!underscore || !underscore[1]) {
        return kBlack;
    }
    const char* category = underscore + 1;

    for (int i = 0; i < nCategoryPalette; ++i) {
        if (std::strcmp(category, kCategoryNames[i]) == 0) {
            return static_cast<Color_t>(kCategoryColors[i]);
        }
    }

    return kBlack;
}
/**
    * @brief Configures the visual appearance and axis labels of a histogram.
    *
    * Sets the histogram's line color and width, disables fill, and configures
    * axis titles, title sizes, and offsets. The line color is determined by
    * the category index if provided, or by the histogram name otherwise.
    *
    * The X-axis title is selected based on the histogram name prefix:
    * - "hPt_"  : transverse momentum, pT [GeV]
    * - "hEta_" : pseudorapidity, eta
    * - "hPhi_" : azimuthal angle, phi [rad]
    * - "hN_"   : multiplicity, N
    *
    * @param h Pointer to the TH1D histogram to configure.
    * @param categoryIndex Optional category index used to select the line color.
    *                      If negative, the color is determined from the histogram name.
    */
void prepare_histogram(TH1D* h, int categoryIndex = -1){

    const Color_t lineColor = (categoryIndex >= 0)
        ? static_cast<Color_t>(kCategoryColors[categoryIndex])
        : category_line_color(h->GetName());
    h->SetLineColor(lineColor);
    h->SetLineWidth(2);
    h->SetFillStyle(0);

    h->GetYaxis()->SetTitle("Entries");
    h->GetYaxis()->CenterTitle();
    h->GetXaxis()->SetTitleSize(0.05);
    h->GetYaxis()->SetTitleSize(0.05);
    h->GetXaxis()->SetTitleOffset(1.1);
    h->GetYaxis()->SetTitleOffset(1.25);

    TString histName(h->GetName());
    if (histName.BeginsWith("hPt_")) {
        h->GetXaxis()->SetTitle("p_{T} [GeV]");
    } else if (histName.BeginsWith("hEta_")) {
        h->GetXaxis()->SetTitle("#eta");
    } else if (histName.BeginsWith("hPhi_")) {
        h->GetXaxis()->SetTitle("#phi [rad]");
    } else if (histName.BeginsWith("hN_")) {
        h->GetXaxis()->SetTitle("Multiplicity N");
    }
}


/**
    * @brief Draws and saves an individual histogram plot.
    * 
    * Creates a canvas for the given histogram, draws it with appropriate
    * axis settings, and saves the resulting plot as PDF and PNG files.
    *
    * The canvas margins and y-axis exponent offset are configured for
    * consistent plot formatting. Histograms whose names begin with "hN"
    * or "hPt" are displayed with a logarithmic y-axis, while histograms
    * beginning with "hPhi_" have their minimum set to zero.
    *
    * @param h Pointer to the TH1D histogram to be drawn and exported.
    */
void make_and_extract_canvas(TH1D* h){

    const char* name = h->GetName();

    TCanvas canvas(Form("c_%s", name), name, 800, 600);
    canvas.SetLeftMargin(0.14);
    canvas.SetRightMargin(0.18);
    canvas.SetBottomMargin(0.14);
    canvas.SetTopMargin(0.12);
    TGaxis::SetExponentOffset(-0.03, 0.0, "y");

    TString histName(name);
    if (histName.BeginsWith("hN") || histName.BeginsWith("hPt")) {
        canvas.SetLogy();
    }
    if (histName.BeginsWith("hPhi_")) {
        h->SetMinimum(0.0);
    }
    h->Draw("HIST");

    canvas.SaveAs(Form("./plots/%s.pdf", name));
    canvas.SaveAs(Form("./plots/%s.png", name));
}


/**
    * @brief Returns the display title for the specified histogram type.
    *
    * Maps the histogram type identifier to its corresponding descriptive title,
    * including ROOT-style formatting for physics symbols.
    *
    * @param kind Histogram type identifier (e.g. "hN", "hPt", "hEta", "hPhi").
    * @return Corresponding plot title, or the original identifier if no match is found.
    */

const char* combined_title(const TString& kind){
    if (kind == "hN") {
        return "Multiplicity Combined";
    }
    if (kind == "hPt") {
        return "p_{T} Combined";
    }
    if (kind == "hEta") {
        return "#eta Combined";
    }
    if (kind == "hPhi") {
        return "#phi Combined";
    }
    return kind.Data();
}

/**
    * @brief Creates and saves a combined plot of available histograms.
    *
    * Draws each category as a line on a shared canvas, configures the
    * y-axis scaling, adds a title and legend, and saves the plot in
    * PDF and PNG formats.
    *
    * @param file ROOT file containing the histograms to be plotted.
    * @param kind Histogram type identifier used to retrieve histograms
    *             and determine plot settings and output filenames.
    */
void make_and_extract_combined_canvas(TFile& file, const TString& kind){

    TH1D* histograms[nCategoryPalette] = {};
    const char* labels[nCategoryPalette] = {};
    int nFound = 0;
    double yMax = 0.0;

    for (int j = 0; j < nCategoryPalette; ++j) {
        TString name = Form("%s_%s", kind.Data(), kCategoryNames[j]);
        TH1D* h = dynamic_cast<TH1D*>(file.Get(name));
        if (!h) {
            printf("No histogram %s\n", name.Data());
            continue;
        }
        prepare_histogram(h, j);
        h->SetFillColor(kWhite);
        h->SetFillStyle(4000);
        h->SetLineWidth(2);
        h->SetStats(0);
        histograms[nFound] = h;
        labels[nFound] = kCategoryNames[j];
        ++nFound;
        if (h->GetMaximum() > yMax) {
            yMax = h->GetMaximum();
        }
    }

    if (nFound == 0) {
        printf("No histograms for combined plot %s\n", kind.Data());
        return;
    }

    const bool useLogY = kind.BeginsWith("hN") || kind.BeginsWith("hPt");
    histograms[0]->SetTitle(combined_title(kind));
    histograms[0]->SetMaximum(useLogY ? yMax * 8.0 : yMax * 1.2);
    if (!useLogY) {
        histograms[0]->SetMinimum(0.0);
    }

    TCanvas canvas(Form("c_%s_combined", kind.Data()), kind, 800, 600);
    canvas.SetLeftMargin(0.14);
    canvas.SetRightMargin(0.14);
    canvas.SetBottomMargin(0.14);
    canvas.SetTopMargin(0.12);
    TGaxis::SetExponentOffset(-0.03, 0.0, "y");
    if (useLogY) {
        canvas.SetLogy();
    }

    histograms[0]->Draw("AXIS");

    TLatex plotTitle;
    plotTitle.SetNDC();
    plotTitle.SetTextAlign(22);
    plotTitle.SetTextSize(0.055);
    plotTitle.DrawLatex(0.50, 0.96, combined_title(kind));

    TGraph graphs[nCategoryPalette];
    for (int i = 0; i < nFound; ++i) {
        TH1D* h = histograms[i];
        const int nBins = h->GetNbinsX();
        graphs[i].SetLineColor(h->GetLineColor());
        graphs[i].SetLineWidth(2);
        graphs[i].SetFillStyle(0);

        int nPoints = 0;
        graphs[i].Set(nBins);
        for (int bin = 1; bin <= nBins; ++bin) {
            const double y = h->GetBinContent(bin);
            if (useLogY && y <= 0.0) {
                continue;
            }
            graphs[i].SetPoint(nPoints, h->GetXaxis()->GetBinCenter(bin), y);
            ++nPoints;
        }
        graphs[i].Set(nPoints);
        graphs[i].Draw("L SAME");
    }

    TLegend legend(0.72, 0.52, 0.88, 0.88);
    legend.SetBorderSize(0);
    legend.SetFillColor(kWhite);
    legend.SetFillStyle(1001);
    legend.SetTextAlign(12);
    legend.SetTextColor(kBlack);
    legend.SetTextSize(0.038);
    legend.SetMargin(0.28);
    for (int i = 0; i < nFound; ++i) {
        TLegendEntry* entry = legend.AddEntry((TObject*)0, labels[i], "l");
        entry->SetLineColor(histograms[i]->GetLineColor());
        entry->SetLineWidth(3);
        entry->SetTextColor(kBlack);
        entry->SetFillStyle(0);
    }
    legend.Draw();

    canvas.SaveAs(Form("./plots/%s_combined.pdf", kind.Data()));
    canvas.SaveAs(Form("./plots/%s_combined.png", kind.Data()));
}


/**
    *@brief Generates individual and combined plots for all histogram types and categories.
    *
    * Generates individual and combined plots for all specified histogram types
    * and categories. Configures ROOT plotting settings, opens the input file,
    * applies histogram styling, and saves the resulting plots in the output
    * directory.
    */
void draw_plots(){

    gROOT->SetBatch(kTRUE);
    gSystem->mkdir("plots", true);
    gStyle->SetTitleFontSize(0.06);
    gStyle->SetTitleY(0.97);
    gStyle->SetStatX(0.98);
    gStyle->SetStatY(0.88);

    TFile file("analysis.root", "READ");

    TString kinds[] = {"hN", "hPt", "hEta", "hPhi"};
    const int nKinds = sizeof(kinds) / sizeof(kinds[0]);

    for (int i = 0; i < nKinds; ++i) {
        for (int j = 0; j < nCategoryPalette; ++j) {
            TString name = Form("%s_%s", kinds[i].Data(), kCategoryNames[j]);
            TH1D* h = dynamic_cast<TH1D*>(file.Get(name));
            if (!h) {
                printf("No histogram %s\n", name.Data());
                continue;
            }
            prepare_histogram(h, j);
            make_and_extract_canvas(h);
        }
        make_and_extract_combined_canvas(file, kinds[i]);
    }
}
