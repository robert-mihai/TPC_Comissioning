#include <TROOT.h>
#include <TH2D.h>
#include <TMath.h>
#include <TRandom.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <TFile.h>

using namespace std;
bool plotfig = false;

// string path = "../Data_ExcelAcquisition/Configuration_12/C3500_LSF1480_ThGUp1400_ThGDnGND_PMesh800_TPC1350_100kpts_100us_ii/";int tot_evt = 1000;//int tot_evt = 1739;
//string path = "../Data_ExcelAcquisition/HamamatsuAna/Configuration_12_like/PMT1350/";int tot_evt = 1000;
//string path = "../Data_ExcelAcquisition/Configuration_14/C3500_LSF1480_ThGUp1400_ThGDnGND_PMesh800_TPC1650_100kpts_100us/"; int tot_evt =590; //int tot_evt =591 ;


// string path = "../Data_ExcelAcquisition/HamamatsuAna/Configuration_12_like/PMT1650_ii/";int tot_evt = 4000; //100mV/div
// string path = "../Data_ExcelAcquisition/HamamatsuAna/Configuration_12_like/PMT1650_iii/";int tot_evt = 1000; //200mV/div

//string path = "../Data_ExcelAcquisition/HamamatsuAna/Configuration_12_like/taped_PMT_1650/"; int tot_evt = 1000;
//string path = "../Data_ExcelAcquisition/HamamatsuAna/Configuration_12_like/taped_PMT_1650_ii/";int tot_evt = 1000; //int tot_evt = 4000; //200mV/div



string path = "../Data_ExcelAcquisition/HamamatsuAna/Configuration_12_like/taped_PMT_1650_ii/";  int tot_evt = 1000;//int tot_evt = 1687;
//string path = "/eos/user/r/ramarine/Year1_PhD/TPC/Comissioning/TPC_Ana/LecroyPMT_Ana/Data_ExcelAcquisition/Configuration_15/C3500_LSF1480_ThGUp1400_ThGDnGND_PMesh800_TPC1650/";  int tot_evt = 1000;//int tot_evt = 1687;

const double  intg_spark[4] = {4,-100000, 0, 0};

//string path = "../Data_ExcelAcquisition/Configuration_12/test/";

//double const t0 = -0.00008;
//double const dt = 1.004082e-9;
double const R(50.), gain(2.5e6), e_ch(1.6e-19), qe(0.25), col_eff(0.8);
bool PlotAllWF (true);

TFile WF_output ((path+"Graphs/WF.root").c_str(),"RECREATE");

TH1* hADC_dump = new TH1F("ADC_dump"," ADC ; ADC amplitude [V];", 200, -1.5, 0.5 );




const double drift_time = 40e-6;
const double extended_drift_time = 80e-6;


//------------------------********************--------------------
//                      Define Integral function
//-----------------------*********************---------------------


