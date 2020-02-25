import numpy as np
import ROOT
import json

from optparse import OptionParser


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  
def get_options():

    parser = OptionParser()
    parser.add_option("--date",type='string',dest='date',default='18_02_2020')  
    #parser.add_option("--date",type='string',dest='date',default='12_02_2020_mjjnorm')  

    parser.add_option("--inp-procs",type='string',dest='inp_procs',default='hh_node_SM,qqh,tth,vh,ggh')  
    parser.add_option("--inp-dir",type='string',dest="inp_dir",default='/work/nchernya/DiHiggs/CMSSW_7_4_7/src/flashggFinalFit/Plots/FinalResults/inputs/')
    parser.add_option("--inp-dir-mjj",type='string',dest="inp_dir_mjj",default='/work/nchernya/DiHiggs/CMSSW_7_4_7/src/flashggFinalFit/Signal/output/mjj/18_02_2020_2D/')
    parser.add_option("--inp-file",type='string',dest="inp_file",default='CMS-HGG_sigfit_2016_2017_2018_18_02_2020.root')
    parser.add_option("--inp-file-mjj",type='string',dest="inp_file_mjj",default='workspace_out_mjj_18_02_2020.root')
    #parser.add_option("--inp-dir-mjj",type='string',dest="inp_dir_mjj",default='/work/nchernya/DiHiggs/CMSSW_7_4_7/src/flashggFinalFit/Signal/output/mjj/04_02_2020_v3/')
    #parser.add_option("--inp-file",type='string',dest="inp_file",default='CMS-HGG_sigfit_2016_2017_2018_04_02_2020.root')
    #parser.add_option("--inp-file-mjj",type='string',dest="inp_file_mjj",default='workspace_out_mjj_12_02_2020.root')
    parser.add_option("--out-dir",type='string',dest="out_dir",default='/work/nchernya/DiHiggs/CMSSW_7_4_7/src/flashggFinalFit/Signal/output/')
    parser.add_option("--cats",type='string',dest="cats",default='DoubleHTag_0,DoubleHTag_1,DoubleHTag_2,DoubleHTag_3,DoubleHTag_4,DoubleHTag_5,DoubleHTag_6,DoubleHTag_7,DoubleHTag_8,DoubleHTag_9,DoubleHTag_10,DoubleHTag_11')
    return parser.parse_args()
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  

(opt,args) = get_options()
cats = opt.cats.split(',')
input_procs = opt.inp_procs.split(',')


tfile = ROOT.TFile(opt.inp_dir + opt.inp_file)
tfile_mjj = ROOT.TFile(opt.inp_dir_mjj + opt.inp_file_mjj)
ws_mgg = tfile.Get("wsig_13TeV")
ws_mjj = tfile_mjj.Get("wsig_13TeV")
for num,f in enumerate(input_procs):
 for year in '2016,2017,2018'.split(','):
  for cat_num,cat in enumerate(cats) : 
  #for cat_num,cat in enumerate([cats[0]]) : 
    pdf_mjj = "hbbpdfsm_13TeV_%s_%s_%s"%(input_procs[num],year,cat)
    pdf_mgg = "hggpdfsmrel_13TeV_%s_%s_%s"%(input_procs[num],year,cat)
    #ws_mjj.pdf(pdf_mjj).Print("v")
    getattr(ws_mgg, 'import')(ws_mjj.pdf(pdf_mjj),ROOT.RooCmdArg())
    getattr(ws_mgg, 'import')(ws_mjj.var("Mjj"),ROOT.RooCmdArg())
    prod_pdf = "hhbbggpdfsm_13TeV_%s_%s_%s"%(input_procs[num],year,cat)
    print ws_mgg.pdf(pdf_mgg)
    print ws_mjj.pdf(pdf_mjj)
    sig_prod_pdf = ROOT.RooProdPdf(prod_pdf,"",ws_mgg.pdf(pdf_mgg),ws_mjj.pdf(pdf_mjj))
    #sig_prod_pdf.Print("v") 
    getattr(ws_mgg, 'import')(sig_prod_pdf,ROOT.RooFit.RecycleConflictNodes())
    #Save normalization for combine
    #sig_prod_pdf_norm = (ws_mgg.function(pdf_mgg+"_norm")).clone(prod_pdf+"_norm")
    sig_prod_pdf_norm = (ws_mjj.function(pdf_mjj+"_norm")).clone(prod_pdf+"_norm")  #take norm from mjj?
    getattr(ws_mgg, 'import')(sig_prod_pdf_norm,ROOT.RooFit.RecycleConflictNodes())
    ##Printing normalization
   # ws_mgg.var("MH").setVal(125.)  #just to check that the normalization is the same for Mgg and Mjj, it is of course.
   # print 'mgg : ',ws_mgg.function(pdf_mgg+"_norm").getVal(),', mjj : ',ws_mjj.function(pdf_mjj+"_norm").getVal(), ", imported product : ",ws_mgg.function(prod_pdf+"_norm").getVal() #just to check that the normalization is the same for Mgg and Mjj, it is of course.
    
 
f_out = ROOT.TFile.Open(opt.out_dir+"CMS-HGG_sigfit_MggMjj_2016_2017_2018_%s.root"%opt.date,"RECREATE")
ws_mgg.Write()
f_out.Close()