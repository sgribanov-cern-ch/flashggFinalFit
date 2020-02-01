import pandas as pd
import root_pandas as rpd
import numpy as np
import ROOT
import json

from root_numpy import tree2array

from optparse import OptionParser

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def add_mc_vars_to_workspace(ws=None, systematics_labels=[],add_benchmarks = False):
  IntLumi = ROOT.RooRealVar("IntLumi","IntLumi",1000)
  IntLumi.setConstant(True)
  getattr(ws, 'import')(IntLumi)

  weight = ROOT.RooRealVar("weight","weight",1)
  weight.setConstant(False)
  getattr(ws, 'import')(weight)

  CMS_hgg_mass = ROOT.RooRealVar("CMS_hgg_mass","CMS_hgg_mass",125,100,180)
  CMS_hgg_mass.setConstant(False)
  CMS_hgg_mass.setBins(160)
  getattr(ws, 'import')(CMS_hgg_mass)

  dZ = ROOT.RooRealVar("dZ","dZ",0.0,-20,20)
  dZ.setConstant(False)
  dZ.setBins(40)
  getattr(ws, 'import')(dZ)

#  ttHScore = ROOT.RooRealVar("ttHScore","ttHScore",0.5,0.,1.)
#  ttHScore.setConstant(False)
#  ttHScore.setBins(40)
#  getattr(ws, 'import')(ttHScore)


def apply_selection(data=None,reco_name=None):
  #function to split up ttree into recobins
  #if 'reco5' in reco_name: recobin_data = data[(data['pTH_reco']>=350.)]
  #recobin_data = recobin_data[((recobin_data['mgg']>=100.)&(recobin_data['mgg']<=180.))]
  return recobin_data
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def add_dataset_to_workspace(data=None,ws=None,name=None,systematics_labels=[],add_benchmarks = False, benchmark_num = -1, benchmark_norm = 1.):

  #apply selection to extract correct recobin
  #recobin_data = apply_selecetion(data,selection_name)

  #define argument set  
  arg_set = ROOT.RooArgSet(ws.var("weight"))
  variables = ["CMS_hgg_mass","dZ" ]#, "ttHScore"] #ttHScore
  for var in variables :
      arg_set.add(ws.var(var))

  #define roodataset to add to workspace
  roodataset = ROOT.RooDataSet (name, name, arg_set, "weight" )

  #Fill the dataset with values
  for index,row in data.iterrows():
    for var in variables:
      if var=='dZ' :  #to ensure only one fit (i.e. all RV fit)
        ws.var(var).setVal( 0. )
        ws.var(var).setConstant()
      else : 
        ws.var(var).setVal( row[ var ] )

    w_val = row['weight']

    roodataset.add( arg_set, w_val )

  #Add to the workspace
  getattr(ws, 'import')(roodataset)

  return [name]
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  
def get_options():

    parser = OptionParser()
    parser.add_option("--inp-files",type='string',dest='inp_files',default='DoubleEG_2016_2017_2018_24_01_2020')  
    #parser.add_option("--inp-files",type='string',dest='inp_files',default='DoubleEG_2016_24_01_2020')  #2016
    parser.add_option("--inp-dir",type='string',dest="inp_dir",default='/work/nchernya/DiHiggs/inputs/25_01_2020/data/')
    parser.add_option("--out-dir",type='string',dest="out_dir",default='/work/nchernya/DiHiggs/inputs/25_01_2020/')
    parser.add_option("--cats",type='string',dest="cats",default='DoubleHTag_0,DoubleHTag_1,DoubleHTag_2,DoubleHTag_3,DoubleHTag_4,DoubleHTag_5,DoubleHTag_6,DoubleHTag_7,DoubleHTag_8,DoubleHTag_9,DoubleHTag_10,DoubleHTag_11')
    #parser.add_option("--MVAcats",type='string',dest="MVAcats",default='0.44,0.67,0.79,1')
    #parser.add_option("--MXcats",type='string',dest="MXcats",default='250,385,470,640,10000,250,345,440,515,10000,250,330,365,545,10000')
    parser.add_option("--MVAcats",type='string',dest="MVAcats",default='0.33,0.55,0.68,1')
    parser.add_option("--MXcats",type='string',dest="MXcats",default='250,360,470,600,10000,250,330,365,540,10000,250,330,360,615,10000')
    parser.add_option("--ttHScore",type='float',dest="ttHScore",default=0.26)
    return parser.parse_args()
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  

(opt,args) = get_options()
cats = opt.cats.split(',')
MVAcats = opt.MVAcats.split(',')
MXcats = opt.MXcats.split(',')
nMVA =len(MVAcats)-1 
nMX = int(len(cats)/(len(MVAcats)-1))
cat_def = {}
for mva_num in range(0,nMVA):
  for mx_num in range(0,nMX):
    cat_num = mva_num*nMX+mx_num 
    cat_name = "DoubleHTag_%d"%(cat_num)
    cat_def[cat_name] = {"MVA" : [],"MX":[]}
    cat_def[cat_name]["MVA"] = [float(MVAcats[nMVA - mva_num]),float(MVAcats[nMVA - (mva_num+1)])]
    cat_def[cat_name]["MX"] = [float(MXcats[mva_num*(nMX+1) + (nMX - mx_num)]),float(MXcats[mva_num*(nMX+1) + (nMX - (mx_num+1))])]

input_files = opt.inp_files.split(',')
target_names = []

for num,f in enumerate(input_files):
   target_names.append('Data_13TeV') 
   input_files[num] = 'output_' + f 


for num,f in enumerate(input_files):
   print 'doing file ',f
   tfile = ROOT.TFile(opt.inp_dir + f+".root")
   tfilename = opt.inp_dir +f+".root"
   #define roo fit workspace
   datasets=[]
   ws = ROOT.RooWorkspace("cms_hgg_13TeV", "cms_hgg_13TeV")
   #Assemble roorealvariable set
   add_mc_vars_to_workspace( ws,'' )  # do not add them for the main systematics file
   for cat in cats : 
       print 'doing cat ',cat
       name = target_names[num]+'_'+cat
       initial_name = target_names[num]+'_DoubleHTag_0'
       #data = pd.DataFrame(tree2array(tfile.Get("tagsDumper/trees/%s"%initial_name))).
       selection = "(MX <= %.2f and MX > %.2f) and (HHbbggMVA <= %.2f and HHbbggMVA > %.2f) and (ttHScore >= %.2f)"%(cat_def[cat]["MX"][0],cat_def[cat]["MX"][1],cat_def[cat]["MVA"][0],cat_def[cat]["MVA"][1],opt.ttHScore)
       print 'doing selection ', selection
       data = rpd.read_root(tfilename,'tagsDumper/trees/%s'%initial_name).query(selection)
 
       datasets += add_dataset_to_workspace( data, ws, name,'') #systemaitcs[1] : this should be done for nominal only, to add weights
         
   f_out = ROOT.TFile.Open("%s/%s.root"%(opt.out_dir,input_files[num]),"RECREATE")
   dir_ws = f_out.mkdir("tagsDumper")
   dir_ws.cd()
   ws.Write()
   f_out.Close()