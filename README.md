Qualification test prepared for one of my emploers

Task: Create mutlithreaded web application that return stat(2) info 
      for given filename and perform perfomance tests

Input: json data like: {"Request type":"GetProperties","Path":"/etc/passwd"}
Result Json data like the folowing

{
   "Path" : "/etc/passwd",
   "Properties" : {
      "Name" : "passwd",
      "number of hard links" : 1,
      "owner ID" : 0,
      "owner group ID" : 0,
      "protection" : "-rw-r--r--",
      "size" : 2877,
      "time of last access" : "Sun Dec 21 23:16:08 2014\n",
      "time of last modification" : "Tue Nov  4 03:25:28 2014\n",
      "time of last status change" : "Tue Nov  4 03:25:28 2014\n",
      "type" : "regular file"
   }
}



1. Prerequisites (for linux)

   - c++11 - aware compiler (sufficiently new gcc or clang is good)
   - boost (default from Your linux distribution is good)
   - libfcgi (libfcgi-dev package for debian/ubuntu)
   - libjsoncpp (libjsoncpp-dev package for debian/ubuntu)

2. Compilaton

   Just unpack source tarball into fresh directory ad type 'make'

3. Nginx setup

   Nginx may come with different configuration files layouts.
   Here is the Ubuntu version.

   Add this lines to the server section of the /etc/nginx/sites-available/default file 
   (put them just after the "location ~ \.php$" section which is commented out by default)

        location ~ \.cgi$ {
               fastcgi_pass 127.0.0.1:9090;
               fastcgi_index index.php;
               include fastcgi_params;
        }

   Restart nginx:
   sudo /etc/init.d/nginx restart


4. Running

   You may type ./statserver --help

./statserver --help
0.1alpha
Allowed options:
  -h [ --help ]                    produce help message
  -v [ --version ]                 print version
  -p [ --listen-port ] arg (=4800) fast CGI listen port
  -d [ --daemon ]                  be a daemon after start
  -f [ --pidfile ] arg             pid file path (for daemon mode)
  -t [ --threads ] arg (=20)       number of working threads

   Tyoe 
   ./statserver -t 20 -p 9090

   The server will start and wait for connections.

   From any browser request the following URL:
   http://localhost/test.cgi?{"Request type":"GetProperties","Path":"/etc/passwd"}

   If You prefer to use command line tools like curl or wget than You have to encode special characters in URL 
   manually:

   wget -O - http://localhost/test.cgi?%7B%22Request%20type%22:%22GetProperties%22,%22Path%22:%22/etc/passwd%22%7D

   Output should be as following:

   {
   "Path" : "/etc/passwd",
   "Properties" : {
      "Name" : "passwd",
      "number of hard links" : 1,
      "owner ID" : 0,
      "owner group ID" : 0,
      "protection" : "-rw-r--r--",
      "size" : 2877,
      "time of last access" : "Sun Dec 21 23:16:08 2014\n",
      "time of last modification" : "Tue Nov  4 03:25:28 2014\n",
      "time of last status change" : "Tue Nov  4 03:25:28 2014\n",
      "type" : "regular file"
      }
   }

5. Performance testing:

gleb@raccoon:~/src/$ uname -a
Linux raccoon 3.13.0-43-generic #72-Ubuntu SMP Mon Dec 8 19:35:06 UTC 2014 x86_64 x86_64 x86_64 GNU/Linux

gleb@raccoon:~/src/$ cpuinfo
Intel(R) Core(TM) i5-3320M  Processor (Intel64 )
=====  Processor composition  =====
Processors(CPUs)  : 4
Packages(sockets) : 1
Cores per package : 2
Threads per core  : 2
=====  Processor identification  =====
Processor	Thread Id.	Core Id.	Package Id.
0       	0   		0   		0   
1       	1   		0   		0   
2       	0   		1   		0   
3       	1   		1   		0   
=====  Placement on packages  =====
Package Id.	Core Id.	Processors
0   		0,1		(0,1)(2,3)
=====  Cache sharing  =====
Cache	Size		Processors
L1	32  KB		(0,1)(2,3)
L2	256 KB		(0,1)(2,3)
L3	3   MB		(0,1,2,3)


gleb@raccoon:~/src/$ ab -n 1000 -c 200 http://localhost/test.cgi?\{%22Request%20type%22:%22GetProperties%22,%22Path%22:%22/etc/init.d%22\} 
This is ApacheBench, Version 2.3 <$Revision: 1528965 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking localhost (be patient)
Completed 100 requests
Completed 200 requests
Completed 300 requests
Completed 400 requests
Completed 500 requests
Completed 600 requests
Completed 700 requests
Completed 800 requests
Completed 900 requests
Completed 1000 requests
Finished 1000 requests


Server Software:        nginx/1.4.6
Server Hostname:        localhost
Server Port:            80

Document Path:          /test.cgi?{%22Request%20type%22:%22GetProperties%22,%22Path%22:%22/etc/init.d%22}
Document Length:        441 bytes

Concurrency Level:      200
Time taken for tests:   1.022 seconds
Complete requests:      1000
Failed requests:        0
Total transferred:      571000 bytes
HTML transferred:       441000 bytes
Requests per second:    978.65 [#/sec] (mean)
Time per request:       204.362 [ms] (mean)
Time per request:       1.022 [ms] (mean, across all concurrent requests)
Transfer rate:          545.71 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    1   1.5      0       6
Processing:     6   53 198.5     12    1014
Waiting:        6   53 198.4     12    1014
Total:          9   54 198.9     12    1017

Percentage of the requests served within a certain time (ms)
  50%     12
  66%     12
  75%     13
  80%     15
  90%     20
  95%     23
  98%   1016
  99%   1016
 100%   1017 (longest request)


Server output:

gleb@raccoon:~/src/$ ./statserver -p 9090 -t 20
last 60 seconds load: 1000 requests processed by 20 threads
last 60 seconds load: 0 requests processed by 0 threads

Known BUGS:
      - No handling for setUID/setGID bits in file mode processing routines
      - No enough handling for symlinks
      - No daemon mode
      - Input validator may crush if request format is bad (critical!).















