#!/usr/bin/env python

##
# @file test.py
#
# @copyright Copyright (C) 2006-2014 SDML (www.srcML.org)
# 
# The srcML Toolkit is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# The srcML Toolkit is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with the srcML Toolkit; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

import sys
sys.path.append("../src")
import srcml
import difflib
import os
import ctypes


test_count = 0
error_count = 0
LIBC_PATH = ""
if sys.platform == "darwin" :
    LIBC_PATH = "libc.dylib"
elif sys.platform == "linux2" :
    LIBC_PATH = "libc.so.6"
else :
    LIBC_PATH = "msvcrt.dll"

libc = ctypes.cdll.LoadLibrary(LIBC_PATH)


if sys.platform == "win32" or sys.platform == "cygwin" :
    os.open = libc._open
    os.close = libc._close
    os.O_WRONLY = 1
    os.O_CREAT = 256
    os.O_RDONLY = 0

libc.fopen.restype = ctypes.c_void_p
libc.fopen.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
libc.fclose.restype = ctypes.c_int
libc.fclose.argtypes = [ctypes.c_void_p]

def verify_test(correct, output) :

    globals()['test_count'] += 1

    if sys.platform == "win32" or sys.platform == "cygwin" :
        correct = str(correct).replace("\r", "")
        output  = str(output).replace("\r", "")

    if str(correct) != str(output) :

        print str(globals()['test_count']) + "\t"
        for line in difflib.unified_diff(str(correct).split("\n"), str(output).split("\n")) :
            print line
        globals()['error_count'] += 1

# test versions
verify_test(srcml.SRCML_VERSION_NUMBER, srcml.version_number())
verify_test(srcml.SRCML_VERSION_STRING, srcml.version_string())

# test set/get archive
archive = srcml.srcml_archive()
archive.set_filename("project")
archive.set_language("C++")
archive.set_directory("dir")
archive.set_version("1.0")
verify_test("project", archive.get_filename())
verify_test("C++", archive.get_language())
verify_test("dir", archive.get_directory())
verify_test("1.0", archive.get_version())

archive.set_options(0)
archive.enable_option(1)
archive.enable_option(2)
verify_test(3, archive.get_options())
archive.disable_option(2)
verify_test(1, archive.get_options())
archive.set_options(2)
verify_test(2, archive.get_options())

archive.set_tabstop(4)
verify_test(4, archive.get_tabstop())

verify_test(7, archive.get_namespace_size());
verify_test("cpp", archive.get_namespace_prefix(1))
verify_test("cpp", archive.get_prefix_from_uri("http://www.sdml.info/srcML/cpp"))
verify_test("http://www.sdml.info/srcML/cpp", archive.get_namespace_uri(1))
verify_test("http://www.sdml.info/srcML/cpp", archive.get_uri_from_prefix("cpp"))

archive.register_macro("MACRO", "src:macro")
verify_test(1, archive.get_macro_list_size());
verify_test("MACRO", archive.get_macro_token(0))
verify_test("src:macro", archive.get_macro_token_type("MACRO"))
verify_test("src:macro", archive.get_macro_type(0))

archive.close()

file = open("a.foo", "w")
gen = file.write("")
file.close()
archive = srcml.srcml_archive()
archive.disable_option(srcml.SRCML_OPTION_TIMESTAMP | srcml.SRCML_OPTION_HASH)
archive.disable_option(srcml.SRCML_OPTION_ARCHIVE)
archive.register_file_extension("foo", "C++")
archive.register_namespace("s", "http://www.sdml.info/srcML/src")
archive.register_macro("MACRO", "src:macro")
archive.write_open_memory()
unit = srcml.srcml_unit(archive)
unit.parse_filename("a.foo")
archive.write_unit(unit)
archive.close()
os.remove("a.foo")
verify_test("""<s:unit xmlns:s="http://www.sdml.info/srcML/src" xmlns:cpp="http://www.sdml.info/srcML/cpp" language="C++"><macro-list token="MACRO" type="src:macro"/></s:unit>""", unit.get_xml())

# write/parse tests
src = "a;\n"
asrcml = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.sdml.info/srcML/src">

<unit xmlns:cpp="http://www.sdml.info/srcML/cpp" language="C++"><expr_stmt><expr><name>a</name></expr>;</expr_stmt>
</unit>

