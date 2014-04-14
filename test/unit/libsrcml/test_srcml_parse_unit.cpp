/**
 * @file test_srcml_parse_unit.cpp
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

  Test cases for srcml_parse_unit
*/

#include <srcml_macros.hpp>

#include <stdio.h>
#include <string.h>
#include <cassert>
#include <fstream>

#if defined(__GNUC__) && !defined(__MINGW32__)
#include <unistd.h>
#else
#include <io.h>
#endif
#include <fcntl.h>

#include <srcml.h>
#include <srcml_types.hpp>
#include <srcmlns.hpp>

#include "dassert.hpp"

int main() {

    const std::string src = "a;\n";
    const std::string utf8_src = "/* \u2713 */\n";
    const std::string latin_src = "/* \xfe\xff */\n";
    const std::string src_macro = "MACRO1;\nMACRO2;\n";
    const std::string srcml = "<unit xmlns:cpp=\"http://www.sdml.info/srcML/cpp\" language=\"C\"><expr_stmt><expr><name>a</name></expr>;</expr_stmt>\n</unit>";
    const std::string srcml_full = "<unit xmlns:cpp=\"http://www.sdml.info/srcML/cpp\" language=\"C++\" dir=\"test\" filename=\"project\" version=\"1\"><expr_stmt><expr><name>a</name></expr>;</expr_stmt>\n</unit>";
    const std::string utf8_srcml = "<unit xmlns=\"http://www.sdml.info/srcML/src\" xmlns:cpp=\"http://www.sdml.info/srcML/cpp\" language=\"C++\" dir=\"test\" filename=\"project\" version=\"1\"><comment type=\"block\">/* \u2713 */</comment>\n</unit>";
    const std::string latin_srcml = "<unit xmlns=\"http://www.sdml.info/srcML/src\" xmlns:cpp=\"http://www.sdml.info/srcML/cpp\" language=\"C++\" dir=\"test\" filename=\"project\" version=\"1\"><comment type=\"block\">/* \u00fe\u00ff */</comment>\n</unit>";
    const std::string srcml_macro = "<unit xmlns=\"http://www.sdml.info/srcML/src\" xmlns:cpp=\"http://www.sdml.info/srcML/cpp\" language=\"C++\" dir=\"test\" filename=\"project\" version=\"1\"><macro-list token=\"MACRO1\" type=\"src:macro\"/><macro-list token=\"MACRO2\" type=\"src:macro\"/><macro><name>MACRO1</name></macro><empty_stmt>;</empty_stmt>\n<macro><name>MACRO2</name></macro><empty_stmt>;</empty_stmt>\n</unit>";
    const std::string srcml_timestamp = "<unit xmlns:cpp=\"http://www.sdml.info/srcML/cpp\" language=\"C\" timestamp=\"today\"><expr_stmt><expr><name>a</name></expr>;</expr_stmt>\n</unit>";
    const std::string srcml_hash = "<unit xmlns:cpp=\"http://www.sdml.info/srcML/cpp\" language=\"C\" hash=\"0123456789abcdef\"><expr_stmt><expr><name>a</name></expr>;</expr_stmt>\n</unit>";
    const std::string srcml_hash_generated = "<unit xmlns:cpp=\"http://www.sdml.info/srcML/cpp\" language=\"C\" hash=\"\"><expr_stmt><expr><name>a</name></expr>;</expr_stmt>\n</unit>";

    std::ofstream src_file_c("project.c");
    src_file_c << src;
    src_file_c.close();

    std::ofstream src_file_foo("project.foo");
    src_file_foo << src;
    src_file_foo.close();

    std::ofstream src_file_utf8("project_utf8.cpp");
    src_file_utf8 << utf8_src;
    src_file_utf8.close();

    std::ofstream src_file_latin("project_latin.cpp");
    src_file_latin << latin_src;
    src_file_latin.close();

    std::ofstream src_file_macro("project_macro.cpp");
    src_file_macro << src_macro;
    src_file_macro.close();

    /*
      srcml_parse_unit_filename
    */
    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_parse_unit_filename(unit, "project.c");
        dassert(*unit->unit, srcml);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C");
        srcml_parse_unit_filename(unit, "project.foo");
        dassert(*unit->unit, srcml);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_set_language(archive, "C");
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_parse_unit_filename(unit, "project.foo");
        dassert(*unit->unit, srcml);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_archive_register_file_extension(archive, "foo", "C++");
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_filename(unit, "project");
        srcml_unit_set_directory(unit, "test");
        srcml_unit_set_version(unit , "1");
        srcml_parse_unit_filename(unit, "project.foo");
        dassert(*unit->unit, srcml_full);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_set_language(archive, "C++");
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C");
        srcml_parse_unit_filename(unit, "project.foo");
        dassert(*unit->unit, srcml);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "project");
        srcml_unit_set_directory(unit, "test");
        srcml_unit_set_version(unit , "1");
        srcml_parse_unit_filename(unit, "project.c");
        dassert(*unit->unit, srcml_full);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_encoding(unit, "UTF-8");
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "project");
        srcml_unit_set_directory(unit, "test");
        srcml_unit_set_version(unit , "1");
        srcml_parse_unit_filename(unit, "project_utf8.cpp");
        dassert(*unit->unit, utf8_srcml);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_archive_set_encoding(archive, "ISO-8859-1");
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_encoding(unit, "UTF-8");
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "project");
        srcml_unit_set_directory(unit, "test");
        srcml_unit_set_version(unit , "1");
        srcml_parse_unit_filename(unit, "project_utf8.cpp");
        dassert(*unit->unit, utf8_srcml);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_encoding(unit, "ISO-8859-1");
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "project");
        srcml_unit_set_directory(unit, "test");
        srcml_unit_set_version(unit , "1");
        srcml_parse_unit_filename(unit, "project_latin.cpp");
        dassert(*unit->unit, latin_srcml);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_archive_set_encoding(archive, "ISO-8859-1");
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_encoding(unit, "ISO-8859-1");
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "project");
        srcml_unit_set_directory(unit, "test");
        srcml_unit_set_version(unit , "1");
        srcml_parse_unit_filename(unit, "project_latin.cpp");
        dassert(*unit->unit, latin_srcml);
        srcml_write_unit(archive, unit);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_archive_register_macro(archive, "MACRO1", "src:macro");
        srcml_archive_register_macro(archive, "MACRO2", "src:macro");
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "project");
        srcml_unit_set_directory(unit, "test");
        srcml_unit_set_version(unit , "1");
        srcml_parse_unit_filename(unit, "project_macro.cpp");
        dassert(*unit->unit, srcml_macro);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_parse_unit_filename(unit, "project.c");
        assert(unit->unit->find("timestamp") == std::string::npos);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_timestamp(unit, "today");
        srcml_parse_unit_filename(unit, "project.c");
        dassert(*unit->unit, srcml_timestamp);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_parse_unit_filename(unit, "project.c");
        dassert(*unit->unit, srcml_hash_generated);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_hash(unit, "0123456789abcdef");
        srcml_parse_unit_filename(unit, "project.c");
        dassert(*unit->unit, srcml_hash);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        dassert(srcml_parse_unit_filename(unit, "project.cpp"), SRCML_STATUS_IO_ERROR);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_unit * unit = srcml_create_unit(archive);
        dassert(srcml_parse_unit_filename(unit, "project.c"), SRCML_STATUS_INVALID_IO_OPERATION);

        srcml_free_unit(unit);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        dassert(srcml_parse_unit_filename(unit, 0), SRCML_STATUS_INVALID_ARGUMENT);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        dassert(srcml_parse_unit_filename(0, "project.c"), SRCML_STATUS_INVALID_ARGUMENT);

        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        dassert(srcml_parse_unit_filename(unit, "project.foo"), SRCML_STATUS_UNSET_LANGUAGE);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    /*
      srcml_parse_unit_memory
    */

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C");
        srcml_parse_unit_memory(unit, src.c_str(), src.size());
        dassert(*unit->unit, srcml);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_set_language(archive, "C");
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_parse_unit_memory(unit, src.c_str(), src.size());
        dassert(*unit->unit, srcml);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_set_language(archive, "C++");
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C");
        srcml_parse_unit_memory(unit, src.c_str(), src.size());
        dassert(*unit->unit, srcml);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "project");
        srcml_unit_set_directory(unit, "test");
        srcml_unit_set_version(unit , "1");
        srcml_parse_unit_memory(unit, src.c_str(), src.size());
        dassert(*unit->unit, srcml_full);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "project");
        srcml_unit_set_directory(unit, "test");
        srcml_unit_set_version(unit , "1");
        srcml_parse_unit_memory(unit, src.c_str(), src.size());
        dassert(*unit->unit, srcml_full);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_encoding(unit, "UTF-8");
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "project");
        srcml_unit_set_directory(unit, "test");
        srcml_unit_set_version(unit , "1");
        srcml_parse_unit_memory(unit, utf8_src.c_str(), utf8_src.size());
        dassert(*unit->unit, utf8_srcml);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_archive_set_encoding(archive, "ISO-8859-1");
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_encoding(unit, "UTF-8");
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "project");
        srcml_unit_set_directory(unit, "test");
        srcml_unit_set_version(unit , "1");
        srcml_parse_unit_memory(unit, utf8_src.c_str(), utf8_src.size());
        dassert(*unit->unit, utf8_srcml);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_encoding(unit, "ISO-8859-1");
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "project");
        srcml_unit_set_directory(unit, "test");
        srcml_unit_set_version(unit , "1");
        srcml_parse_unit_memory(unit, latin_src.c_str(), latin_src.size());
        dassert(*unit->unit, latin_srcml);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_archive_set_encoding(archive, "ISO-8859-1");
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_encoding(unit, "ISO-8859-1");
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "project");
        srcml_unit_set_directory(unit, "test");
        srcml_unit_set_version(unit , "1");
        srcml_parse_unit_memory(unit, latin_src.c_str(), latin_src.size());
        dassert(*unit->unit, latin_srcml);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_archive_register_macro(archive, "MACRO1", "src:macro");
        srcml_archive_register_macro(archive, "MACRO2", "src:macro");
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "project");
        srcml_unit_set_directory(unit, "test");
        srcml_unit_set_version(unit , "1");
        srcml_parse_unit_memory(unit, src_macro.c_str(), src_macro.size());
        dassert(*unit->unit, srcml_macro);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C");
        srcml_parse_unit_memory(unit, src.c_str(), src.size());
        assert(unit->unit->find("timestamp" != 0));

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_timestamp(unit, "today");
        srcml_unit_set_language(unit, "C");
        srcml_parse_unit_memory(unit, src.c_str(), src.size());
        dassert(*unit->unit, srcml_timestamp);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_hash(unit, "0123456789abcdef");
        srcml_unit_set_language(unit, "C");
        srcml_parse_unit_memory(unit, src.c_str(), src.size());
        dassert(*unit->unit, srcml_hash);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C");
        srcml_parse_unit_memory(unit, src.c_str(), src.size());
        dassert(*unit->unit, srcml_hash_generated);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C");
        dassert(srcml_parse_unit_memory(unit, src.c_str(), src.size()), SRCML_STATUS_INVALID_IO_OPERATION);

        srcml_free_unit(unit);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C");
        dassert(srcml_parse_unit_memory(unit, 0, src.size()), SRCML_STATUS_INVALID_ARGUMENT);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C");
        dassert(srcml_parse_unit_memory(unit, src.c_str(), 0), SRCML_STATUS_OK);
        dassert(*unit->unit, "<unit xmlns:cpp=\"http://www.sdml.info/srcML/cpp\" language=\"C\"/>");

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C");
        dassert(srcml_parse_unit_memory(unit, 0, 0), SRCML_STATUS_OK);
        dassert(*unit->unit, "<unit xmlns:cpp=\"http://www.sdml.info/srcML/cpp\" language=\"C\"/>");

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        dassert(srcml_parse_unit_memory(0, src.c_str(), src.size()), SRCML_STATUS_INVALID_ARGUMENT);

        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        dassert(srcml_parse_unit_memory(unit, src.c_str(), src.size()), SRCML_STATUS_UNSET_LANGUAGE);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    /*
      srcml_parse_unit_FILE
    */

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C");
        FILE * file = fopen("project.c", "r");
        srcml_parse_unit_FILE(unit, file);
        dassert(*unit->unit, srcml);
        fclose(file);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_set_language(archive, "C");
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        FILE * file = fopen("project.c", "r");
        srcml_parse_unit_FILE(unit, file);
        dassert(*unit->unit, srcml);
        fclose(file);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_set_language(archive, "C++");
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C");
        FILE * file = fopen("project.c", "r");
        srcml_parse_unit_FILE(unit, file);
        dassert(*unit->unit, srcml);
        fclose(file);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "project");
        srcml_unit_set_directory(unit, "test");
        srcml_unit_set_version(unit , "1");
        FILE * file = fopen("project.c", "r");
        srcml_parse_unit_FILE(unit, file);
        dassert(*unit->unit, srcml_full);
        fclose(file);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "project");
        srcml_unit_set_directory(unit, "test");
        srcml_unit_set_version(unit , "1");
        FILE * file = fopen("project.c", "r");
        srcml_parse_unit_FILE(unit, file);
        dassert(*unit->unit, srcml_full);
        fclose(file);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_encoding(unit, "UTF-8");
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "project");
        srcml_unit_set_directory(unit, "test");
        srcml_unit_set_version(unit , "1");
        FILE * file = fopen("project_utf8.cpp", "r");
        srcml_parse_unit_FILE(unit, file);
        dassert(*unit->unit, utf8_srcml);
        fclose(file);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_archive_set_encoding(archive, "ISO-8859-1");
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_encoding(unit, "UTF-8");
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "project");
        srcml_unit_set_directory(unit, "test");
        srcml_unit_set_version(unit , "1");
        FILE * file = fopen("project_utf8.cpp", "r");
        srcml_parse_unit_FILE(unit, file);
        dassert(*unit->unit, utf8_srcml);
        fclose(file);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_encoding(unit, "ISO-8859-1");
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "project");
        srcml_unit_set_directory(unit, "test");
        srcml_unit_set_version(unit , "1");
        FILE * file = fopen("project_latin.cpp", "r");
        srcml_parse_unit_FILE(unit, file);
        dassert(*unit->unit, latin_srcml);
        fclose(file);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_archive_set_encoding(archive, "ISO-8859-1");
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_encoding(unit, "ISO-8859-1");
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "project");
        srcml_unit_set_directory(unit, "test");
        srcml_unit_set_version(unit , "1");
        FILE * file = fopen("project_latin.cpp", "r");
        srcml_parse_unit_FILE(unit, file);
        dassert(*unit->unit, latin_srcml);
        fclose(file);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_archive_register_macro(archive, "MACRO1", "src:macro");
        srcml_archive_register_macro(archive, "MACRO2", "src:macro");
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "project");
        srcml_unit_set_directory(unit, "test");
        srcml_unit_set_version(unit , "1");
        FILE * file = fopen("project_macro.cpp", "r");
        srcml_parse_unit_FILE(unit, file);
        dassert(*unit->unit, srcml_macro);
        fclose(file);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        FILE * file = fopen("project.c", "r");
        srcml_unit_set_language(unit, "C");
        srcml_parse_unit_FILE(unit, file);
        assert(unit->unit->find("timestamp" != 0));
        fclose(file);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_timestamp(unit, "today");
        srcml_unit_set_language(unit, "C");
        FILE * file = fopen("project.c", "r");
        srcml_parse_unit_FILE(unit, file);
        dassert(*unit->unit, srcml_timestamp);
        fclose(file);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C");
        FILE * file = fopen("project.c", "r");
        srcml_parse_unit_FILE(unit, file);
        dassert(*unit->unit, srcml_hash_generated);
        fclose(file);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_hash(unit, "0123456789abcdef");
        srcml_unit_set_language(unit, "C");
        FILE * file = fopen("project.c", "r");
        srcml_parse_unit_FILE(unit, file);
        dassert(*unit->unit, srcml_hash);
        fclose(file);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C");
        FILE * file = fopen("project.c", "r");
        srcml_parse_unit_FILE(unit, file);
        dassert(*unit->unit, srcml_hash_generated);
        fclose(file);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C");
        FILE * file = fopen("project.c", "r");
        dassert(srcml_parse_unit_FILE(unit, file), SRCML_STATUS_INVALID_IO_OPERATION);
        fclose(file);

        srcml_free_unit(unit);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C");
        dassert(srcml_parse_unit_FILE(unit, 0), SRCML_STATUS_INVALID_ARGUMENT);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        FILE * file = fopen("project.c", "r");
        dassert(srcml_parse_unit_FILE(0, file), SRCML_STATUS_INVALID_ARGUMENT);
        fclose(file);

        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        FILE * file = fopen("project.c", "r");
        dassert(srcml_parse_unit_FILE(unit, file), SRCML_STATUS_UNSET_LANGUAGE);
        fclose(file);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    /*
      srcml_parse_unit_fd
    */

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C");
        int fd = OPEN("project.c", O_RDONLY, 0);
        srcml_parse_unit_fd(unit, fd);
        dassert(*unit->unit, srcml);
        CLOSE(fd);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_set_language(archive, "C");
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        int fd = OPEN("project.c", O_RDONLY, 0);
        srcml_parse_unit_fd(unit, fd);
        dassert(*unit->unit, srcml);
        CLOSE(fd);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_set_language(archive, "C++");
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C");
        int fd = OPEN("project.c", O_RDONLY, 0);
        srcml_parse_unit_fd(unit, fd);
        dassert(*unit->unit, srcml);
        CLOSE(fd);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }


    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "project");
        srcml_unit_set_directory(unit, "test");
        srcml_unit_set_version(unit , "1");
        int fd = OPEN("project.c", O_RDONLY, 0);
        srcml_parse_unit_fd(unit, fd);
        dassert(*unit->unit, srcml_full);
        CLOSE(fd);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "project");
        srcml_unit_set_directory(unit, "test");
        srcml_unit_set_version(unit , "1");
        int fd = OPEN("project.c", O_RDONLY, 0);
        srcml_parse_unit_fd(unit, fd);
        dassert(*unit->unit, srcml_full);
        CLOSE(fd);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_encoding(unit, "UTF-8");
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "project");
        srcml_unit_set_directory(unit, "test");
        srcml_unit_set_version(unit , "1");
        int fd = OPEN("project_utf8.cpp", O_RDONLY, 0);
        srcml_parse_unit_fd(unit, fd);
        dassert(*unit->unit, utf8_srcml);
        CLOSE(fd);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_archive_set_encoding(archive, "ISO-8859-1");
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_encoding(unit, "UTF-8");
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "project");
        srcml_unit_set_directory(unit, "test");
        srcml_unit_set_version(unit , "1");
        int fd = OPEN("project_utf8.cpp", O_RDONLY, 0);
        srcml_parse_unit_fd(unit, fd);
        dassert(*unit->unit, utf8_srcml);
        CLOSE(fd);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_encoding(unit, "ISO-8859-1");
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "project");
        srcml_unit_set_directory(unit, "test");
        srcml_unit_set_version(unit , "1");
        int fd = OPEN("project_latin.cpp", O_RDONLY, 0);
        srcml_parse_unit_fd(unit, fd);
        dassert(*unit->unit, latin_srcml);
        CLOSE(fd);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_archive_set_encoding(archive, "ISO-8859-1");
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_encoding(unit, "ISO-8859-1");
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "project");
        srcml_unit_set_directory(unit, "test");
        srcml_unit_set_version(unit , "1");
        int fd = OPEN("project_latin.cpp", O_RDONLY, 0);
        srcml_parse_unit_fd(unit, fd);
        dassert(*unit->unit, latin_srcml);
        CLOSE(fd);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);
        srcml_archive_register_macro(archive, "MACRO1", "src:macro");
        srcml_archive_register_macro(archive, "MACRO2", "src:macro");
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C++");
        srcml_unit_set_filename(unit, "project");
        srcml_unit_set_directory(unit, "test");
        srcml_unit_set_version(unit , "1");
        int fd = OPEN("project_macro.cpp", O_RDONLY, 0);
        srcml_parse_unit_fd(unit, fd);
        dassert(*unit->unit, srcml_macro);
        CLOSE(fd);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        int fd = OPEN("project.c", O_RDONLY, 0);
        srcml_unit_set_language(unit, "C");
        srcml_parse_unit_fd(unit, fd);
        assert(unit->unit->find("timestamp" != 0));
        CLOSE(fd);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_timestamp(unit, "today");
        srcml_unit_set_language(unit, "C");
        int fd = OPEN("project.c", O_RDONLY, 0);
        srcml_parse_unit_fd(unit, fd);
        dassert(*unit->unit, srcml_timestamp);
        CLOSE(fd);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_hash(unit, "0123456789abcdef");
        srcml_unit_set_language(unit, "C");
        int fd = OPEN("project.c", O_RDONLY, 0);
        srcml_parse_unit_fd(unit, fd);
        dassert(*unit->unit, srcml_hash);
        CLOSE(fd);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C");
        int fd = OPEN("project.c", O_RDONLY, 0);
        dassert(srcml_parse_unit_fd(unit, fd), SRCML_STATUS_INVALID_IO_OPERATION);
        CLOSE(fd);

        srcml_free_unit(unit);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        srcml_unit_set_language(unit, "C");
        dassert(srcml_parse_unit_fd(unit, -1), SRCML_STATUS_INVALID_ARGUMENT);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        int fd = OPEN("project.c", O_RDONLY, 0);
        dassert(srcml_parse_unit_fd(0, fd), SRCML_STATUS_INVALID_ARGUMENT);
        CLOSE(fd);

        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    {

        srcml_archive * archive = srcml_create_archive();
        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
        srcml_write_open_filename(archive, "project.xml");
        srcml_unit * unit = srcml_create_unit(archive);
        int fd = OPEN("project.c", O_RDONLY, 0);
        dassert(srcml_parse_unit_fd(unit, fd), SRCML_STATUS_UNSET_LANGUAGE);
        CLOSE(fd);

        srcml_free_unit(unit);
        srcml_close_archive(archive);
        srcml_free_archive(archive);

    }

    UNLINK("project.c");
    UNLINK("project.foo");
    UNLINK("project_utf8.foo");
    UNLINK("project_latin.foo");
    UNLINK("project.xml");

    srcml_cleanup_globals();

    return 0;
}