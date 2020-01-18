#include <iostream>
#include "md5.hpp"
#include <cstdio>
 
using std::cout; using std::endl;
 
int main(int argc, char *argv[])
{
    cout << "md5 of 'Olivier NIVOIX': " << md5("Olivier NIVOIX") << endl;
    cout << "md5 of 'Hello World': " << md5("Hello World") << endl;
    
    MD5 test;
	FILE * pFile;
   unsigned char buffer [64];
   size_t result;

   pFile = fopen ("md5.hpp" , "r");
   if (pFile == NULL) perror ("Error opening file");
   else
   {
     while ( ! feof (pFile) )
     {
       result = fread (buffer,1,64,pFile);
       test.update(buffer, result);
     }
     fclose (pFile);
   }
   
   test.finalize();
   
   cout << "md5 of file "<< test.hexdigest() << endl;
   
   return 0;
}