</unit>
"""

# filename
file = open("a.cpp", "w")
gen = file.write(src)
file.close()
archive = srcml.srcml_archive()
archive.disable_option(srcml.SRCML_OPTION_TIMESTAMP | srcml.SRCML_OPTION_HASH)
archive.write_open_filename("project.xml")
unit = srcml.srcml_unit(archive)
unit.parse_filename("a.cpp")
archive.write_unit(unit)
archive.close()

file = open("project.xml", "r")
gen = file.read()
file.close()
verify_test(asrcml, gen)
os.remove("a.cpp")
os.remove("project.xml")

# memory
archive = srcml.srcml_archive()
archive.disable_option(srcml.SRCML_OPTION_TIMESTAMP | srcml.SRCML_OPTION_HASH)
archive.write_open_memory()
unit = srcml.srcml_unit(archive)
unit.set_language("C++")
unit.parse_memory(src)
archive.write_unit(unit)
archive.close()

verify_test(asrcml, archive.srcML())

# fd
file = open("a.cpp", "w")
gen = file.write(src)
file.close()
archive = srcml.srcml_archive()
archive.disable_option(srcml.SRCML_OPTION_TIMESTAMP | srcml.SRCML_OPTION_HASH)
fd = os.open("project.xml", os.O_WRONLY | os.O_CREAT)
archive.write_open_fd(fd)
src_fd = os.open("a.cpp", os.O_RDONLY)
unit = srcml.srcml_unit(archive)
unit.set_language("C++")
unit.parse_fd(src_fd)
archive.write_unit(unit)
archive.close()
#os.close(src_fd)
os.close(fd)

file = open("project.xml", "r")
gen = file.read()
file.close()
verify_test(asrcml, gen)
os.remove("a.cpp")
os.remove("project.xml")

# FILE
file = open("a.cpp", "w")
gen = file.write(src)
file.close()
archive = srcml.srcml_archive()
archive.disable_option(srcml.SRCML_OPTION_TIMESTAMP | srcml.SRCML_OPTION_HASH)
file = libc.fopen("project.xml", "w")
archive.write_open_FILE(file)
src_file = libc.fopen("a.cpp", "r")
unit = srcml.srcml_unit(archive)
unit.set_language("C++")
unit.parse_FILE(src_file)
archive.write_unit(unit)
archive.close()
libc.fclose(src_file)
libc.fclose(file)

file = open("project.xml", "r")
gen = file.read()
file.close()
verify_test(asrcml, gen)
os.remove("a.cpp")
os.remove("project.xml")

# read/unparse

# filename
file = open("project.xml", "w")
gen = file.write(asrcml)
file.close()
archive = srcml.srcml_archive()
archive.read_open_filename("project.xml")
unit = archive.read_unit()
unit.unparse_filename("a.cpp")
archive.close()

file = open("a.cpp", "r")
gen = file.read()
file.close()
verify_test(src, gen)
os.remove("a.cpp")
os.remove("project.xml")

# memory
archive = srcml.srcml_archive()
archive.read_open_memory(asrcml)
unit = archive.read_unit()
unit.unparse_memory()
archive.close()
verify_test(src, unit.src())

# fd
file = open("project.xml", "w")
gen = file.write(asrcml)
file.close()
archive = srcml.srcml_archive()
fd = os.open("project.xml", os.O_RDONLY)
archive.read_open_fd(fd)
unit = archive.read_unit()
src_fd = os.open("a.cpp", os.O_WRONLY | os.O_CREAT)
unit.unparse_fd(src_fd)
archive.close()
os.close(src_fd)
os.close(fd)

file = open("a.cpp", "r")
gen = file.read()
file.close()
verify_test(src, gen)
os.remove("a.cpp")
os.remove("project.xml")

# FILE
file = open("project.xml", "w")
gen = file.write(asrcml)
file.close()
archive = srcml.srcml_archive()
file = libc.fopen("project.xml", "r")
archive.read_open_FILE(file)
unit = archive.read_unit()
src_file = libc.fopen("a.cpp", "w")
unit.unparse_FILE(src_file)
archive.close()
libc.fclose(src_file)
libc.fclose(file)

file = open("a.cpp", "r")
gen = file.read()
file.close()
verify_test(src, gen)
os.remove("a.cpp")
os.remove("project.xml")

src = "b;\n"
asrcml = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.sdml.info/srcML/src">

<unit xmlns:cpp="http://www.sdml.info/srcML/cpp" language="C++"><expr_stmt><expr><name>a</name></expr>;</expr_stmt>
</unit>

<unit xmlns:cpp="http://www.sdml.info/srcML/cpp" language="C++"><expr_stmt><expr><name>b</name></expr>;</expr_stmt>
</unit>

</unit>
"""

