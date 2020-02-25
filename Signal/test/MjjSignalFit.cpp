// C++ headers
#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>
#include <string>

// ROOT headers
#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TPaveText.h>

// RooFit headers
#include <RooWorkspace.h>
#include <RooFitResult.h>
#include <RooRealVar.h>
#include <RooAbsReal.h>
#include <RooDataHist.h>
#include <RooCategory.h>
#include <RooArgSet.h>
#include "RooStats/HLFactory.h"
#include <RooDataSet.h>
#include <RooFormulaVar.h>
#include <RooGenericPdf.h>
#include <RooPlot.h>
#include <RooAbsPdf.h>
#include <RooBernstein.h>
#include <RooExtendPdf.h>
#include <RooMinimizer.h>
#include "RooStats/RooStatsUtils.h"
#include <RooMsgService.h>
#include <RooProdPdf.h>
#include <RooExponential.h>
#include <RooPolynomial.h>
#include <RooMoment.h>
#include <RooFitResult.h>

#include "boost/program_options.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/classification.hpp"
#include "boost/algorithm/string/predicate.hpp"
#include <boost/algorithm/string/replace.hpp>

#include "TGaxis.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TLegend.h"
#include "TLatex.h"
#include "TPaveText.h"
#include "TArrow.h"
#ifndef TDRSTYLE_C
#define TDRSTYLE_C
#include "../../tdrStyle/tdrstyle.C"
#include "../../tdrStyle/CMS_lumi.C"
#endif

using namespace RooFit ;
using namespace RooStats ;
using namespace std;

namespace po = boost::program_options;
string indir_;
string year_;
string templatefile_;
string signalproc_;
string outdir_;
string plotdir_;
string procStr_;
vector<string> procs_;
string flashggCatsStr_;
vector<string> flashggCats_;
string infilesStr_;
vector<string> infiles_;
bool mergeFitMVAcats_;
int nCats_;
int nMVA_;
int nMX_;

void OptionParser(int argc, char *argv[]){
	po::options_description desc1("Allowed options");
	desc1.add_options()
		("indir,d", po::value<string>(&indir_), "Input file dir")
		("infiles,i", po::value<string>(&infilesStr_), "Input files (comma sep)")
		("template,t", po::value<string>(&templatefile_), "Fit template file name")
		("year,y", po::value<string>(&year_), "year")
		("outfiledir,o", po::value<string>(&outdir_)->default_value("test/"), "Output file dir")
		("plotdir,p", po::value<string>(&plotdir_)->default_value("test/"), "Plot dir")
		("procs", po::value<string>(&procStr_)->default_value("hh_node_SM,ggh,qqh,vh,tth"), "Processes (comma sep)")
		("signalproc,s", po::value<string>(&signalproc_)->default_value("hh_node_SM"), "Name of the signal process")
		("flashggCats,f", po::value<string>(&flashggCatsStr_)->default_value("DoubleHTag_0,DoubleHTag_1,DoubleHTag_2,DoubleHTag_3,DoubleHTag_4,DoubleHTag_5,DoubleHTag_6,DoubleHTag_7,DoubleHTag_8,DoubleHTag_9,DoubleHTag_10,DoubleHTag_11"), "Flashgg categories")
		("nCats", po::value<int>(&nCats_)->default_value(12), "Numer of flashgg categories")
		("nMVA", po::value<int>(&nMVA_)->default_value(3), "Numer of MVA categories")
		("nMX", po::value<int>(&nMX_)->default_value(4), "Numer of MX categories")
		("mergeFitMVAcats,m", po::value<bool>(&mergeFitMVAcats_)->default_value(false), "Merge categories into 3 MVA for single Higgs fits ")
		;                                                                                             		
	po::options_description desc("Allowed options");
	desc.add(desc1);

	po::variables_map vm;
	po::store(po::parse_command_line(argc,argv,desc),vm);
	po::notify(vm);

	split(procs_,procStr_,boost::is_any_of(","));
	split(flashggCats_,flashggCatsStr_,boost::is_any_of(","));
	system(Form("mkdir -p %s",plotdir_ .c_str()));
	system(Form("mkdir -p %s",outdir_.c_str()));
}

