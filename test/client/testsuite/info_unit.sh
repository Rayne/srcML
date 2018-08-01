#!/bin/bash

# test framework
source $(dirname "$0")/framework_test.sh

# test

##
# srcML info and longinfo

define info <<- 'STDOUT'
	xmlns="http://www.srcML.org/srcML/src"
	xmlns:cpp="http://www.srcML.org/srcML/cpp"
	encoding="UTF-8"
	language="C++"
	url="test"
	filename="sub/unit.cpp"
	STDOUT

define longinfo <<- 'STDOUT'
	xmlns="http://www.srcML.org/srcML/src"
	xmlns:cpp="http://www.srcML.org/srcML/cpp"
	encoding="UTF-8"
	language="C++"
	url="test"
	filename="sub/unit.cpp"
	units="1"
	STDOUT

define srcml <<- 'STDOUT'
	<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
	<unit xmlns="http://www.srcML.org/srcML/src" xmlns:cpp="http://www.srcML.org/srcML/cpp" revision="REVISION" language="C++" url="test" filename="sub/unit.cpp"><expr_stmt><expr><name>a</name></expr>;</expr_stmt></unit>
	STDOUT

xmlcheck "$srcml"
createfile sub/unit.cpp.xml "$srcml"

srcml sub/unit.cpp.xml -i
check "$info"

srcml sub/unit.cpp.xml --info
check "$info"

srcml --info < sub/unit.cpp.xml
check "$info"

srcml -i sub/unit.cpp.xml
check "$info"

srcml --info sub/unit.cpp.xml
check "$info"

srcml --all-info sub/unit.cpp.xml
check "$longinfo"

srcml sub/unit.cpp.xml --all-info
check "$longinfo"

srcml --all-info < sub/unit.cpp.xml
check "$longinfo"
