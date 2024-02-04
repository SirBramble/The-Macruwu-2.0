#include <iostream>
#include <fstream>
#include <string>
using namespace std;


int main(void){
	string line;
  	ifstream myfile ("input.txt");
	ofstream outFile;
  	outFile.open ("output.txt");
	int found = 0;
	int error = 0;
	bool first = 1;
  	if (myfile.is_open()){
    	while ( getline (myfile,line) ){
      		//cout << line << endl;
			if(line.find("delayTime")!=std::string::npos){
  				if(first){
					first = 0;
					outFile << line << endl;
					continue;
				}
				else{
					outFile << "Found Lines: " << (found + error) << endl;
					outFile << "Errors: " << error << endl << endl;
					outFile << line << endl;
					found = 0;
					error = 0;
					continue;
				}
			}
			if(line.find("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789")!=std::string::npos){
				found++;
			}
			else{
				error++;
			}
    	}
	if((found != 0)||(error != 0)){
		outFile << "Found Lines: " << (found + error) << endl;
		outFile << "Errors: " << error << endl;
	}
    myfile.close();
  	}
  	outFile.close();
	ifstream check ("output.txt");
	while ( getline (check,line) ){
		cout << line << endl;
	}
	check.close();
	return 0;
}