void RooDraw(TCanvas *can, TH1F *h, RooPlot* frame,RooDataHist* hist, RooAbsPdf* model,string iproc,string category){
	can->cd();
	frame->GetXaxis()->SetTitle("M_{bb} (GeV)");
	frame->SetTitle("");
	frame->GetXaxis()->SetTitle("m_{jj} (GeV)");
	frame->GetXaxis()->SetTitleSize(0.05);
	frame->GetYaxis()->SetTitleSize(0.05);
	frame->GetYaxis()->SetTitleOffset(1.5);
	frame->SetMinimum(0.0);
	TGaxis::SetExponentOffset(-0.07,0,"xy");
   frame->GetYaxis()->SetTitle(Form("Events / (%.1f GeV)",h->GetBinWidth(1)));
   frame->GetYaxis()->SetNdivisions(505);
	frame->GetYaxis()->SetRangeUser(0.,h->GetBinContent(h->GetMaximumBin())*1.2);
 	frame->Draw();
	hist->plotOn(frame,MarkerStyle(kOpenSquare));
	TObject *dataLeg = frame->getObject(int(frame->numItems()-1));
	model->plotOn(frame,Normalization(h->Integral(),RooAbsReal::NumEvent),LineColor(kBlue),LineWidth(2),FillStyle(0));
	TObject *pdfLeg = frame->getObject(int(frame->numItems()-1));
	frame->Draw("same");

	TPaveText *pave = new TPaveText(0.55,0.65,0.8,0.85,"NDC");
	pave->SetTextAlign(11);
	pave->SetFillStyle(-1);
	pave->SetBorderSize(0);
	pave->SetTextFont(42);
	pave->SetTextSize(.05);
	pave->SetTextColor(kBlue+1);
	pave->AddText(category.c_str());
	pave->AddText((iproc).c_str());
	pave->AddText((year_).c_str());
//	pave->Draw("same");

	double offset =0.05;
	TString newtitle = iproc;
	if (iproc.find("hh") != std::string::npos) newtitle = "HH SM : H#rightarrow bb H#rightarrow#gamma#gamma"; 
	TLatex *lat1 = new TLatex(.129+0.03+offset,0.85,newtitle);
	lat1->SetNDC(1);
	lat1->SetTextSize(0.047);

	TString catLabel_humanReadable  = year_+" "+category;
	catLabel_humanReadable.ReplaceAll("_"," ");
	catLabel_humanReadable.ReplaceAll("DoubleHTag","CAT");
	TLatex *lat2 = new TLatex(0.93,0.88,catLabel_humanReadable);
	lat2->SetTextAlign(33);
	lat2->SetNDC(1);
	lat2->SetTextSize(0.045);

	cout<<"latex : "<<catLabel_humanReadable<<"  "<<newtitle<<endl;

	TLegend *leg = new TLegend(0.15+offset,0.60,0.5+offset,0.82);
	leg->SetFillStyle(0);
	leg->SetLineColor(0);
	leg->SetTextSize(0.037);
	leg->AddEntry(dataLeg,"#bf{Simulation}","lep");
	leg->AddEntry(pdfLeg,"#splitline{#bf{Parametric}}{#bf{model}}","l");

	lat2->Draw("same");
	lat1->Draw("same");
	leg->Draw("same");
}

