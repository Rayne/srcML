#!/bin/bash

# test framework
source $(dirname "$0")/framework_test.sh

# exit cleanup of generated files
trap "{ cleanup; }" EXIT

# check the result of the test
define output <<- 'STDOUT'
	<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
	<unit xmlns="http://www.sdml.info/srcML/src" xmlns:cpp="http://www.sdml.info/srcML/cpp" revision="0.8.0" language="C++"/>
	STDOUT

# conduct test
echo -n "" | src2srcml -l C++

check 3<<< "$output"


createfile sub/test.cpp ""

echo -n "" | src2srcml -l C++ -o sub/stuff.cpp

check sub/stuff.cpp 3<<< "$output"