void Integral(double *v0=NULL, FILE *readfile = NULL, int NPeds = 200, double NSigmas = 3. , TH1D *single = NULL, int evt = -1, double t0 = 0, double tmax = 0, int nentries = 0, int TrigTimeIdx = -1, double a_idx = -1, double ab_idx = -1){

  cout << " Trig Time Index " << TrigTimeIdx << endl;
  cout << " Index at end of drift " << a_idx << endl;
  cout << " Index at end of extended drift " << ab_idx << endl;

  std::cout << "Calculate Integrals " << endl;

  /*
  std::vector <double> dum_vec;
  dum_vec.push_back(-1000000);
  */


  //filename = path+"C4Test0006.txt";
  string filename = path;


  //double n,t;
  /*
  std::vector<float> adc;
  std::vector<float> time;
  std::vector<float> pedv;
  std::vector<float> threshv;
  */

  double adc[nentries];
  double pedv[nentries];
  double threshv[nentries];

  string ch;
  int nv;
  double a1,a2,sampling;
  string line;

/*
  if( filename == "Event91_RMS3.txt" ) {
    ifstream myfile("Event91_RMS3.txt");
    if (myfile.is_open()){
      int i = 0;
      while( getline(myfile,line) ) {
        stringstream ss(line);
        ss >> i >> t >> n;
        adc.push_back(n);
        time.push_back(t0 +(float)i*dt);
      }
      myfile.close();
    }
    else
    std::cout << "Unable to open file";
  }
  else {
    std::cout << " Filename " << filename << std::endl;
    ifstream myfile(filename.c_str());
    if (myfile.is_open()){
      getline (myfile,line);

      std::cout << line << std::endl;
      int i = 0;
      while( getline(myfile,line) ) {
        stringstream ss(line);
        ss >> n;
        adc.push_back(n);
        time.push_back(t0 +(float)i*dt);
        i++;
      }
      myfile.close();
    }
    else {
      std::cout << "Unable to open file";
      return dum_vec;
    }
  }

  Long64_t nentries = adc.size();
  */

//  std::cout << "Trig Time " << time.at(TrigTimeIdx) << " at " << TrigTimeIdx << endl;
//  std::cout << "Drift Time " << time.at(a_idx) << " extended_drift_time " << time.at(ab_idx) << endl;

  //if( nentries < NPeds ) return dum_vec;
  if (nentries < NPeds) std::cout << "Warning: Not enough entries!" << endl;

  for (int k=0; k<nentries; k++){
      fread(&adc[k], sizeof(double), 1, readfile);
  }

  float ped = 0;
  float ped2 = 0;

//*********
//calculate simple baseline for
//*********
  for( int i = 0; i < NPeds; i++ ) {
    ped += adc[i]/NPeds;
    ped2 += adc[i]*adc[i]/NPeds;
  }

  double rms = TMath::Sqrt(ped2-ped*ped); // determine rms
  double thres = NSigmas*rms;

  double pedsaved = ped;
  ped = ped2 = 0;
  double iped = 0;


  double min = 100000;

  //************************
  //calculate pedestal for the first 2000 entries, only for adc smaller than NSigmas* rms
  //************************

  for( int i = 0; i < NPeds; i++ ) {
    if( TMath::Abs(adc[i]-pedsaved) < thres ) {
      ped += adc[i];
      ped2 += adc[i]*adc[i];
      iped++;
    }
    if( TMath::Abs(adc[i]-adc[i+1]) < min && TMath::Abs(adc[i]-adc[i+1])  > 0   ) min = TMath::Abs( adc[i]-adc[i+1] );
  }

  ped /= iped;
  ped2 /= iped;

  rms = TMath::Sqrt(ped2-ped*ped);
  thres = NSigmas*rms;

  pedsaved = ped;
  double thressaved = thres;

  //*********************
  // Now compute the pedestals for 200, 201, 202, 203 etc.. and save them to pedv. each
  //*******************
  for(int i = 0; i < nentries; i++ ) {
    if( TMath::Abs(adc[i]-adc[i+1]) < min && TMath::Abs(adc[i]-adc[i+1]) > 0 ) min = TMath::Abs( adc[i]-adc[i+1] );
    double pp = ped*iped;
    double pp2 = ped2*iped;
    if( i >= NPeds ) {
      if( TMath::Abs(adc[i]-ped) < thres ) {pp += adc[i]; pp2+=adc[i]*adc[i]; iped++;}
      if( TMath::Abs(adc[i-NPeds]-pedv[i-NPeds]) < threshv[i-NPeds] ) {pp -= adc[i-NPeds]; pp2-=adc[i-NPeds]*adc[i-NPeds]; iped--;} // Remove it if it was used.
    }

    if( iped > NPeds ) std::cout << " Error " << iped << "  > " <<  NPeds  << std::endl;

    //    std::cout << iped << std::endl;

    pedv[i]=pp/iped; // Same pedestal used in this sample calculation
    threshv[i]=thres;

    ped = pp/iped;
    ped2 = pp2/iped;
    thres = NSigmas*TMath::Sqrt(ped2-ped*ped);

    if( thres < min ) thres = NSigmas*min;

    if( ped2 < ped*ped ) {thres = NSigmas* min;}

  }


  static TH1F *WF = NULL;
  if( WF )
  WF->Reset();
  else
  WF = new TH1F("WF","  ",nentries, t0-0.5e-9,((tmax)-0.5e-9));
  static TH1F *Cum = NULL;
  if( Cum )
  Cum->Reset();
  else
  Cum = new TH1F("Cum","  ",nentries, t0-0.5e-9,((tmax)-0.5e-9));
  static TH1F *Ped = NULL;
  if( Ped )
  Ped->Reset();
  else
  Ped =  new TH1F("Ped","  ",nentries, t0-0.5e-9,((tmax)-0.5e-9));
  static TH1F *Thr = NULL;
  if( Thr )
  Thr->Reset();
  else
  Thr = new TH1F("Thr","  ",nentries, t0-0.5e-9,((tmax)-0.5e-9));

  static TH1F * WF_raw = NULL;
  if( WF_raw )
  WF_raw->Reset();
  else
  WF_raw = new TH1F("WF_raw","  ",nentries, t0-0.5e-9,((tmax)-0.5e-9));

  static TH1F * WF_spark = NULL;
  if( WF_spark )
  WF_spark->Reset();
  else
  WF_spark =  new TH1F("WF_spark","  ",nentries, t0-0.5e-9,((tmax)-0.5e-9));



  float cadc = 0.;

  int iaccum = 0;
  double accum = 0;

  for(int i = 0; i < nentries; i++ ) {
    ped = pedv[i];
    cadc += adc[i]-ped;
    if (adc[i] - ped < -1.) {
      WF_spark->SetBinContent(i+1,adc[i]-ped);
      WF_output.WriteObject(WF_spark, Form("evt%d", evt));
      //return &intg_spark[0];
      //come back to this later once I get the non-spark return working
    }
    if (adc[i]-ped > 1.5) cout<<  "adc - iped " << adc[i]-ped << endl;
    hADC_dump->Fill(adc[i]-ped);
    if (single){
      if(  TMath::Abs(adc[i]-ped) > thres/NSigmas*3. ) {
        accum += -(adc[i]-ped);
        iaccum++;
      }
      else {
        if(  iaccum == 2 || iaccum == 3 ) single->Fill(accum);
        iaccum = 0;
        accum = 0;
      }
    }
    WF_raw->SetBinContent(i+1,adc[i]);
    WF->SetBinContent(i+1,adc[i]-ped);
    Cum->SetBinContent(i+1,cadc);
    Ped->SetBinContent(i+1,ped);
    Thr->SetBinContent(i+1,threshv[i]);
    if( i >= NPeds ) {
      if( TMath::Abs(adc[i]-ped) < thres && TMath::Abs(adc[i-NPeds]-pedv[i-NPeds]) < thres )
      ped += adc[i]/NPeds-adc[i-NPeds]/NPeds;
    }
  }

  if(PlotAllWF){
    WF->SetTitle(Form("%s",filename.c_str()));
    WF_output.WriteObject(WF, Form("evt%d", evt));

  }

  if( plotfig ) {

    TCanvas *c =  new TCanvas("c"," ",800,800);
    c->Divide(1,4);

    c->cd(1);
    WF_raw->Draw("Hist");


    c->cd(2);
    WF->Draw("Hist");

    c->cd(3);

    Cum->Draw("Hist"); Cum->SetLineColor(2);

    c->cd(2);
    Ped->Draw("Hist same"); Ped->SetLineColor(3);

    c->cd(4);
    Ped->Draw("Hist");

    c->Update();

    string fn = filename+".pdf";


    //c->Print(fn.c_str());
  }
  /*std::vector<double> intg_vect;

  intg_vect.push_back(Cum->GetBinContent(TrigTimeIdx));
  intg_vect.push_back(Cum->GetBinContent(nentries) - Cum->GetBinContent(TrigTimeIdx));
  intg_vect.push_back(Cum->GetBinContent(a_idx) - Cum->GetBinContent(TrigTimeIdx));
  intg_vect.push_back(Cum->GetBinContent(ab_idx) - Cum->GetBinContent(TrigTimeIdx));
  */


  *v0 = Cum->GetBinContent(TrigTimeIdx);
  *(v0+sizeof(double)) = Cum->GetBinContent(nentries) - Cum->GetBinContent(TrigTimeIdx);
  *(v0+2*sizeof(double)) = Cum->GetBinContent(a_idx) - Cum->GetBinContent(TrigTimeIdx);
  *(v0+3*sizeof(double)) = Cum->GetBinContent(ab_idx) - Cum->GetBinContent(TrigTimeIdx);
  //this method returns a vector with 4 entries:
  //int_bef, int_aft, int_a, int_ab
  //return intg_vect;
}


