//code to merge the files into a single file in binary format with a header and data. 
#include <iostream>
#include<sstream>
#include<fstream>
#include<cmath>

using namespace std;

double convert_sci(string* str){
    stringstream ss(*str);
    string temp; 
    int flag = 0;
    float base;
    int exp;
    while (getline(ss,temp,'E')){
            //cout << temp << endl;
            if (0==flag) {
             flag=1; 
             base = stof(temp);
            }
            else {
             flag=2;
             exp = stoi(temp);
            }
    }
    if (0==flag){
       base = 0;
       exp = 0;
    }
    else if (1==flag){
     exp = 0;   
    }
    return double(base*pow(10,exp));
}

int main(int argc, char* argv[]){
cout<<"Merging files; please be patient..."<<endl;
int size_int = sizeof(int);
//int size_float = sizeof(float);
int size_double = sizeof(double);
/*int size_intptr = sizeof *int;
int size_floatptr = sizeof *float;
int size_strptr = sizeof *string ;*/
//double flagval = 999999999; //Nine nines. Will be used to signal end of a subfile. 

//Get filename(s) from user
string readfilepath, writefilepath, name, lastchar;
string stub = "/eos/user/r/ramarine/Year1_PhD/TPC/Comissioning/TPC_Ana/LecroyPMT_Ana/Data_ExcelAcquisition/";

if (1 == argc){
    //this path is just a backup in case none is entered. 
  readfilepath = "/eos/user/r/ramarine/Year1_PhD/TPC/Comissioning/TPC_Ana/LecroyPMT_Ana/Data_ExcelAcquisition/Configuration_15/C3500_LSF1480_ThGUp1400_ThGDnGND_PMesh800_TPC1650/";
}
else {
 readfilepath = argv[1];   
}
lastchar = readfilepath.substr(readfilepath.length()-1,1);
 if ("/" != lastchar){ //we need to make sure the folders are referenced correctly so we automatically add a / if needed. 
     readfilepath = readfilepath+"/";
     cout << "Added a / to the end of the readfile name." << endl;
 }
if (argc<3){
 writefilepath = readfilepath;
}
else {
 writefilepath = argv[2];   
}
lastchar = writefilepath.substr(writefilepath.length()-1,1);
if ("/" != lastchar){ //we need to make sure the folders are referenced correctly so we automatically add a / if needed. 
     writefilepath = writefilepath+"/";
     cout << "Added a / to the end of the writefile name." << endl;
}
writefilepath = writefilepath + "binary_dat";
if (argc>3){
   cout << "Warning: Using user-provided name." << endl;
   name = argv[3];
}
else if (stub.length()<readfilepath.length()-2){
    name = readfilepath.substr(stub.length(), readfilepath.length()-stub.length()-1);
}
else {
 cout << "Please enter a name including the values of all pertinent parameters corresponding to the data in the read file.";
 cin >> name;
}

cout<< "readfilepath = " << readfilepath << endl;
cout<< "writefilepath = " << writefilepath << endl;
cout<< "name = " << name <<endl;
    
    
FILE *outfile; 
outfile = fopen(writefilepath.c_str(), "w");

/*
First we do the header of the file. The format is as follows: 
1. File Name, 2. t0, 3. dt
In the future, we could replace file name with the relevant parameters. 
However, the parameters we want to associate to each file are still 
changing a bit as we go, so for now I made it flexible by putting a name. 
*/
    
string infilename = readfilepath + "C1Test0000.txt";
ifstream file;
file.open(infilename,ios::in);
    
string line_in;
    
if (file.is_open()){
    string junk, t0str, tfstr, dtstr, tabsstr;
    
    getline(file,junk, ','); //first entry is C4
    getline(file,junk, ','); //second entry is 0
    getline(file,t0str, ','); //third entry is t0
    getline(file,tfstr, ','); //fourth entry is t_final
    getline(file, dtstr, ','); //fifth entry is dt
    getline(file, tabsstr); //sixth entry is the absolute trigger time
    
    
    //convert from scientific notation; we will save base and exponent separately to make sure we don't run into underflow problems. In reality the base may be written as 0.000xx or something, but it doesn't matter. 
    double t0 = convert_sci(&t0str);
    double tf = convert_sci(&tfstr);
    double dt = convert_sci(&dtstr);
    double tabs = convert_sci(&tabsstr);
    
    //fprintf(outfile, name.c_str());
    //fprintf(outfile, ",%f,%d,%f,%d\n",t0base,t0exp,dtbase,dtexp);
    const char* nm = name.c_str(); 
    fwrite(nm, sizeof(char[100]), 1, outfile); //since we have to read out a specific size, we force it to be stored as a specific size, even if all the space is not needed. 
    fwrite(&t0, size_double, 1, outfile);
    fwrite(&tf, size_double, 1, outfile);
    fwrite(&dt, size_double, 1, outfile);
    fwrite(&tabs, size_double, 1, outfile);

    int index = 0;
    int mintrigindex = -1;
    double mintrigval = 10000;
    double trigval = 0;
    while(getline(file, line_in)){ //read the file until we reach the end. 
        trigval = convert_sci(&line_in);
        if (trigval<mintrigval){
            mintrigval = trigval;
            mintrigindex = index;
        }
        index++;
    }
    fwrite(&index, sizeof(int), 1, outfile); //write the number of entries per file
    fwrite(&mintrigindex, sizeof(int), 1, outfile); //write the trigger peak index
    
    file.close();
}
else {
    printf("Error opening file C1Test0000.txt\n"); 
    return 1;
}
    //that completes the part for the header. Next we want to concatenate all the files' data columns (excluding their headers). 
    //in the future we may be able to further save space by the following method: give the zero of the ADC values (closest val to zero), the min. spacing between values (save these in the file header), and then save the integer value of number of spaces away from zero (positive or negative). Then we can convert back easily in our other program. However, this would also take computation at the other end so is maybe not worth it. We could of course do all our analysis on that side just with the integers, which would really be faster... can talk to Federico about this but it's probably not too important in the long run. 
    
    bool incomplete = true;
    int tolerance = 2; 
    int num_missing_files = 0;
    int current_file_num = 0; 
    double adc;
    int curr_line_num = 1;
    //to avoid having to manually program in the number of files, we take advantage of the fact that they are labelled in sequential order. In case there are one or two missing, we try again a few extra times 
    while (incomplete){
        infilename = readfilepath+"C4Test000"+to_string(current_file_num)+".txt";//it's a little silly the way the files are named, but convenient for programming this! There are always three zeros before the number; it doesn't do a replacement. 
        //we reuse the fstream named "file" which was defined above. 
        //printf("Attempting to open %s\n", infilename);
        file.open(infilename,ios::in);
        if (file.is_open()){
            //printf("Successfully opened %s\n", infilename);
            //cout << "Opened " << infilename << endl;
            num_missing_files=0;
            getline(file, line_in, '\n'); 
            //cout << line_in << endl;
            //curr_line_num=1;
            while(getline(file, line_in)){ //read each file until we reach the end. 
                //printf("Reading line %d \n", curr_line_num);
                //cout << line_in << endl;
                //curr_line_num++;
                
                adc = convert_sci(&line_in);
                
                //fprintf(outfile, "%f %d\n", adc_base, adc_exp);
                fwrite(&adc, size_double, 1, outfile);
            }
            file.close();
            //fprintf(outfile, "\n"); //we want a way to separate the data from different events, in case we change the length of an event, so that we don't need to change our analysis codes. Here I just use a newline.  
            //fwrite(&flagval, size_double, 1, outfile);
            //the above is no longer needed since the files are a constant length. we now just read the proper amount. 
        }
        else{
         cout << "Failed to open " << infilename << ". This is normal behavior if occurring at the end of the program; otherwise a file may be missing. " << endl;
         num_missing_files++;
         if (num_missing_files>tolerance){
          incomplete = false;   
         }
        }
            
         current_file_num++;
    } //end of main while loop

fclose(outfile);
cout << "Program complete. Merged data stored in file: binary_dat" << endl;
return 0;
}