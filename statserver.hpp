/**
 * @file   statserver.hpp
 * @author Gleb Semenov <gleb.semenov@gmail.com>
 * @date   Sat Dec 20 17:22:40 2014
 * 
 * @brief  common definitions and prototypes
 * 
 * 
 */

#include <fcgiapp.h>

#define THREADS_HARDLIMIT 1024

/** 
 * @fn int runWorkers(short port, int wcount, bool daemonMode);
 * @brief 'main' function that does all the work
 * @param port - listening port
 * @param wcount - number of threads to create
 * @param statInterval - statistics counting interval
 * @param daemonMode - do we are running as a daemon
 * 
 * @return 0 if no error, else error code
 */
int runWorkers(int port, int wcount, int statInterval, bool daemonMode);

/** 
 * @fn void SetCGIStatus(const int code, FCGX_Stream *stream)
 * @brief prints CGI status message like "Status: 200 OK" aand sets CGI exit status
 * @param code - HTTP status code 
 * @param stream - FCGI output stream
 * @return none
 */
void SetCGIStatus(const int code, FCGX_Stream *stream);

/** 
 * @fn std::string urlDecode(std::string &SRC);
 * 
 * @param SRC - reference to URL to decode
 *
 * @return decoded URL
 */
std::string urlDecode(std::string &SRC);

/** 
 * @fn bool validateRequest(paramstr, path)
 * 
 * @param request -- (in) request body (decoded)
 * @param path -- (out) file to stat()
 * 
 * @return request validation status. Path is not valid if false is returned
 */
bool validateRequest(const std::string& request, std::string& path);

/** 
 * @fn size_t stat2json(path, jsonData);
 * 
 * @param path -- (in) file to stat()  
 * @param jsonData -- (out) stat data serialized to json
 * 
 * @return jsonData size
 */
size_t stat2json(const std::string& path, std::string& jsonData);