void All(void ) {

  TH1D *single = new TH1D("single","single",1000,-0.1,1.5);
  TH1D *integral_bef = new TH1D("integral_bef","",100, -10,1.2e3);
  TH1D *integral_aft = new TH1D("integral_aft","",100, -10,1.2e3);
  TH1D *integral_a = new TH1D("integral_a","",100, -10,1.2e3);
  TH1D *integral_ab = new TH1D("integral_ab","",100, -10,1.2e3);
  TH2D *integral_a_ab = new TH2D("integral_a_ab","b vs a; a; b",100, -10,1.2e3,100, -10,1.2e3);

  double max = 0;
  double imax;
  FILE *readfile;
  readfile = fopen("binary_dat", "r");

  char name[100];
// float float_in;
    int int_in;
   double double_in;

 //first, read the header.
 //name
 fread(&name, sizeof(char), 100, readfile);
 cout << "Name from binary file: " << name << endl;
 cout << "If this does not match the name you were expecting, double-check the configuration parameters!" << endl;

 //t0
 double t0;
 fread(&t0, sizeof(double), 1, readfile); //get t0 from the binary file

 //dt
 double dt;
 fread(&dt, sizeof(double), 1, readfile); //get dt from the binary file

 //number of entries per event
 int nentries;
 fread(&nentries, sizeof(int), 1, readfile);

 //time of scintillator signal peak
 int TrigTimeIdx;
 fread(&TrigTimeIdx, sizeof(int), 1, readfile);

 double const a_idx = TrigTimeIdx + int(drift_time/dt);
 double const ab_idx = TrigTimeIdx + int(extended_drift_time/dt) ;

 const double tmax = t0+ (dt*nentries);
 double v[4];

  for( int i = 0;i < tot_evt; i++) { //can change this to  a while loop so we don't need to put in number of events...
    //  string path = "../Data_ExcelAcquisition/Configuration_12/C3500_LSF1480_ThGUp1400_ThGDnGND_PMesh800_TPC1350_100kpts_100us_ii/";

    /*
    Run the integral function. It saves the results in the array pointed to by &v (i.e. it saves the results in v).
    */
    Integral(&v[0], readfile,150,4.,single, i, t0, tmax, nentries, TrigTimeIdx, a_idx, ab_idx); //not sure if this will work!

    cout << " asdasdassds " << (dt /(R* gain *e_ch*qe*col_eff)) << endl;
    integral_bef->Fill(-v[0]  * dt /(R * gain *e_ch*qe*col_eff) );
    integral_aft->Fill(-v[1]*  dt /(R* gain *e_ch*qe*col_eff));
    integral_a->Fill(-v[2]*   dt /(R* gain *e_ch*qe*col_eff));
    integral_ab->Fill(-v[3]*  dt /(R* gain *e_ch*qe*col_eff));
    integral_a_ab->Fill(-v[2]*  dt /(R* gain *e_ch*qe*col_eff), -v[3]*  dt /(R* gain *e_ch*qe*col_eff) );
    if( -v[0] > max ) { max = -v[0]; imax = i;}
  }

  fclose(readfile);

  TCanvas * c1_single =new TCanvas("c1_single","",800,600);

  single->Draw();
  c1_single->Update();


  TCanvas *c_Intg = new TCanvas("c1","",1200,800);
  c_Intg->Divide(2,2);
  c_Intg->cd(1);
  integral_bef->Draw();
  c_Intg->cd(2);
  integral_aft->Draw();
  c_Intg->cd(3);
  integral_a->Draw();
  c_Intg->cd(4);
  integral_ab->Draw();

  TCanvas *c_Intg_2 = new TCanvas("c_Intg_2","",1200,800);
  integral_a_ab->Draw("colz");
  string fn_str = path+"Graphs/outputFile.root";
// Form("%s" , (name_store.at(i)).c_str() )

  TFile outputFile (fn_str.c_str(),"RECREATE");

  TCanvas * cADC = new TCanvas("cADC", " ", 800,600);
  hADC_dump->Draw();

  c_Intg->Write();
  c_Intg_2->Write();
  cADC->Write();
  integral_bef->Write();
  integral_aft->Write();
  integral_a->Write();
  integral_ab->Write();
  hADC_dump->Write();
  single->Write();

  std::cout << " Max " <<  max << " Event " << imax << std::endl;
}