int main(int argc, char *argv[]){

	OptionParser(argc,argv);

	RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);
	RooMsgService::instance().setSilentMode(true);
	setTDRStyle();
	extraText  = "";  // default extra text is "Preliminary"
	lumi_13TeV  = "2.7 fb^{-1}"; // default is "19.7 fb^{-1}"
	lumi_sqrtS = "13 TeV";       // used with iPeriod = 0, e.g. for simulation-only plots (default is an empty string)


	gROOT->SetBatch();
	gStyle->SetTextFont(42);

	string HLFactoryname = "wsig_fit";
	string allCatFitsTemplate = templatefile_; 
	HLFactory hlf(HLFactoryname.c_str(),allCatFitsTemplate.c_str(), false);
	RooWorkspace *w = (RooWorkspace*)hlf.GetWs();
	RooWorkspace *wAll = new RooWorkspace("wsig_13TeV","wsig_13TeV");

	float minSigFitMjj = 70;
	float maxSigFitMjj = 190;
	const int NCAT = flashggCats_.size(); 

	unsigned int iproc_num = 0;
	for (auto &iproc:procs_) {
		string signalfile = indir_+"output_"+iproc+"_"+year_+".root";
		if (infiles_.size()!=0)  //If inputfiles are provided by a user
			signalfile = indir_+infiles_[iproc_num];
		TFile *sigFile = TFile::Open(signalfile.c_str());
		RooWorkspace *w_original = (RooWorkspace*)sigFile->Get("tagsDumper/cms_hgg_13TeV");
		RooRealVar* Mjj  = (RooRealVar*)w_original->var("Mjj");
	
		string proc_type = "hig";	
		string proc_type_upper = proc_type;
		proc_type_upper[0] = std::toupper(proc_type_upper[0]);
		string iproc_type = "_"+iproc;
		if (iproc == signalproc_)  {
			proc_type = "sig";
			proc_type_upper = proc_type;
			proc_type_upper[0] = std::toupper(proc_type_upper[0]);
			iproc_type = "";
		}

		Mjj->setRange((proc_type_upper+"FitRange").c_str(),minSigFitMjj,maxSigFitMjj);
		int nbins = 24;
		Mjj->setBins(nbins); //70 - 190, reasonbale bins 5 GeV
		//  RooRealVar *weight = (RooRealVar*)w_original->var("weight");
		//
		RooDataSet* sigToFit[NCAT];
		RooDataSet* sigToFitMVA[nMVA_];
		std::map<int,int> categories_scheme = {{0,0},{1,0},{2,0},{3,0},{4,1},{5,1},{6,1},{7,1},{8,2},{9,2},{10,2},{11,2}};
		for (int ic = 0; ic < NCAT; ++ic)
		{
			auto icat = flashggCats_[ic];
			sigToFit[ic] = (RooDataSet*) w_original->data((iproc+"_"+year_+"_13TeV_125_"+icat).c_str());
			if ((mergeFitMVAcats_) && (NCAT==nCats_)) { //only works in case the set of categories is complete
				int mva_cat = categories_scheme[ic];
				if ( (ic+1)%nMX_ == 1 )
					sigToFitMVA[mva_cat] = ((RooDataSet*)sigToFit[ic]->Clone((iproc+"_"+year_+"_13TeV_125_MVA"+to_string(mva_cat)).c_str()));
				else 
					sigToFitMVA[mva_cat]->append(*((RooDataSet*)sigToFit[ic]));
			}
		}

		RooAbsPdf* MjjSig[NCAT];
		//RooFitResult* fitResult[NCAT];
		for (int ic = 0; ic < NCAT; ++ic)
		{
			auto icat = flashggCats_[ic];
			int c = stoi(icat.substr(icat.find_last_of("_")+1)); //find category number used
			
			if(iproc.find("ggh") != string::npos || iproc.find("qqh") != string::npos) {
					MjjSig[ic] = new RooBernstein(("MjjHig_"+iproc+"_cat"+std::to_string(c)).c_str(),"",*Mjj,
					RooArgList( *w->var( ("Mjj_hig_par1_"+iproc+"_cat"+std::to_string(c)).c_str()) ,
							*w->var(("Mjj_hig_par2_"+iproc+"_cat"+std::to_string(c)).c_str()) ,//))); //,
							*w->var(("Mjj_hig_par3_"+iproc+"_cat"+std::to_string(c)).c_str() )));
					w->import(*MjjSig[ic]);
			}
			else MjjSig[ic] = (RooAbsPdf*) w->pdf(("Mjj"+proc_type_upper+iproc_type+"_cat"+to_string(c)).c_str());

			MjjSig[ic]->Print();

			//Normalization per category
			RooRealVar *MjjSig_normalization = new RooRealVar(("hbbpdfsm_13TeV_"+iproc+"_"+year_+"_DoubleHTag_"+to_string(c)+"_normalization").c_str(),("hbbpdfsm_13TeV_"+iproc+"_"+year_+"_DoubleHTag_"+to_string(c)+"_normalization").c_str(),sigToFit[ic]->sumEntries()/1000.,"");//as for Hgg use fb
			MjjSig_normalization->setConstant(true);
			RooFormulaVar *finalNorm = new RooFormulaVar(("hbbpdfsm_13TeV_"+iproc+"_"+year_+"_DoubleHTag_"+to_string(c)+"_norm").c_str(),("hbbpdfsm_13TeV_"+iproc+"_"+year_+"_DoubleHTag_"+to_string(c)+"_norm").c_str(),"@0",RooArgList(*MjjSig_normalization));
			w->import( *finalNorm);


			if (proc_type == "sig")  ((RooRealVar*) w->var(("Mjj_"+proc_type+"_m0"+iproc_type+"_cat"+to_string(c)).c_str()))->setVal(125.);
			if ((mergeFitMVAcats_) && (NCAT==nCats_)){  //only works in case the set of categories is complete
				MjjSig[ic]->fitTo(*sigToFitMVA[categories_scheme[ic]],Range((proc_type_upper+"FitRange").c_str()),SumW2Error(kTRUE),PrintLevel(3));
			} else 
				MjjSig[ic]->fitTo(*sigToFit[ic],Range((proc_type_upper+"FitRange").c_str()),SumW2Error(kTRUE),PrintLevel(3));

			//fitResult[ic] =  (RooFitResult*)MjjSig[ic]->fitTo(*sigToFit[ic],Range((proc_type_upper+"FitRange").c_str()),SumW2Error(kTRUE),PrintLevel(3))->Clone();

			RooArgSet *sigParams = 0;
			sigParams = (RooArgSet*) MjjSig[ic]->getParameters(RooArgSet(*Mjj));

			w->defineSet((proc_type_upper+iproc_type+"PdfParam_cat"+to_string(c)).c_str(), *sigParams);
			auto params = w->set((proc_type_upper+iproc_type+"PdfParam_cat"+to_string(c)).c_str());

			std::vector<std::pair<string,string>> varsToChange;
			TIterator* iter(params->createIterator());
			for (TObject *a = iter->Next(); a != 0; a = iter->Next())
			{
				RooRealVar *rrv = dynamic_cast<RooRealVar *>(a);
				if (rrv) rrv->setConstant(true);
				string thisVarName(a->GetName());
				string newVarName = string(thisVarName);
				boost::replace_all(newVarName,"_cat", "_"+year_+"_DoubleHTag_");
				boost::replace_all(newVarName,proc_type+"_", "");
				boost::replace_all(newVarName,year_, (iproc+"_"+year_));
				//boost::replace_all(newVarName,"Mjj", "CMS_hbb_mass"); //will do this later
				w->import( *rrv, RenameVariable( thisVarName.c_str(), newVarName.c_str()));
				wAll->import(*rrv, RenameVariable( thisVarName.c_str(), newVarName.c_str()));  
				varsToChange.push_back(std::make_pair(thisVarName.c_str(), newVarName.c_str()));
			}
			//  w->import(*w->var("Mjj"), RenameVariable("Mjj","CMS_hbb_mass" ) );//will do this later
			//   wAll->import(*w->var("Mjj"), RenameVariable("Mjj","CMS_hbb_mass" ));  //will do this later if needed
			      
			sigParams->Print("v");

			string EditPDF = ("EDIT::hbbpdfsm_13TeV_"+iproc+"_"+year_+"_DoubleHTag_"+to_string(c)+"(Mjj"+proc_type_upper+iproc_type+"_cat"+to_string(c));
			for (unsigned int iv = 0; iv < varsToChange.size(); iv++)
			{
				EditPDF += (","+varsToChange[iv].first +"="+varsToChange[iv].second).c_str();		
			}
			EditPDF += (")");
			w->factory(EditPDF.c_str());

			///create TH1//////
			TH1F *h = new TH1F(("h_"+iproc+"_"+year_+"_"+to_string(c)).c_str(),("h_"+iproc+"_"+year_+"_"+to_string(c)).c_str(),nbins,minSigFitMjj,maxSigFitMjj);
			MjjSig[ic]->fillHistogram(h,RooArgList(*Mjj),sigToFit[ic]->sumEntries());


			TCanvas* can = new TCanvas("can","can",650,650);
  			can->SetLeftMargin(0.16);
  			can->SetTickx(); can->SetTicky();
			RooDraw(can,h,Mjj->frame(),((RooDataHist*)sigToFit[ic]),MjjSig[ic],iproc,("CAT "+to_string(c)));
			string sim="Simulation Preliminary";
			//string sim="Simulation"; //for the paper
			CMS_lumi( can, 0,0,sim);
			string canname = plotdir_+"fit_"+iproc+"_"+year_+"_cat"+to_string(c);
			can->Print((canname+".pdf").c_str());
			can->Print((canname+".jpg").c_str());
  			delete can;

			if ((mergeFitMVAcats_) && (NCAT==nCats_)){  //only works in case the set of categories is complete
				///create TH1//////
				TH1F *hMVA = new TH1F(("hMVA_"+iproc+"_"+year_+"_"+to_string(c)).c_str(),("h_"+iproc+"_"+year_+"_"+to_string(c)).c_str(),nbins,minSigFitMjj,maxSigFitMjj);
				MjjSig[ic]->fillHistogram(h,RooArgList(*Mjj),sigToFitMVA[ic]->sumEntries());

				TCanvas* canMVA = new TCanvas("canMVA","canMVA",650,650);
				RooDraw(canMVA,hMVA,Mjj->frame(),((RooDataHist*)sigToFitMVA[categories_scheme[ic]]),MjjSig[ic],iproc,("MVA "+to_string(categories_scheme[ic])));
				CMS_lumi( canMVA, 0,0,sim);
				string cannameMVA = plotdir_+"fit_"+iproc+"_"+year_+"_MVA"+to_string(categories_scheme[ic]);
				canMVA->Print((cannameMVA+".pdf").c_str());
				canMVA->Print((cannameMVA+".jpg").c_str());
  				delete canMVA;
			}

			string finalpdfname = "hbbpdfsm_13TeV_"+iproc+"_"+year_+"_DoubleHTag_"+to_string(c);
			wAll->import(*w->pdf(finalpdfname.c_str()));
			wAll->import( *w->function((finalpdfname+"_norm").c_str()));

			iproc_num+=1;
		}
		wAll->writeToFile((outdir_+"workspace_out_"+iproc+"_"+year_+".root").c_str());
	}
	std::cout<<"All fits have finished, now merge the workspaces : mergeWorkspace.py workspace_out_mjj.root *.root"<<std::endl;	
}