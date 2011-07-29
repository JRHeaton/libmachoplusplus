//
//  MachOFile.h
//  libmacho++
//
//  Created by John Heaton on 7/29/11.
//  Copyright 2011 GJB Software. All rights reserved.
//

#ifndef MACHOFILE_H
#define MACHOFILE_H

#include <stdint.h>
#include <mach/machine.h>

class MachOFile {
public:
    MachOFile();
    MachOFile(const char *path);
    
    ~MachOFile();
    
    void open(const char *path);
    void close();
    
    bool isOpen();
    bool isValid();
    bool isFat();
    bool isDylib();
    
    uint32_t    numSupportedCPUTypes();
    cpu_type_t  *&supportedCPUTypes();
    
    bool cpuTypeIsLittleEndian(cpu_type_t &type);
    
    uint32_t offsetForCPUType(cpu_type_t &type);
    
    uint32_t numLoadCommands();
    uint32_t numLoadCommands(cpu_type_t &type);
    
    uint32_t dylibCurrentVersion(cpu_type_t &type);
    uint32_t dylibCompatVersion(cpu_type_t &type);
    
    void *getSectionPointer(cpu_type_t &type, const char *segment, const char *section, uint32_t *outputSize);
    
private:
    uint8_t flags_, *buffer_;
    uint64_t fsize_;
    cpu_type_t *s_types;
};

#endif /* MACHOFILE_H */
