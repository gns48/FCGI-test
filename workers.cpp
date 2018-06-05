/**
 * @file   workers.cpp
 * @author Gleb Semenov <gleb.semenov@gmail.com>
 * @date   Sat Dec 20 18:11:07 2014
 * 
 * @brief  main program loop
 */

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <thread>
#include <mutex>
#include <boost/lexical_cast.hpp>
#include "statserver.hpp"

#define PARAMBUF_LENGTH 1024

class TControl {
    unsigned long   *m_perThreadCounter;
    std::thread::id *m_threadID;
    std::mutex      m_counterMutex;
    int   m_tcount;
    int   m_fcgiSocket;
    bool  m_debug;

public:
    TControl(int tcount, int fcgi_socket):
        m_fcgiSocket(fcgi_socket), m_tcount(tcount), m_debug(false)
    {
        m_perThreadCounter = new unsigned long[tcount];
        m_threadID = new std::thread::id[tcount];
        bzero(m_perThreadCounter, tcount*sizeof(unsigned long));
    }

    ~TControl() {
        delete [] m_perThreadCounter;
        delete [] m_threadID;
    }

    inline std::thread::id& setID(size_t i) {
        std::lock_guard<std::mutex> lock(m_counterMutex);
        return m_threadID[i] = std::this_thread::get_id();
    }

    inline std::thread::id& getID(size_t i) {
        return m_threadID[i];
    }

    inline int getSocket() {
        return m_fcgiSocket;
    }
    
    inline void addCounter(size_t number) {
        std::lock_guard<std::mutex> lock(m_counterMutex);
        ++m_perThreadCounter[number];
    }

    inline bool debugSwitch() {
        bool rv = m_debug;
        m_debug = !m_debug;
        return rv;
    }
    
    void doStatistics(int seconds) {
        unsigned long total = 0;
        int tcount = 0;
        std::lock_guard<std::mutex> lock(m_counterMutex);
        for(auto i = 0; i < m_tcount; i++) {
            if(m_debug) {
                std::cout << m_threadID[i] << ": "
                          << m_perThreadCounter[i] << std::endl;
            }
            if(m_perThreadCounter[i]) tcount++, total += m_perThreadCounter[i];
        }
        std::cout << "last " << seconds <<  " seconds load: "
                  << total << " requests processed by "
                  << tcount << " threads" << std::endl;
        if(m_debug) std::cout << std::endl;
        bzero(m_perThreadCounter, m_tcount*sizeof(unsigned long));
    }
};

static TControl *TC;

static void* worker_proc(void *addr) {
    size_t tcount = reinterpret_cast<size_t>(addr);
    FCGX_Request request;
    char params[PARAMBUF_LENGTH];
    int rv;
    static std::mutex acceptMutex;

    TC->setID(tcount);
    
    rv = FCGX_InitRequest(&request, TC->getSocket(), 0);
    if(rv < 0) {
        std::lock_guard<std::mutex> lock(acceptMutex);
        std::cout << "Thread " << TC->getID(tcount) << "is out. FCGX_InitRequest error: " << rv << std::endl;
        return addr;
    }

    
    
    while(1) {
        acceptMutex.lock();
        rv = FCGX_Accept_r(&request);
        acceptMutex.unlock();
        
        if(rv < 0) {
            std::lock_guard<std::mutex> lock(acceptMutex);
            std::cout << "Thread " << TC->getID(tcount) << "is out. FCGX_Accept_r error: " << rv << std::endl;
            break;
        }

        // parse QUERY_STRING
        // 1. Copy QUERY_STRING to the params buffer
        bzero(params, sizeof(params));

        const char *request_method  = FCGX_GetParam("REQUEST_METHOD", request.envp);
        const char *content_length  = FCGX_GetParam("CONTENT_LENGTH", request.envp);
        const char *query_string;
        size_t paramlen = 0;

        if(strncasecmp(request_method, "GET", 3) == 0) {
            
            query_string = FCGX_GetParam("QUERY_STRING", request.envp);
            if(query_string) paramlen = strlen(query_string);
        }
        else if(strncasecmp(request_method, "POST", 4) == 0) {
            if(content_length && strlen(content_length) > 0)
                paramlen = boost::lexical_cast<int>(content_length);
            if(paramlen) {
                // This is test! We use much less than 2Mb HTTP POST request size 
                if(paramlen > PARAMBUF_LENGTH-1) paramlen = PARAMBUF_LENGTH-1;
                FCGX_GetStr(params, paramlen, request.in);
                query_string = params;
            }
        }
        
        if(!paramlen)  // no data in request -- length error
            SetCGIStatus(411, request.out);
        else {
            std::string paramstr(query_string);
            std::string path;
            std::string jsonData;
            paramstr = urlDecode(paramstr);
            try {
                if(validateRequest(paramstr, path)) {
                    size_t len = stat2json(path, jsonData);
                    if(len) {
                        FCGX_PutS("Content-type: application/json\r\n", request.out);
                        FCGX_PutS(jsonData.c_str(), request.out);
                    }
                    else SetCGIStatus(500, request.out);
                }
                else SetCGIStatus(500, request.out);
            }
            catch (...) {
                SetCGIStatus(500, request.out);
            }
        }
        
        FCGX_Finish_r(&request);
        TC->addCounter(tcount);
    }
    
    return addr;
}

int runWorkers(int listenPort, int workerThreads, int statInterval, bool daemon_mode) {
    std::string port = ":";
    
    port += boost::lexical_cast<std::string>(listenPort);
    
    if(FCGX_Init() < 0) {
        std::cerr << "FCGX_Init error: " << errno << std::endl;
        return errno;
    }
    
    int fcgi_socket = FCGX_OpenSocket(port.c_str(), THREADS_HARDLIMIT);
    if(fcgi_socket < 0) {
        std::cerr << "FCGX_OpenSocket error: " << errno << std::endl;
        return errno;
    }
    
    TC = new TControl(workerThreads, fcgi_socket);
    for(size_t i = 0; i < workerThreads; i++) {
        std::thread thr(worker_proc, (void*)i);
        thr.detach();
    }
    
    while(1) {
        sleep(statInterval);
        TC->doStatistics(statInterval);
    }
    
    delete TC;
    
    return 0;
}

// {"Request%20type":"GetProperties","Path":"/home/r2d2/shared/notes.txt"}