/*
void plotWFSum() //don't worry about this; Robert is changing some things with it...
{
        TCanvas *time = new TCanvas("c1", "overlap", 0, 0, 800, 600);

        const char* histoname = "sc";

        const int NFiles = tot_evt;
        for (int fileNumber = 0; fileNumber < NFiles; fileNumber++)
        {
                TFile* myFile = TFile::Open(Form("Processed_Data/hists%i_blinded.root", fileNumber));
                if (!myFile)
                {
                        printf("Nope, no such file!\n");
                        return;
                }
                TH1* h1 = (TH1*)myFile->Get(histoname);
                if (!h1)
                {
                        printf("Nope, no such histogram!\n");
                        return;
                }
                h1->SetDirectory(gROOT);
                h1->Draw("same");
                myFile->Close();
        }
}
*/

void PlotSingles(double *dt){
  TCanvas * cPht = new TCanvas("cPhNumber","Photons Hitting Cathode", 800,600 );
    TF1 *fPhNumber =  new TF1("phot_hit", " x /([0] ) ", 0., 0.5);
    fPhNumber->SetParameter(0,  R* gain *e_ch*qe*col_eff / *dt);
    fPhNumber->SetTitle("Photons number as a function of Ampl [V]; Volt on te Scope [V] ; Photons Hitting the PMT ");
    fPhNumber->Draw();
}
