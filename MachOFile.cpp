//
//  MachOFile.cpp
//  libmacho++
//
//  Created by John Heaton on 7/29/11.
//  Copyright 2011 GJB Software. All rights reserved.
//

#include <fstream>
#include <string.h>
#include <sys/stat.h>
#include <mach-o/fat.h>
#include <mach-o/loader.h>

#include "MachOFile.h"
#include "bitstuff.h"

using namespace std;

enum {
    FLAG_VALID = (1 << 0),
    FLAG_OPEN = (1 << 1)
};

#define IS_LE_H(hdr) ((hdr)->magic == MH_MAGIC || (hdr)->magic == MH_MAGIC_64)

MachOFile::MachOFile() {
    buffer_ = NULL;
    flags_ = 0;
    s_types = NULL;
}

MachOFile::MachOFile(const char *path) {
    buffer_ = NULL;
    flags_ = 0;
    s_types = NULL;
    
    this->open(path);
}

MachOFile::~MachOFile() {
    this->close();
}

void MachOFile::open(const char *path) {
    struct stat st;
    ifstream stream;
    
    if(this->isOpen())
        return;
    
    if(stat(path, &st) != 0) 
        return;
    
    fsize_ = st.st_size;
    stream.open(path);
    if(!stream.good()) 
        return;
    
    buffer_ = new unsigned char[fsize_];
    
    stream.read((char *)buffer_, fsize_);
    stream.close();
    
    ENABLE_FLAG(flags_, FLAG_OPEN);
    
    bool fat = (ESWAP32(*(uint32_t *)buffer_) == FAT_MAGIC);
    
    if(*(uint32_t *)buffer_ == MH_MAGIC || *(uint32_t *)buffer_ == MH_MAGIC_64 || fat) {
        ENABLE_FLAG(flags_, FLAG_VALID);
    }
}

void MachOFile::close() {
    if(!this->isOpen())
        return;
    
    if(buffer_) 
        delete [] buffer_;
    
    buffer_ = NULL;
    flags_ = 0;
    
    if(s_types != NULL) {
        if(isFat())
            delete [] s_types;
        else
            delete s_types;
    }
    
    s_types = NULL;
}

bool MachOFile::isOpen() {
    return CHECK_FLAG(flags_, FLAG_OPEN);
}

bool MachOFile::isValid() {
    return CHECK_FLAG(flags_, FLAG_VALID);
}

bool MachOFile::isFat() { // eat your cheezburger fatty
    if(!this->isValid())
        return false;
    
    return *(uint32_t *)buffer_ == FAT_CIGAM;
}

bool MachOFile::isDylib() {
    struct mach_header *hdr = (struct mach_header *)(buffer_+this->offsetForCPUType(this->supportedCPUTypes()[0]));
    
    return (IS_LE_H(hdr) ? hdr->filetype == MH_DYLIB : ESWAP32(hdr->filetype) == MH_DYLIB);
}

uint32_t MachOFile::numSupportedCPUTypes() {
    if(!this->isValid())
        return 0;
    
    if(this->isFat()) {
        return ESWAP32(((struct fat_header *)buffer_)->nfat_arch);
    }
    
    return 1;
}

cpu_type_t *&MachOFile::supportedCPUTypes() {
    if(!this->isValid())
        return s_types;
    
    if(this->isFat()) {
        if(s_types != NULL)
            return s_types;
        
        uint8_t *navBuf;
        s_types = new cpu_type_t[numSupportedCPUTypes()];
        
        navBuf=(buffer_+sizeof(struct fat_header));
        for(int i=0;i<numSupportedCPUTypes();++i) {
            s_types[i] = ESWAP32(((struct fat_arch *)navBuf)->cputype);
            navBuf += sizeof(struct fat_arch);
        }
    } else {
        if(s_types != NULL)
            return s_types;
        
        s_types = new cpu_type_t;
        s_types[0] = ((struct mach_header *)buffer_)->cputype;
    }
    
    return s_types;
}

bool MachOFile::cpuTypeIsLittleEndian(cpu_type_t &type) {
    return IS_LE_H((struct mach_header *)(buffer_+this->offsetForCPUType(type)));
}

uint32_t MachOFile::offsetForCPUType(cpu_type_t &type) {
    if(!this->isValid())
        return 0;
    
    if(this->isFat()) {
        uint8_t *nBuf = (buffer_+sizeof(struct fat_header));
        for(int i=0;i<this->numSupportedCPUTypes();++i) { 
            struct fat_arch *cArch = (struct fat_arch *)nBuf;
            if(ESWAP32(cArch->cputype) == type) {
                return ESWAP32(cArch->offset);
            }
            
            nBuf += sizeof(struct fat_arch);
        }
    }
    
    return 0;
}

uint32_t MachOFile::numLoadCommands() {
    return this->numLoadCommands(this->supportedCPUTypes()[0]);
}

