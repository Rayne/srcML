#!/bin/bash

# test framework
source $(dirname "$0")/framework_test.sh

# test on compressed files with bz2.gz extension
define src <<- 'STDOUT'

	a;
	STDOUT

define empty_output <<- 'STDOUT'
	<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
	<unit xmlns="http://www.srcML.org/srcML/src" revision="REVISION"/>
	STDOUT

define foutput <<- 'STDOUT'
	<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
	<unit xmlns="http://www.srcML.org/srcML/src" xmlns:cpp="http://www.srcML.org/srcML/cpp" revision="REVISION" language="C++" filename="archive/a.cpp.bz2.gz">
	<expr_stmt><expr><name>a</name></expr>;</expr_stmt>
	</unit>
	STDOUT

define output <<- 'STDOUT'
	<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
	<unit xmlns="http://www.srcML.org/srcML/src" xmlns:cpp="http://www.srcML.org/srcML/cpp" revision="REVISION" language="C++">
	<expr_stmt><expr><name>a</name></expr>;</expr_stmt>
	</unit>
	STDOUT

define archive_output <<- 'STDOUT'
	<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
	<unit xmlns="http://www.srcML.org/srcML/src" revision="REVISION">

	<unit xmlns:cpp="http://www.srcML.org/srcML/cpp" revision="REVISION" language="C++" filename="archive/a.cpp.bz2.gz" hash="1a2c5d67e6f651ae10b7673c53e8c502c97316d6">
	<expr_stmt><expr><name>a</name></expr>;</expr_stmt>
	</unit>

	</unit>
	STDOUT

xmlcheck "$archive_output"
xmlcheck "$foutput"
xmlcheck "$output"
xmlcheck "$empty_output"


createfile archive/a.cpp "$src"
bzip2 -c archive/a.cpp > archive/a.cpp.bz2
gzip -c archive/a.cpp.bz2 > archive/a.cpp.bz2.gz

createfile list.txt "archive/a.cpp.bz2.gz"
bzip2 -c list.txt > list.txt.bz2
gzip -c list.txt.bz2 > list.txt.bz2.gz

createfile empty.txt " "
bzip2 -c empty.txt > empty.txt.bz2
gzip -c empty.txt.bz2 > empty.txt.bz2.gz

# src --> srcml
srcml archive/a.cpp.bz2.gz -o archive/a.cpp.xml
check archive/a.cpp.xml "$foutput"

srcml archive/a.cpp.bz2.gz
check "$foutput"

srcml -l C++ < archive/a.cpp.bz2.gz
check "$output"

srcml -l C++ -o archive/a.cpp.xml < archive/a.cpp.bz2.gz
check archive/a.cpp.xml "$output"

# files from
srcml --files-from list.txt
check "$archive_output"

srcml --files-from list.txt.bz2.gz
check "$archive_output"

srcml --files-from list.txt -o archive/list.xml
check archive/list.xml "$archive_output"

srcml --files-from list.txt.bz2.gz -o archive/compressed_list.xml
check archive/compressed_list.xml "$archive_output"

# files from empty_archive
srcml --files-from empty.txt
check "$empty_output"

srcml --files-from empty.txt.bz2.gz
check "$empty_output"

srcml --files-from empty.txt -o archive/empty.xml
check archive/empty.xml "$empty_output"

srcml --files-from empty.txt.bz2.gz -o archive/compressed_empty.xml
check archive/compressed_empty.xml "$empty_output"


rmfile list.txt
rmfile list.txt.bz2
rmfile list.txt.bz2.gz
rmfile empty.txt
rmfile empty.txt.bz2
rmfile empty.txt.bz2.gz
rmfile archive/a.cpp
rmfile archive/a.cpp.bz2
rmfile archive/a.cpp.bz2.gz


# srcml --> src
srcml archive/a.cpp.xml
check "$src"

srcml archive/a.cpp.xml -o archive/a.cpp
check archive/a.cpp "$src"

srcml < archive/a.cpp.xml
check "$src"

srcml -o archive/a.cpp < archive/a.cpp.xml
check archive/a.cpp "$src"