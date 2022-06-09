//read binary files to arrays
#include <iostream>
#include<fstream>

using namespace std;

int read_binary_3(){
 string path = "/eos/user/r/ramarine/Year1_PhD/TPC/Comissioning/TPC_Ana/LecroyPMT_Ana/Data_ExcelAcquisition/Configuration_17/C3500_LSF1480_ThGUp1400_ThGDnGND_PMesh1400_TPC1650";
 string binfilepath = path+"/binary_dat";
 FILE *readfile;
 readfile = fopen(binfilepath.c_str(), "r");

 const int sd = sizeof(double); //we define this to save a bit of time by not having to fetch it each time we read

 double t0;
 double dt;
 int nentries; 
 int trig_idx;
 char name[100];    
 const int nevents = 1000;

 //first, read the header. 
 //name
 fread(&name, sizeof(char), 100, readfile);
 //cout << "Name of file is " << name << endl;
    
 //t0
 fread(&t0, sd, 1, readfile);
 //cout << "t0 = "<< t0 << endl;
  
 //dt
 fread(&dt, sd, 1, readfile);
 //cout << "dt = " << dt << endl;
    
 //n_entries_per_file
 fread(&nentries, sizeof(int), 1, readfile);
 //cout << "Number of entries per file = " << nentries << endl;
 
 //trigger index
 fread(&trig_idx, sizeof(int), 1, readfile);
 //cout << "Index of scintillator minimum = " << trig_idx << endl;

 vector<vector<double>> ampl_tpc(nevents, (vector<double>(nentries)));   
 
    for (int evt = 0; evt<nevents; evt++){
     for (int entry = 0; entry<nentries; entry++){
        fread(&(ampl_tpc[evt][entry]), sd, 1, readfile);
     }
        
        //Put your code which does the per-event analysis here. 
        
    }
    
    //Put your code to do the analysis of the full dataset here. 
    
    

 return 0;   
}