# clone
archive = srcml.srcml_archive()
archive.set_language("C++")
clone_archive = archive.clone()
verify_test("C++", clone_archive.get_language())
archive.close()

# transforms

# xpath
archive = srcml.srcml_archive()
archive.read_open_memory(asrcml)
archive.append_transform_xpath("//src:unit")
oarchive = archive.clone()
oarchive.write_open_memory()
archive.apply_transforms(oarchive)
oarchive.close()
archive.close()

verify_test(asrcml, oarchive.srcML())

# xslt filename
archive = srcml.srcml_archive()
archive.read_open_memory(asrcml)
archive.append_transform_xslt_filename("copy.xsl")
oarchive = archive.clone()
oarchive.write_open_memory()
archive.apply_transforms(oarchive)
oarchive.close()
archive.close()

verify_test(asrcml, oarchive.srcML())

# xslt memory
archive = srcml.srcml_archive()
archive.read_open_memory(asrcml)
f = open("copy.xsl", "r")
copy = f.read()
f.close()
archive.append_transform_xslt_memory(copy)
oarchive = archive.clone()
oarchive.write_open_memory()
archive.apply_transforms(oarchive)
oarchive.close()
archive.close()

verify_test(asrcml, oarchive.srcML())

# xslt FILE
archive = srcml.srcml_archive()
archive.read_open_memory(asrcml)
file = libc.fopen("copy.xsl", "r")
archive.append_transform_xslt_FILE(file)
libc.fclose(file)
oarchive = archive.clone()
oarchive.write_open_memory()
archive.apply_transforms(oarchive)
oarchive.close()
archive.close()

verify_test(asrcml, oarchive.srcML())

# xslt fd
archive = srcml.srcml_archive()
archive.read_open_memory(asrcml)
fd = os.open("copy.xsl", os.O_RDONLY)
archive.append_transform_xslt_fd(fd)
os.close(fd)
oarchive = archive.clone()
oarchive.write_open_memory()
archive.apply_transforms(oarchive)
oarchive.close()
archive.close()

verify_test(asrcml, oarchive.srcML())

# relaxng filename
archive = srcml.srcml_archive()
archive.read_open_memory(asrcml)
archive.append_transform_relaxng_filename("schema.rng")
oarchive = archive.clone()
oarchive.write_open_memory()
archive.apply_transforms(oarchive)
oarchive.close()
archive.close()

verify_test(asrcml, oarchive.srcML())

# relaxng memory
archive = srcml.srcml_archive()
archive.read_open_memory(asrcml)
f = open("schema.rng", "r")
schema = f.read()
f.close()
archive.append_transform_relaxng_memory(schema)
oarchive = archive.clone()
oarchive.write_open_memory()
archive.apply_transforms(oarchive)
oarchive.close()
archive.close()

verify_test(asrcml, oarchive.srcML())

# relaxng FILE
archive = srcml.srcml_archive()
archive.read_open_memory(asrcml)
file = libc.fopen("schema.rng", "r")
archive.append_transform_relaxng_FILE(file)
libc.fclose(file)
oarchive = archive.clone()
oarchive.write_open_memory()
archive.apply_transforms(oarchive)
oarchive.close()
archive.close()

verify_test(asrcml, oarchive.srcML())

# relaxng fd
archive = srcml.srcml_archive()
archive.read_open_memory(asrcml)
fd = os.open("schema.rng", os.O_RDONLY)
archive.append_transform_relaxng_fd(fd)
os.close(fd)
oarchive = archive.clone()
oarchive.write_open_memory()
archive.apply_transforms(oarchive)
oarchive.close()
archive.close()

verify_test(asrcml, oarchive.srcML())

# clear transforms
archive = srcml.srcml_archive()
archive.read_open_memory(asrcml)
archive.append_transform_xpath("//src:unit")
archive.append_transform_xslt_filename("copy.xsl")
archive.append_transform_relaxng_filename("schema.rng")
archive.clear_transforms()
oarchive = archive.clone()
oarchive.write_open_memory()
archive.apply_transforms(oarchive)
oarchive.close()
archive.close()

verify_test("""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.sdml.info/srcML/src"/>
""", oarchive.srcML())

