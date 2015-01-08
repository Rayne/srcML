#!/bin/bash

# test framework
source $(dirname "$0")/framework_test.sh

# test
##
# xml flag

define srcml <<- 'STDOUT'
	<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
	<unit xmlns="http://www.sdml.info/srcML/src" xmlns:cpp="http://www.sdml.info/srcML/cpp" revision="0.8.0" language="C++">
	</unit>
	STDOUT

createfile sub/a.cpp.xml "$srcml"

srcml2src -X srcml srcml)

srcml2src -X sub/a.cpp.xml

srcml2src -X -o sub/a.cpp.xml srcml

check sub/a.cpp.xml <<< "$srcml"

srcml2src -X sub/a.cpp.xml -o sub/b.cpp.xml

check sub/a.cpp.xml <<< "$srcml"
