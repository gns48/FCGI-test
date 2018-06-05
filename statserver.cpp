/**
 * @file   statserver.cpp
 * @author Gleb Semenov <gleb.semenov@gmail.com>
 * @date   Sat Dec 20 17:05:01 MSK 2014
 *
 * @brief  main module
 */

#include <cerrno>
#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "statserver.hpp"

namespace po = boost::program_options;

const char* serverVersion = "0.1alpha";    

int main(int ac, char** av) {
    int rv = 0;

    // command line option variables
    int listenPort = 4800;  // default listen port

    bool daemon_mode = false; // daemonize?
    std::string pidfile("/tmp/statserver.pid");
    
    int workerThreads = 20;   // Number of threads to create
    int statInterval = 60;

    // boost::program_options staff
    po::options_description op_cmdline("Allowed options");
    op_cmdline.add_options()
        ("help,h", "produce help message")
        ("version,v", "print version")
        ("listen-port,p", po::value<int>(&listenPort)->default_value(4800), "fast CGI listen port")
        ("daemon,d",
         po::value<bool>(&daemon_mode)->zero_tokens()->default_value(false)->implicit_value(true),
         "be a daemon after start")
        ("pidfile,f", po::value<std::string>(&pidfile), "pid file path (for daemon mode)")
        ("threads,t", po::value<int>(&workerThreads)->default_value(20), "number of working threads")
        ("interval,i", po::value<int>(&statInterval)->default_value(60), "Statistics counting interval, seconds")
        ;

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(ac, av, op_cmdline), vm);
        po::notify(vm);
    }
    catch(po::error &err) {
        std::cerr << "command line option error: " << err.what() << std::endl;
        std::cout << op_cmdline << std::endl;
        return EINVAL;
    }

    if(vm.count("help")) {
        std::cout << serverVersion << std::endl
                  << op_cmdline << std::endl;
        return 0;
    }
    
    if(vm.count("version")) {
        std::cout << serverVersion << std::endl;
        return 0;
    }

/*    
    if(daemon_mode) {
        daemonize(pidfile);
        write_pid(pidfile.c_str());
    }
*/
    if(workerThreads < 0 || workerThreads > THREADS_HARDLIMIT) {
        std::cerr << "Invalid threads count specified!" << std::endl;
        return EINVAL;
    }
    
    runWorkers(listenPort, workerThreads, statInterval, daemon_mode);
    
    if(daemon_mode) unlink(pidfile.c_str());
    
    return 0;
}










