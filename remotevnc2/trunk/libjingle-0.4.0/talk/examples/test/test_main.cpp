#define TABLE_SIZE 100
#define getmax(a,b) ((a)>(b)?(a):(b))
#define str(x) #x
#define glue(a,b) a##b
#define ARRAY_SIZE TABLE_SIZE 
#ifndef __cplusplus
#error A C++ compiler is required
#endif

#include <stdio.h>
#include <iostream>
//#include <fstream>  /// these are derived from ostream, istream as cout and cin is
#include <fstream>
#include <string>
#include <string.h>
#include <conio.h>
using namespace std;
int main()
{

//#line 20 "assigning variable"
int a=2; 
#ifdef TABLE_SIZE
int table[ARRAY_SIZE];
table[0]=1;
#undef TABLE_SIZE
#endif
	
	cout << getmax(4,5) <<endl;
   cout<<str(text)<<endl;
   glue(c,out)<<"waleed"<<endl;
   cout<<"this is the line number "<<__LINE__<<endl;
	cout<<"this is the line number "<<__FILE__<<endl;
	cout<<"this is the line number "<<__DATE__<<endl;
	cout<<"this is the line number "<<__TIME__<<endl;
	ofstream myfile;
	myfile.open("newfile.txt");
	myfile<<"my name is waleed"<<endl;
	myfile.close();
	ifstream myfile2;
	string line;
	myfile2.open("newfile.txt");
	getline(myfile2,line);
	cout<<line<<endl;
	myfile.close();

//	strlen(line);
	ofstream myfile3;
		myfile3.open("waleed.dat",ios::out|ios::app|ios::binary);
		if(myfile3.is_open())
	myfile3.close();
	
   _getch();
	return 0;
}