uint32_t MachOFile::numLoadCommands(cpu_type_t &type) {
    if(!this->isValid())
        return 0;
    
    struct mach_header *hdr =  (struct mach_header *)(buffer_+this->offsetForCPUType(type));
    
    return (IS_LE_H(hdr) ? hdr->ncmds : ESWAP32(hdr->ncmds));
}

uint32_t MachOFile::dylibCurrentVersion(cpu_type_t &type) {
    if(!this->isDylib())
        return 0;
    
    struct mach_header *hdr = (struct mach_header *)(buffer_+this->offsetForCPUType(type));
    bool archIsLE = IS_LE_H(hdr);
    uint8_t *nBuf = (uint8_t *)(buffer_+(archIsLE ? (hdr->magic == MH_MAGIC_64 ? sizeof(struct mach_header_64) : sizeof(struct mach_header)) : (ESWAP32(hdr->magic) == MH_MAGIC_64 ? sizeof(struct mach_header_64) : sizeof(struct mach_header))));
    
    for(int i=0;i<this->numLoadCommands(type);++i) {
        struct load_command *cmd = (struct load_command *)(nBuf);
        
        switch(archIsLE ? cmd->cmd : ESWAP32(cmd->cmd)) {
            case LC_ID_DYLIB: return ((struct dylib_command *)cmd)->dylib.current_version; break;
        }
    }
    
    return 0;
}

uint32_t MachOFile::dylibCompatVersion(cpu_type_t &type) {
    if(!this->isDylib())
        return 0;
    
    struct mach_header *hdr = (struct mach_header *)(buffer_+this->offsetForCPUType(type));
    bool archIsLE = IS_LE_H(hdr);
    uint8_t *nBuf = (uint8_t *)(buffer_+(archIsLE ? (hdr->magic == MH_MAGIC_64 ? sizeof(struct mach_header_64) : sizeof(struct mach_header)) : (ESWAP32(hdr->magic) == MH_MAGIC_64 ? sizeof(struct mach_header_64) : sizeof(struct mach_header))));
    
    for(int i=0;i<this->numLoadCommands(type);++i) {
        struct load_command *cmd = (struct load_command *)(nBuf);
        
        switch(archIsLE ? cmd->cmd : ESWAP32(cmd->cmd)) {
            case LC_ID_DYLIB: return ((struct dylib_command *)cmd)->dylib.compatibility_version; break;
        }
    }
    
    return 0;
}

void *MachOFile::getSectionPointer(cpu_type_t &type, const char *segment, const char *sectionname, uint32_t *outputSize) {
    if(!this->isValid())
        return NULL;
    
    uint32_t fileoff = this->offsetForCPUType(type);
    struct mach_header *hdr = (struct mach_header *)(buffer_+fileoff);
    bool archIsLE = IS_LE_H(hdr);
    uint8_t *nBuf = (uint8_t *)(buffer_+fileoff+(archIsLE ? (hdr->magic == MH_MAGIC_64 ? sizeof(struct mach_header_64) : sizeof(struct mach_header)) : (ESWAP32(hdr->magic) == MH_MAGIC_64 ? sizeof(struct mach_header_64) : sizeof(struct mach_header))));
    
    for(int i=0;i<this->numLoadCommands(type);++i) {
        struct load_command *cmd = (struct load_command *)(nBuf);
        
        switch(archIsLE ? cmd->cmd : ESWAP32(cmd->cmd)) {
            case LC_SEGMENT: {
                struct segment_command *seg = (struct segment_command *)cmd;
                
                uint32_t offset=0;
                if(strcmp(seg->segname, segment) == 0) {
                    for(int i=0;i<(archIsLE ? seg->nsects : ESWAP32(seg->nsects));++i) {
                        struct section *section = (struct section *)(nBuf+sizeof(struct segment_command)+offset);
                        
                        if(strcmp(section->sectname, sectionname) == 0) {
                            *outputSize = section->size;
                            return (void *)(buffer_+fileoff+(archIsLE ? section->offset : ESWAP32(section->offset)));
                        }
                        
                        offset += sizeof(struct section);
                    }
                }
            } break;
            case LC_SEGMENT_64: {
                struct segment_command_64 *seg = (struct segment_command_64 *)cmd;
                
                uint32_t offset=0;
                if(strcmp(seg->segname, segment) == 0) {
                    for(int i=0;i<(archIsLE ? seg->nsects : ESWAP32(seg->nsects));++i) {
                        struct section_64 *section = (struct section_64 *)(nBuf+sizeof(struct segment_command_64)+offset);
                        
                        if(strcmp(section->sectname, sectionname) == 0) {
                            return (void *)(buffer_+fileoff+(archIsLE ? section->offset : ESWAP32(section->offset)));
                        }
                        
                        offset += sizeof(struct section_64);
                    }
                }
            } break;
        }
        
        nBuf += (archIsLE ? cmd->cmdsize : ESWAP32(cmd->cmdsize));
    }
    
    return 0;
}