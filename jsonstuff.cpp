/**
 * @file   jsonstuff.cpp
 * @author Gleb Semenov <gleb.semenov@gmail.com>
 * @date   Sun Dec 21 20:39:57 2014
 * 
 * @brief  Json & stat stuff
 * 
 * 
 */

#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <cstdlib>
#include <cstring>
#include <boost/lexical_cast.hpp>
#include <json/json.h>
#include <libgen.h>
#include "statserver.hpp"

/** 
 * @fn bool validateRequest(paramstr, path)
 * 
 * @param request -- (in) request body (decoded)
 * @param path -- (out) file to stat()
 * 
 * @return request validation status. Path is not valid if false is returned
 */
bool validateRequest(const std::string& request, std::string& path) {
    Json::Value root;
    Json::Reader reader;
    bool rv = reader.parse(request, root, false);
    if(rv) {
        std::string req = root.get("Request type", "null").asString();
        rv = req == "GetProperties";
        if(rv) path = root.get("Path", "null").asString();
    }
    
    return rv;
}

static const char* fileType(mode_t mode) {
    switch (mode & S_IFMT) {
    case S_IFBLK:  return "block device";
    case S_IFCHR:  return "character device";
    case S_IFDIR:  return "directory";
    case S_IFIFO:  return "FIFO/pipe";
    case S_IFLNK:  return "symlink";
    case S_IFREG:  return "regular file";
    case S_IFSOCK: return "socket";
    }
    return "unknown?";
}

static char* fileProtection(mode_t mode, char* modeString) {
    switch (mode & S_IFMT) {
    case S_IFBLK:
        modeString[0] = 'b';
        break;
    case S_IFCHR:
        modeString[0] = 'c';
        break;
    case S_IFDIR:
        modeString[0] = 'd';
        break;
    case S_IFIFO:
        modeString[0] = 'f';
        break;
    case S_IFLNK:
        modeString[0] = 'l';
        break;
    case S_IFSOCK:
        modeString[0] = 's';
        break;
    default:
        modeString[0] = '-';
        break;
    }
    int i = 1;
    modeString[i++] = (mode & S_IRUSR) ? 'r' : '-';
    modeString[i++] = (mode & S_IWUSR) ? 'w' : '-';
    modeString[i++] = (mode & S_IXUSR) ? 'x' : '-';
    modeString[i++] = (mode & S_IRGRP) ? 'r' : '-';
    modeString[i++] = (mode & S_IWGRP) ? 'w' : '-';
    modeString[i++] = (mode & S_IXGRP) ? 'x' : '-';
    modeString[i++] = (mode & S_IROTH) ? 'r' : '-';
    modeString[i++] = (mode & S_IWOTH) ? 'w' : '-';
    modeString[i++] = (mode & S_IXOTH) ? 'x' : '-';
    modeString[i] = 0;
    return modeString;
}


/** 
 * @fn size_t stat2json(path, jsonData);
 * 
 * @param path -- (in) file to stat()  
 * @param jsonData -- (out) stat data serialized to json
 * 
 * @return jsonData size
 */
size_t stat2json(const std::string& path, std::string& jsonData) {
    if(access(path.c_str(), R_OK) == 0) { // file exists and is readable
        struct stat st;
        if(stat(path.c_str(), &st) == 0) {
            Json::Value root;
            char tmstr[64];
            char bname[path.length()+1];
            strcpy(bname, path.c_str());
            
            /*
              struct stat {
               dev_t     st_dev;     // ID of device containing file 
               ino_t     st_ino;     // inode number 
               mode_t    st_mode;    // protection 
               nlink_t   st_nlink;   // number of hard links 
               uid_t     st_uid;     // user ID of owner 
               gid_t     st_gid;     // group ID of owner 
               dev_t     st_rdev;    // device ID (if special file) 
               off_t     st_size;    // total size, in bytes 
               blksize_t st_blksize; // blocksize for filesystem I/O 
               blkcnt_t  st_blocks;  // number of 512B blocks allocated 
               time_t    st_atime;   // time of last access 
               time_t    st_mtime;   // time of last modification 
               time_t    st_ctime;   // time of last status change 
               };
            */
            root["Path"] = path;
            root["Properties"]["Name"] = basename(bname);
            root["Properties"]["type"] = fileType(st.st_mode);
            // it is safe to assign the same char pointer: constructor has copy semantics
            root["Properties"]["time of last access"] = ctime_r(&st.st_atime, tmstr);
            root["Properties"]["time of last modification"] = ctime_r(&st.st_mtime, tmstr);
            root["Properties"]["time of last status change"] = ctime_r(&st.st_ctime, tmstr);
            root["Properties"]["protection"] = fileProtection(st.st_mode, bname);
            root["Properties"]["number of hard links"] = (Json::UInt64)st.st_nlink;
            root["Properties"]["owner ID"] = st.st_uid;
            root["Properties"]["owner group ID"] = st.st_gid;
            root["Properties"]["size"] = (Json::UInt64)st.st_size;
            Json::StyledWriter writer;
            jsonData = writer.write(root);
            return jsonData.length();
        }
    }    
    return 0;
}




