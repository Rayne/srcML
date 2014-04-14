/**
 * @file test_srcml_write_unit.cpp
 *
 * @copyright Copyright (C) 2013-2014 SDML (www.srcML.org)
 *
 * The srcML Toolkit is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * The srcML Toolkit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the srcML Toolkit; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*

  Test cases for srcml_archive_check_extension
*/
#include <stdio.h>
#include <string.h>
#include <cassert>

#include <srcml.h>
#include <srcml_types.hpp>
#include <srcmlns.hpp>

#include "dassert.hpp"

int main() {

    std::string xml_decl ="<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    std::string empty_unit = "<unit xmlns=\"http://www.sdml.info/srcML/src\" language=\"C++\" dir=\"dir\" filename=\"a.cpp\" version=\"1\" timestamp=\"today\" hash=\"0123456789abcdef\"/>\n";
    std::string empty_inner_unit = "<unit language=\"C++\" dir=\"dir\" filename=\"a.cpp\" version=\"1\" timestamp=\"today\" hash=\"0123456789abcdef\"/>\n";
    std::string start_root_unit_tag = "<unit xmlns=\"http://www.sdml.info/srcML/src\">\n\n";
    std::string end_root_unit_tag = "\n</unit>\n";

    std::string start_unit = "<unit xmlns=\"http://www.sdml.info/srcML/src\">";
    std::string end_unit = "</unit>\n";



    /*
      srcml_write_start_unit
    */

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_memory(archive, &s, &size);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "a.cpp");
        srcml_unit_set_directory(unit, "dir");
        srcml_unit_set_version(unit, "1");
        srcml_unit_set_timestamp(unit, "today");
        srcml_unit_set_hash(unit, "0123456789abcdef");
        srcml_write_start_unit(archive, unit);
        srcml_write_end_unit(archive);
        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

        dassert(s, xml_decl + empty_unit);
        free(s);
    }

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_memory(archive, &s, &size);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "a.cpp");
        srcml_unit_set_directory(unit, "dir");
        srcml_unit_set_version(unit, "1");
        srcml_unit_set_timestamp(unit, "today");
        srcml_unit_set_hash(unit, "0123456789abcdef");
        srcml_write_start_unit(archive, unit);
        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

        dassert(s, xml_decl + empty_unit);
        free(s);
    }

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_write_open_memory(archive, &s, &size);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "a.cpp");
        srcml_unit_set_directory(unit, "dir");
        srcml_unit_set_version(unit, "1");
        srcml_unit_set_timestamp(unit, "today");
        srcml_unit_set_hash(unit, "0123456789abcdef");
        srcml_write_start_unit(archive, unit);
        srcml_write_end_unit(archive);
        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

        dassert(s, xml_decl + start_root_unit_tag + empty_inner_unit + end_root_unit_tag);
        free(s);
    }

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_write_open_memory(archive, &s, &size);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "a.cpp");
        srcml_unit_set_directory(unit, "dir");
        srcml_unit_set_version(unit, "1");
        srcml_unit_set_timestamp(unit, "today");
        srcml_unit_set_hash(unit, "0123456789abcdef");
        srcml_write_start_unit(archive, unit);
        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

        dassert(s, xml_decl + start_root_unit_tag + empty_inner_unit + end_root_unit_tag);
        free(s);
    }

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_write_open_memory(archive, &s, &size);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "a.cpp");
        srcml_unit_set_directory(unit, "dir");
        srcml_unit_set_version(unit, "1");
        srcml_unit_set_timestamp(unit, "today");
        srcml_unit_set_hash(unit, "0123456789abcdef");
        srcml_write_start_unit(archive, unit);
        srcml_write_end_unit(archive);
        srcml_write_start_unit(archive, unit);
        srcml_write_end_unit(archive);
        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

        dassert(s, xml_decl + start_root_unit_tag + empty_inner_unit + "\n" + empty_inner_unit + end_root_unit_tag);
        free(s);
    }

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_write_open_memory(archive, &s, &size);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "a.cpp");
        srcml_unit_set_directory(unit, "dir");
        srcml_unit_set_version(unit, "1");
        srcml_unit_set_timestamp(unit, "today");
        srcml_unit_set_hash(unit, "0123456789abcdef");
        srcml_write_start_unit(archive, unit);
        srcml_write_end_unit(archive);
        srcml_write_start_unit(archive, unit);
        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

        dassert(s, xml_decl + start_root_unit_tag + empty_inner_unit + "\n" + empty_inner_unit + end_root_unit_tag);
        free(s);
    }

    {
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "a.cpp");
        srcml_unit_set_directory(unit, "dir");
        srcml_unit_set_version(unit, "1");
        srcml_unit_set_timestamp(unit, "today");
        srcml_unit_set_hash(unit, "0123456789abcdef");
        dassert(srcml_write_start_unit(archive, unit), SRCML_STATUS_INVALID_IO_OPERATION);
        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);
    }

    {
        srcml_unit * unit = srcml_create_unit(0);
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "a.cpp");
        srcml_unit_set_directory(unit, "dir");
        srcml_unit_set_version(unit, "1");
        srcml_unit_set_timestamp(unit, "today");
        srcml_unit_set_hash(unit, "0123456789abcdef");
        dassert(srcml_write_start_unit(0, unit), SRCML_STATUS_INVALID_ARGUMENT);
        srcml_free_unit(unit);
     }

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_write_open_memory(archive, &s, &size);
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        dassert(srcml_write_start_unit(archive, 0), SRCML_STATUS_INVALID_ARGUMENT);
        srcml_close_archive(archive);
        srcml_free_archive(archive);
    }

    /*
      srcml_write_end_unit
    */

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_memory(archive, &s, &size);
        dassert(srcml_write_end_unit(archive), SRCML_STATUS_INVALID_INPUT);
        srcml_close_archive(archive);
        srcml_free_archive(archive);
    }

    {
        dassert(srcml_write_end_unit(0), SRCML_STATUS_INVALID_ARGUMENT);
    }

    /*
        srcml_write_start_element
    */

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_memory(archive, &s, &size);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_write_start_unit(archive, unit);
        srcml_write_start_element(archive, 0, "element", 0);
        srcml_write_end_element(archive);
        srcml_write_end_unit(archive);
        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

        dassert(s, xml_decl + start_unit + "<element/>" + end_unit);
        free(s);
    }

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_memory(archive, &s, &size);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_write_start_unit(archive, unit);
        srcml_write_start_element(archive, 0, "element", 0);
        srcml_write_end_unit(archive);
        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

        dassert(s, xml_decl + start_unit + "<element/>" + end_unit);
        free(s);
    }

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_memory(archive, &s, &size);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_write_start_unit(archive, unit);
        srcml_write_start_element(archive, "foo", "element", 0);
        srcml_write_end_element(archive);
        srcml_write_end_unit(archive);
        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

        dassert(s, xml_decl + start_unit + "<foo:element/>" + end_unit);
        free(s);
    }

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_memory(archive, &s, &size);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_write_start_unit(archive, unit);
        srcml_write_start_element(archive, 0, "element", "bar");
        srcml_write_end_element(archive);
        srcml_write_end_unit(archive);
        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

        dassert(s, xml_decl + start_unit + "<element xmlns=\"bar\"/>" + end_unit);
        free(s);
    }


    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_memory(archive, &s, &size);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_write_start_unit(archive, unit);
        srcml_write_start_element(archive, "foo", "element", "bar");
        srcml_write_end_element(archive);
        srcml_write_end_unit(archive);
        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

        dassert(s, xml_decl + start_unit + "<foo:element xmlns:foo=\"bar\"/>" + end_unit);
        free(s);
    }

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_memory(archive, &s, &size);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_write_start_unit(archive, unit);
        srcml_write_start_element(archive, 0, "element", 0);
        srcml_write_end_element(archive);
        srcml_write_start_element(archive, 0, "element", 0);
        srcml_write_end_element(archive);
        srcml_write_end_unit(archive);
        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

        dassert(s, xml_decl + start_unit + "<element/><element/>" + end_unit);
        free(s);
    }

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_memory(archive, &s, &size);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_write_start_unit(archive, unit);
        srcml_write_start_element(archive, 0, "element", 0);
        srcml_write_end_element(archive);
        srcml_write_start_element(archive, 0, "element", 0);
        srcml_write_end_unit(archive);
        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

        dassert(s, xml_decl + start_unit + "<element/><element/>" + end_unit);
        free(s);
    }

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_memory(archive, &s, &size);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_write_start_unit(archive, unit);
        srcml_write_start_element(archive, 0, "element", 0);
        srcml_write_start_element(archive, 0, "element", 0);
        srcml_write_end_element(archive);
        srcml_write_end_element(archive);
        srcml_write_end_unit(archive);
        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

        dassert(s, xml_decl + start_unit + "<element><element/></element>" + end_unit);
        free(s);
    }

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_memory(archive, &s, &size);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_write_start_unit(archive, unit);
        srcml_write_start_element(archive, 0, "element", 0);
        srcml_write_start_element(archive, 0, "element", 0);
        srcml_write_end_unit(archive);
        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

        dassert(s, xml_decl + start_unit + "<element><element/></element>" + end_unit);
        free(s);
    }

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_memory(archive, &s, &size);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_write_start_unit(archive, unit);
        dassert(srcml_write_start_element(archive, 0, 0, 0), SRCML_STATUS_INVALID_ARGUMENT);
        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);
        free(s);
    }

    {
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        dassert(srcml_write_start_element(archive, 0, "element", 0), SRCML_STATUS_INVALID_IO_OPERATION);
        srcml_close_archive(archive);
        srcml_free_archive(archive);
    }

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_memory(archive, &s, &size);
        dassert(srcml_write_start_element(archive, 0, "element", 0), SRCML_STATUS_INVALID_INPUT);
        srcml_close_archive(archive);
        srcml_free_archive(archive);
    }

    
    {
        dassert(srcml_write_start_element(0, "foo", "element", "bar"), SRCML_STATUS_INVALID_ARGUMENT);
    }

    /*
        srcml_write_end_element
    */

    {
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        dassert(srcml_write_end_element(archive), SRCML_STATUS_INVALID_IO_OPERATION);
        srcml_close_archive(archive);
        srcml_free_archive(archive);
    }

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_memory(archive, &s, &size);
        dassert(srcml_write_end_element(archive), SRCML_STATUS_INVALID_INPUT);
        srcml_close_archive(archive);
        srcml_free_archive(archive);
    }
    
    {
        dassert(srcml_write_end_element(0), SRCML_STATUS_INVALID_ARGUMENT);
    }

    /*
        srcml_write_namespace
    */

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_memory(archive, &s, &size);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_write_start_unit(archive, unit);
        srcml_write_start_element(archive, 0, "element", 0);
        srcml_write_namespace(archive, "foo", "bar");
        srcml_write_end_element(archive);
        srcml_write_end_unit(archive);
        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

        dassert(s, xml_decl + start_unit + "<element xmlns:foo=\"bar\"/>" + end_unit);
        free(s);
    }

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_memory(archive, &s, &size);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_write_start_unit(archive, unit);
        srcml_write_start_element(archive, 0, "element", 0);
        srcml_write_namespace(archive, 0, "bar");
        srcml_write_end_element(archive);
        srcml_write_end_unit(archive);
        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

        dassert(s, xml_decl + start_unit + "<element xmlns=\"bar\"/>" + end_unit);
        free(s);
    }

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_memory(archive, &s, &size);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_write_start_unit(archive, unit);
        srcml_write_start_element(archive, 0, "element", 0);
        dassert(srcml_write_namespace(archive, "foo", 0), SRCML_STATUS_INVALID_ARGUMENT);
        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);
    }

    {
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        dassert(srcml_write_namespace(archive, "foo", "bar"), SRCML_STATUS_INVALID_IO_OPERATION);
        srcml_close_archive(archive);
        srcml_free_archive(archive);
    }

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_memory(archive, &s, &size);
        dassert(srcml_write_namespace(archive, "foo", "bar"), SRCML_STATUS_INVALID_INPUT);
        srcml_close_archive(archive);
        srcml_free_archive(archive);
    }
    
    {
        dassert(srcml_write_namespace(0, "foo", "bar"), SRCML_STATUS_INVALID_ARGUMENT);
    }

    /*
        srcml_write_attribute
    */

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_memory(archive, &s, &size);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_write_start_unit(archive, unit);
        srcml_write_start_element(archive, 0, "element", 0);
        srcml_write_attribute(archive, 0, "foo", 0, "bar");
        srcml_write_end_element(archive);
        srcml_write_end_unit(archive);
        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

        dassert(s, xml_decl + start_unit + "<element foo=\"bar\"/>" + end_unit);
        free(s);
    }

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_memory(archive, &s, &size);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_write_start_unit(archive, unit);
        srcml_write_start_element(archive, 0, "element", 0);
        srcml_write_attribute(archive, "f", "foo", 0, "bar");
        srcml_write_end_element(archive);
        srcml_write_end_unit(archive);
        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

        dassert(s, xml_decl + start_unit + "<element f:foo=\"bar\"/>" + end_unit);
        free(s);
    }

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_memory(archive, &s, &size);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_write_start_unit(archive, unit);
        srcml_write_start_element(archive, 0, "element", 0);
        srcml_write_attribute(archive, 0, "foo", "b", "bar");
        srcml_write_end_element(archive);
        srcml_write_end_unit(archive);
        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

        dassert(s, xml_decl + start_unit + "<element foo=\"bar\" xmlns=\"b\"/>" + end_unit);
        free(s);
    }

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_memory(archive, &s, &size);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_write_start_unit(archive, unit);
        srcml_write_start_element(archive, 0, "element", 0);
        srcml_write_attribute(archive, "f", "foo", "b", "bar");
        srcml_write_end_element(archive);
        srcml_write_end_unit(archive);
        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

        dassert(s, xml_decl + start_unit + "<element f:foo=\"bar\" xmlns:f=\"b\"/>" + end_unit);
        free(s);
    }

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_memory(archive, &s, &size);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_write_start_unit(archive, unit);
        srcml_write_start_element(archive, 0, "element", 0);
        srcml_write_attribute(archive, 0, "foo", 0, "bar");
        srcml_write_end_element(archive);
        srcml_write_end_unit(archive);
        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

        dassert(s, xml_decl + start_unit + "<element foo=\"bar\"/>" + end_unit);
        free(s);
    }

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_memory(archive, &s, &size);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_write_start_unit(archive, unit);
        srcml_write_start_element(archive, 0, "element", 0);
        dassert(srcml_write_attribute(archive, "f", 0, "b", "bar"), SRCML_STATUS_INVALID_ARGUMENT);
        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);
    }

    {
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        dassert(srcml_write_attribute(archive, 0, "foo", 0, "bar"), SRCML_STATUS_INVALID_IO_OPERATION);
        srcml_close_archive(archive);
        srcml_free_archive(archive);
    }

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_memory(archive, &s, &size);
        dassert(srcml_write_attribute(archive, 0, "foo", 0, "bar"), SRCML_STATUS_INVALID_INPUT);
        srcml_close_archive(archive);
        srcml_free_archive(archive);
    }
    
    {
        dassert(srcml_write_attribute(0, "f", "foo", "b", "bar"), SRCML_STATUS_INVALID_ARGUMENT);
    }

    /*
        srcml_write_string
    */

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_memory(archive, &s, &size);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_write_start_unit(archive, unit);
        srcml_write_start_element(archive, 0, "element", 0);
        srcml_write_string(archive, "foo");
        srcml_write_end_element(archive);
        srcml_write_end_unit(archive);
        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

        dassert(s, xml_decl + start_unit + "<element>foo</element>" + end_unit);
        free(s);
    }

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_memory(archive, &s, &size);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_write_start_unit(archive, unit);
        srcml_write_start_element(archive, 0, "element", 0);
        srcml_write_string(archive, "foo");
        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

        dassert(s, xml_decl + start_unit + "<element>foo</element>" + end_unit);
        free(s);
    }

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_memory(archive, &s, &size);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_write_start_unit(archive, unit);
        srcml_write_start_element(archive, 0, "element", 0);
        srcml_write_start_element(archive, 0, "element", 0);
        srcml_write_string(archive, "foo");
        srcml_write_end_element(archive);
        srcml_write_end_element(archive);
        srcml_write_end_unit(archive);
        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

        dassert(s, xml_decl + start_unit + "<element><element>foo</element></element>" + end_unit);
        free(s);
    }

    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_memory(archive, &s, &size);
        dassert(srcml_write_string(archive, "foo"), SRCML_STATUS_INVALID_INPUT);
        srcml_close_archive(archive);
        srcml_free_archive(archive);
    }


    {
        char * s = 0;
        int size;
        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_memory(archive, &s, &size);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_write_start_unit(archive, unit);
        srcml_write_start_element(archive, 0, "element", 0);
        dassert(srcml_write_string(archive, 0), SRCML_STATUS_INVALID_ARGUMENT);
        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);
    }
    
    {
        dassert(srcml_write_string(0, "foo"), SRCML_STATUS_INVALID_ARGUMENT);
    }

    srcml_cleanup_globals();

    return 0;

}