# unit set/get
archive = srcml.srcml_archive()
unit = srcml.srcml_unit(archive)
unit.set_filename("b.cpp")
unit.set_language("C")
unit.set_directory("directory")
unit.set_version("1.1")
unit.set_timestamp("today")
unit.set_hash("0123456789abcdef")
verify_test("b.cpp", unit.get_filename())
verify_test("C", unit.get_language())
verify_test("directory", unit.get_directory())
verify_test("1.1", unit.get_version())
verify_test("today", unit.get_timestamp())
verify_test("0123456789abcdef", unit.get_hash())
archive.close()

# exception
archive = srcml.srcml_archive()
test = ""
try :
    archive.write_unit(unit)

except srcml.srcMLException as e :
    test = "Exception"
archive.close()
verify_test("Exception", test)

# cleanup_globals
srcml.cleanup_globals()

# test language list
verify_test(4, str(srcml.get_language_list_size()))
verify_test("C", str(srcml.get_language_list(0)))
verify_test("C++", str(srcml.get_language_list(1)))
verify_test("C#", str(srcml.get_language_list(2)))
verify_test("Java", str(srcml.get_language_list(3)))
verify_test(None, str(srcml.get_language_list(4)))

file = open("a.cpp", "w")
file.write("a;\n")
file.close()

srcml.srcml("a.cpp", "project.xml")

os.remove("a.cpp")
file = open("project.xml", "r")
xml = file.read()
file.close()

asrcml = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.sdml.info/srcML/src" xmlns:cpp="http://www.sdml.info/srcML/cpp" language="C++" filename="a.cpp"><expr_stmt><expr><name>a</name></expr>;</expr_stmt>
</unit>
"""
verify_test(asrcml, xml)

srcml.set_language("C++")
srcml.set_filename("a.cpp")
srcml.set_directory("directory")
srcml.set_version("version")
srcml.set_timestamp("timestamp")
srcml.set_hash("hash")

verify_test("C++", srcml.get_language())
verify_test("a.cpp", srcml.get_filename())
verify_test("directory", srcml.get_directory())
verify_test("version", srcml.get_version())
verify_test("timestamp", srcml.get_timestamp())
verify_test("hash", srcml.get_hash())

srcml.set_options(0)
srcml.enable_option(1)
verify_test(1, srcml.get_options())

srcml.set_options(2)
verify_test(2, srcml.get_options())

srcml.set_options(1 | 2)
srcml.disable_option(2)
verify_test(1, srcml.get_options())

srcml.set_tabstop(4)
verify_test(4, srcml.get_tabstop())

os.remove("project.xml")

file = open("a.foo", "w")
file.write("a;\n")
file.close()

srcml.set_language(None)
srcml.set_filename(None)
srcml.set_directory(None)
srcml.set_version(None)
srcml.set_options(srcml.SRCML_OPTION_XML_DECL | srcml.SRCML_OPTION_NAMESPACE_DECL)
srcml.set_tabstop(8)

verify_test(7, srcml.get_namespace_size());
verify_test("cpp", srcml.get_namespace_prefix(1))
verify_test("cpp", srcml.get_prefix_from_uri("http://www.sdml.info/srcML/cpp"))
verify_test("http://www.sdml.info/srcML/cpp", srcml.get_namespace_uri(1))
verify_test("http://www.sdml.info/srcML/cpp", srcml.get_uri_from_prefix("cpp"))

srcml.register_macro("MACRO", "src:macro")
verify_test(1, srcml.get_macro_list_size());
verify_test("MACRO", srcml.get_macro_token(0))
verify_test("src:macro", srcml.get_macro_token_type("MACRO"))
verify_test("src:macro", srcml.get_macro_type(0))

srcml.register_file_extension("foo", "C++")
srcml.register_namespace("s", "http://www.sdml.info/srcML/src")
srcml.srcml("a.foo", "project.xml")

file = open("project.xml", "r")
xml = file.read()
file.close()
os.remove("a.foo")

asrcml = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<s:unit xmlns:s="http://www.sdml.info/srcML/src" xmlns:cpp="http://www.sdml.info/srcML/cpp" language="C++" filename="a.foo" timestamp="timestamp" hash="hash"><macro-list token="MACRO" type="src:macro"/><s:expr_stmt><s:expr><s:name>a</s:name></s:expr>;</s:expr_stmt>
</s:unit>
"""

verify_test(asrcml, xml)

verify_test(2, srcml.check_language("C++"))
verify_test("C++", srcml.check_extension("a.cpp"))
verify_test(1, srcml.check_encoding("UTF-8"))
verify_test(1, srcml.check_xslt())
verify_test(1, srcml.check_exslt())
srcml.srcml("", "")
verify_test("No language provided.", srcml.error_string())

os.remove("project.xml")