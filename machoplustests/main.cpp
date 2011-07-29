//
//  main.cpp
//  machoplustests
//
//  Created by John Heaton on 7/29/11.
//  Copyright 2011 GJB Software. All rights reserved.
//

#include <iostream>
#include "MachOFile.h"

#include "bitstuff.h"

using namespace std;

int main (int argc, const char * argv[]) {
    MachOFile file("/bin/chmod");
    
    cout << "isOpen: " << file.isOpen() << endl;
    cout << "isValid: " << file.isValid() << endl;
    cout << "isFat: " << file.isFat() << endl;
    cout << "numSupportedCPUTypes: " << file.numSupportedCPUTypes() << endl;
    
    for(int i=0;i<file.numSupportedCPUTypes();++i) {
        uint32_t cVer = file.dylibCurrentVersion(file.supportedCPUTypes()[i]);
        
        cout << "offsetForCPUType[" << i << "]: " << file.offsetForCPUType(file.supportedCPUTypes()[i]) << endl;
        cout << "numSupportedCPUTypes[" << i << "]: " << file.supportedCPUTypes()[i] << endl;
        cout << "numLoadCommands[" << i << "]: " << file.numLoadCommands(file.supportedCPUTypes()[i]) << endl;
        cout << "dylibCurrentVersion[" << i << "]: " << cVer << endl;
    }
    
    //uint32_t size, index=0;
    //const char *cstrings = (const char *)file.getSectionPointer(file.supportedCPUTypes()[0], "__TEXT", "__cstring", &size);
//    while(cstrings != NULL && size > 0) {
//        unsigned long long len = strlen(cstrings)+1;
//        
//        printf("%s", cstrings);
//        if(cstrings[len-1] != '\n')
//            printf("\n");
//        
//        cstrings += len;
//        size -= len;
//        
//        //printf("%p\n", cstrings);
//        
//        while(!*cstrings) {
//            cstrings++, size--;
//            //printf("++ %p\n", cstrings);
//        }
//        
//        index++;
//    }
    
    file.close();
    
    return 0;
}