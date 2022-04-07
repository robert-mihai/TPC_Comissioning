//read binary files into arrays containing time and adc values for each event. For now this is just a test file to make sure the binary was saved correctly. 

#include <iostream>
#include<fstream>

using namespace std;


int main(){
 FILE *readfile;
 readfile = fopen("binary_dat", "r");
 const int sd = sizeof(double);
 
 char name[100];
// float float_in;
    int int_in;
   double double_in;
    
 //first, read the header. 
 //name
 fread(&name, sizeof(char), 100, readfile);
    
 //t0
 fread(&double_in, sd, 1, readfile);
 cout << "t0 = "<< double_in << endl;
  
 //dt
 fread(&double_in, sd, 1, readfile);
 cout << "dt = " << double_in << endl;
    
 //n_entries_per_file
 fread(&int_in, sizeof(int), 1, readfile);
 cout << "Number of entries per file = " << int_in << endl;
 
 bool flag = true;
 while(flag){
     if(1==fread(&double_in, sd, 1, readfile)){
         //cout << double_in << endl;
     }
     else{
      flag = false;
     }
     
 }
    cout << "Finished reading. " << endl;
 return 0;   
}