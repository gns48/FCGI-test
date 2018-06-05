#!/bin/sh

if test -z "$1" ; then
   echo "Usage: $0 <path>"
   exit 1
fi  

URL=http://localhost/test.cgi?%7B%22Request%20type%22:%22GetProperties%22,%22Path%22:%22@@%22%7D 

for fn in `find $1` ; do echo "$URL" | sed "s|@@|$fn|" ; done | xargs wget -O -

