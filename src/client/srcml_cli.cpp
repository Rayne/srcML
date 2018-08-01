/**
 * @file srcml_cli.cpp
 *
 * @copyright Copyright (C) 2014 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the srcml command-line client.
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
 * along with the srcml command-line client; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <srcml_cli.hpp>
#include <src_prefix.hpp>
#include <boost/program_options.hpp>
#include <stdlib.h>
#include <SRCMLStatus.hpp>

namespace prog_opts = boost::program_options;

const char* SRCML_HEADER = R"(Usage: srcml [options] <src_infile>... [-o <srcML_outfile>]
       srcml [options] <srcML_infile>... [-o <src_outfile>]

  Translates C, C++, C#, and Java source code to and from the XML
  source-code representation srcML. Also supports querying and transformation of srcML.
)";


const char* SRCML_FOOTER = R"(
  Have a question or need to report a bug?
  Contact us at http://www.srcml.org/support.html
  www.srcML.org)";

const char* SRC2SRCML_HEADER = R"(Usage: srcml [options] <src_infile>... [-o <srcML_outfile>]

  Translates C, C++, C#, and Java source code into the XML
  source-code representation srcML. Input can be from standard input, a file,
  a directory, or an archive file, i.e., tar, cpio, and zip. Multiple files
  are stored in a srcML archive.
  
  The source-code language is based on the file extension. Additional extensions
  for a language can be registered, and can be directly set using the --language
  option.
  
  By default, output is to stdout. You can specify a file for output using the
  --output or -o option. When no filenames are given, input is from stdin and
  output is to stdout. An input filename of '-' also reads from stdin.

  Any input file can be a local filename or a URI with the protocols http:,
  ftp:, or file:
)";

const char* SRC2SRCML_FOOTER = R"(Examples:

  Read from standard input, write to standard output:
  srcml
  
  Read from file m.cpp, write to standard output:
  srcml m.cpp
  
  Read from file m.cpp, write to file m.cpp.xml:
  srcml m.cpp -o m.cpp.xml
  
  Read from URI, write to standard output:
  srcml http://www.srcML.org/projects/srcml/ex/main.cpp
  
  Element unit attribute filename "m.cpp":
  srcml --filename=m.cpp m.cpp -o m.cpp.xml
  
  Set encoding of input text file to UTF-8:
  srcml --src-encoding=UTF-8 m.cpp m.cpp.xml
  
  Set encoding of srcML file to ISO-8859-1:
  srcml --xml-encoding=ISO-8859-1 m.cpp m.cpp.xml
)";

const char* SRCML2SRC_HEADER = R"(Usage: srcml [options] <srcML_infile>... [-o <src_outfile>]
  
  Translates from the the XML source-code representation srcML back to
  source-code.
  
  Extracts back to standard output or disk. Provides access to metadata about
  the srcML document. For srcML archives provides extraction of specific
  files, and efficient querying/transformation using XPath, XSLT, and RelaxNG.
  
  srcML archives contain multiple individual source code files, e.g., an
  entire project or directory tree.
  
  By default, output is to stdout.You can specify a file for output using the
  --output or -o option. When no filenames are given, input is from stdin and
  output is to stdout. An input filename of '-' also reads from stdin.
  
  Any input file, including XSLT and RelaxNG files, can be a local filename
  or a URI with the protocols http:, ftp:, or file:
  
  The srcML files can be in xml, or compressed with gzip or bzip2 (detected
  automatically).
)";

const char* SRCML2SRC_FOOTER = R"(Examples:
  
  Read from file m.cpp.xml, write to file m.cpp:
  srcml m.cpp.xml -o m.cpp
  
  Read from URI, write to file m.cpp:
  srcml http://www.example.org/m.cpp.xml m.cpp
  
  Read from file m.cpp.xml, output language attribute to stdout:
  srcml m.cpp.xml --show-language
)";

srcml_request_t srcml_request;

// Define Program Options
prog_opts::options_description general("GENERAL OPTIONS");
prog_opts::options_description src2srcml_options("CREATING SRCML");
prog_opts::options_description srcml2src_options("EXTRACTING SOURCE CODE");
prog_opts::options_description query_transform("TRANSFORMATIONS");
prog_opts::options_description positional_options("POSITIONAL");
prog_opts::options_description deprecated_options("DEPRECATED OPTIONS");
prog_opts::options_description debug_options("DEBUG OPTIONS");
prog_opts::options_description experimental_options("EXPERIMENTAL OPTIONS");
prog_opts::options_description implicit_value_handlers("IMPLICIT VALUE HANDLER");
prog_opts::options_description all("ALL OPTIONS");

prog_opts::options_description markup_options("MARKUP OPTIONS");
prog_opts::options_description xml_form("XML FORMAT");
prog_opts::options_description metadata_options("METADATA OPTIONS");

prog_opts::options_description src2srcml("");
prog_opts::options_description srcml2src("");


// Positional Args
prog_opts::positional_options_description input_file;

// tranformation check on input
bool is_transformation(const srcml_input_src& input);

/* Most of the no parameter options could be recorded this way */
template <int option>
void option_markup(bool opt) {
    /*
      If we have markup options the NULL optional arguement needs to
      first be initializied before the bitwise work can be done.
    */
    if (!srcml_request.markup_options)
        srcml_request.markup_options = 0;

    if (opt)
        *srcml_request.markup_options |= option;
}

template <int command>
void option_command(bool opt) {
    if (opt)
      srcml_request.command |= command;
}

// deprecated option command
// (This is required as non-deprecated options may use same values)
template <int command>
void option_command_deprecated(bool opt) {
    if (opt) {
      srcml_request.command |= command;

      // Notify user of deprecated options
      if (command == SRCML_COMMAND_UNITS)
        SRCMLstatus(INFO_MSG, "srcml: use of option --units or -n is deprecated");
      if (command == SRCML_COMMAND_EXPRESSION)
        SRCMLstatus(INFO_MSG, "srcml: use of option --expression or -e is deprecated");
    }
}

// Generic fields
template <boost::optional<std::string> srcml_request_t::*pfield>
void option_field(const std::string& value) { srcml_request.*pfield = value; }

template <std::vector<std::string> srcml_request_t::*pfield>
void option_field(const std::vector<std::string>& value) { srcml_request.*pfield = value; }

template <int srcml_request_t::*pfield>
void option_field(int value) { srcml_request.*pfield = value; }

template <boost::optional<size_t> srcml_request_t::*pfield>
void option_field(size_t value) { srcml_request.*pfield = value; }

// option files_from
template <>
void option_field<&srcml_request_t::files_from>(const std::vector<std::string>& value) {

    srcml_request.files_from = value;
    for (const auto& inputFile : value) {
        srcml_request.input_sources.push_back(src_prefix_add_uri("filelist", inputFile));
    }
}

// option src encoding
template <>
void option_field<&srcml_request_t::src_encoding>(const std::string& value) {
/*
    if (value.empty() || srcml_check_encoding(value.c_str()) == 0) {
        SRCMLstatus(ERROR_MSG, "srcml: invalid src encoding \"" + value + "\"");
        exit(CLI_ERROR_INVALID_ARGUMENT);
    }
    */
    srcml_request.src_encoding = value;
}

// option xml encoding attribute
template <>
void option_field<&srcml_request_t::att_xml_encoding>(const std::string& value) {

    if (value.empty() || srcml_check_encoding(value.c_str()) == 0) {
        SRCMLstatus(ERROR_MSG, "srcml: invalid xml encoding \"%s\"", value);
        exit(CLI_ERROR_INVALID_ARGUMENT);
    }
    srcml_request.att_xml_encoding = value;
}

// option language attribute
template <>
void option_field<&srcml_request_t::att_language>(const std::string& value) {

    // check language
    if (value.empty() || srcml_check_language(value.c_str()) == 0) {
        SRCMLstatus(ERROR_MSG, "srcml: invalid language \"%s\"", value);
        exit(6); //ERROR CODE TBD
    }
    srcml_request.att_language = value;
}

// option tabs
template <>
void option_field<&srcml_request_t::tabs>(int value) {

    // check tabstop
    if (value < 1) {
        SRCMLstatus(ERROR_MSG, "srcml: %d is an invalid tab stop. Tab stops must be 1 or higher.", value);
        exit(1); //ERROR CODE TBD
    }

    srcml_request.tabs = value;
    *srcml_request.markup_options |= SRCML_OPTION_POSITION;
}

void option_output_filename(const std::string& value) {
    srcml_request.output_filename = srcml_output_dest(value == "" ? "stdout://-" : value);

    if (srcml_request.output_filename.protocol == "file")  {
      if (srcml_request.output_filename.isdirectory || (srcml_request.output_filename.extension == ""
          && srcml_request.output_filename.filename[srcml_request.output_filename.filename.length() - 1] == '/')) {

        srcml_request.command |= SRCML_COMMAND_TO_DIRECTORY;
        srcml_request.command |= SRCML_COMMAND_NOARCHIVE;
      }
    }
}

void option_xmlns_uri(const std::string& value) {
    srcml_request.xmlns_namespaces[""] = value;
    srcml_request.xmlns_namespace_uris[value] = "";
}

void option_xmlns_prefix(const std::vector<std::string>& values) {
    for (const auto& value : values ) {

      std::size_t delim = value.find("=");
      if (delim == std::string::npos) {
        SRCMLstatus(ERROR_MSG, "srcml: xmlns format missing \"=\"");
        exit(1); //ERROR CODE TBD
      }

      srcml_request.xmlns_namespaces[value.substr(0, delim)] = value.substr(delim + 1);
      srcml_request.xmlns_namespace_uris[value.substr(delim + 1)] = value.substr(0, delim);
    }
}

// option output to directory
void option_to_dir(const std::string& value) {
    srcml_request.output_filename = srcml_output_dest(value);
    srcml_request.command |= SRCML_COMMAND_TO_DIRECTORY;
    srcml_request.command |= SRCML_COMMAND_NOARCHIVE;
}

void positional_args(const std::vector<std::string>& value) {
    srcml_request.input_sources.reserve(srcml_request.input_sources.size() + value.size());

    for (const auto& iname : value) {

        // record the position of stdin
        if (iname == "-" || iname == "stdin://-")
            srcml_request.stdindex = (int) srcml_request.input_sources.size();

        srcml_input_src input(iname);

        if (!(is_transformation(input))) {
          srcml_request.input_sources.push_back(input);
        }
    }
}

void raw_text_args(const std::vector<std::string>& value) {
    for (const auto& raw_text : value) {
        srcml_request.input_sources.push_back(src_prefix_add_uri("text", raw_text));
    }
}

void raw_null_text_arg(const std::vector<std::string>& value) {
    for (size_t i = 0; i < value.size(); ++i)
        srcml_request.input_sources.push_back(src_prefix_add_uri("text", ""));
}

void option_help(const std::string& help_opt) {

    int status = 0;

    if (help_opt.empty()) {
        // TODO: A new header and footer for the general option
        std::cout << SRCML_HEADER << "\n";
        std::cout << general;

        std::cout << src2srcml;
        std::cout << srcml2src;
    }
    else if (help_opt == "src2srcml") {
        std::cout << SRC2SRCML_HEADER << "\n";
        std::cout << src2srcml << "\n";
        std::cout << SRC2SRCML_FOOTER;
    }
    else if (help_opt == "srcml2src") {
        std::cout << SRCML2SRC_HEADER << "\n";
        std::cout << srcml2src << "\n";
        std::cout << SRCML2SRC_FOOTER;
    }
    else {
        std::cout << "Unknown module '"
                  << help_opt << "' in --help\n";
        status = 1;
    }

    std::cout << SRCML_FOOTER << "\n";

    exit(status);
}

/* Function used to check that 'opt1' and 'opt2' are not specified
   at the same time. (FROM BOOST LIBRARY EXAMPLES)*/
void conflicting_options(const prog_opts::variables_map& vm, const char* opt1, const char* opt2);

// Determine dependent options
void option_dependency(const prog_opts::variables_map& vm, const char* option, const char* dependent_option);

// Custom Parser Definition
std::pair<std::string, std::string> custom_parser(const std::string& s);

// Debug
void debug_cli_opts(const struct srcml_request_t srcml_request);

// Sanitize element input
element clean_element_input(const std::string& element_input);

// Sanitize attribute input
attribute clean_attribute_input(const std::string& attribute_input);

// Interpretation of CLI options
srcml_request_t parseCLI(int argc, char* argv[]) {
    try {
        general.add_options()
            ("help,h", prog_opts::value<std::string>()->notifier(&option_help)->value_name("MODULE"),"Display this help and exit. USAGE: help or help [module name]. MODULES: src2srcml, srcml2src")
            ("version,V", prog_opts::bool_switch()->notifier(&option_command<SRCML_COMMAND_VERSION>), "Display version number and exit")
            ("verbose,v", prog_opts::bool_switch()->notifier(&option_command<SRCML_COMMAND_VERBOSE>), "Conversion and status information to stderr")
            ("quiet,q", prog_opts::bool_switch()->notifier(&option_command<SRCML_COMMAND_QUIET>), "Suppress status messages")
            ("list", prog_opts::bool_switch()->notifier(&option_command<SRCML_COMMAND_LIST>), "List all files in the srcML archive and exit")
            ("info,i", prog_opts::bool_switch()->notifier(&option_command<SRCML_COMMAND_INFO>), "Display most metadata except srcML file count and exit")
            ("full-info,I", prog_opts::bool_switch()->notifier(&option_command<SRCML_COMMAND_LONGINFO>), "Display all metadata including srcML file count and exit")
            ("jobs,j", prog_opts::value<int>()->notifier(&option_field<&srcml_request_t::max_threads>)->value_name("NUM")->default_value(4), "Allow up to NUM threads for source parsing")
            ("output,o", prog_opts::value<std::string>()->notifier(&option_output_filename)->value_name("FILE")->default_value(""), "Write output to FILE")
            ;

        src2srcml_options.add_options()
            ("language,l", prog_opts::value<std::string>()->notifier(&option_field<&srcml_request_t::att_language>)->value_name("LANG"), "Set the source-code language to C, C++, C#, or Java")
            ("register-ext", prog_opts::value< std::vector<std::string> >()->notifier(&option_field<&srcml_request_t::language_ext>)->value_name("EXT=LANG"), "Register file extension EXT for source-code language LANG")
            ("src-encoding", prog_opts::value<std::string>()->notifier(&option_field<&srcml_request_t::src_encoding>)->value_name("ENCODING"), "Set the input source-code encoding")
            ("files-from", prog_opts::value<std::vector<std::string> >()->notifier(&option_field<&srcml_request_t::files_from>)->value_name("FILE"), "Read list of source-code files to form a srcML archive")
            ("output-xml,X", prog_opts::bool_switch()->notifier(&option_command<SRCML_COMMAND_XML>), "Output in XML instead of text")
            ("fragment", prog_opts::bool_switch()->notifier(&option_command<SRCML_COMMAND_XML_FRAGMENT>), "Output an XML fragment")
            ("raw", prog_opts::bool_switch()->notifier(&option_command<SRCML_COMMAND_XML_RAW>), "Output XML without root unit")
            ("archive,r", prog_opts::bool_switch()->notifier(&option_markup<SRCML_ARCHIVE>), "Store output in a srcML archive, default for multiple input files")
            ("text,t", prog_opts::value< std::vector<std::string> >()->notifier(&raw_text_args)->value_name("STRING"), "Raw string source-code input")
            ;

        markup_options.add_options()
            ("position", prog_opts::bool_switch()->notifier(&option_markup<SRCML_OPTION_POSITION>), "Include line/column attributes, namespace 'http://www.srcML.org/srcML/position'")
            ("tabs", prog_opts::value<int>()->notifier(&option_field<&srcml_request_t::tabs>)->value_name("NUM"), "Set tabs NUM characters apart.  Default is 8")
            ("cpp", prog_opts::bool_switch()->notifier(&option_markup<SRCML_OPTION_CPP>), "Enable preprocessor parsing and markup for Java and non-C/C++ languages")
            ("cpp-markup-if0", prog_opts::bool_switch()->notifier(&option_markup<SRCML_OPTION_CPP_MARKUP_IF0>), "Markup cpp #if 0 regions")
            ("cpp-nomarkup-else", prog_opts::bool_switch()->notifier(&option_markup<SRCML_OPTION_CPP_TEXT_ELSE>), "Leave cpp #else regions as text")
            ;

        xml_form.add_options()
            ("xml-encoding", prog_opts::value<std::string>()->notifier(&option_field<&srcml_request_t::att_xml_encoding>)->value_name("ENCODING")->default_value("UTF-8"),"Set output XML encoding. Default is UTF-8")
            ("no-xml-declaration", prog_opts::bool_switch()->notifier(&option_markup<SRCML_OPTION_XML_DECL>), "Do not output the XML declaration")
            ("xmlns", prog_opts::value<std::string>()->notifier(&option_xmlns_uri)->value_name("URI"), "Set the default namespace URI")
            ("xmlns:", prog_opts::value< std::vector<std::string> >()->notifier(&option_xmlns_prefix)->value_name("PREFIX=\"URI\""), "Set the namespace URI for PREFIX")
            ;

        metadata_options.add_options()
            ("show-language", prog_opts::bool_switch()->notifier(&option_command<SRCML_COMMAND_DISPLAY_SRCML_LANGUAGE>), "Display source language and exit")
            ("show-url", prog_opts::bool_switch()->notifier(&option_command<SRCML_COMMAND_DISPLAY_SRCML_URL>), "Display source url name and exit")
            ("show-filename", prog_opts::bool_switch()->notifier(&option_command<SRCML_COMMAND_DISPLAY_SRCML_FILENAME>), "Display source filename and exit")
            ("show-src-version", prog_opts::bool_switch()->notifier(&option_command<SRCML_COMMAND_DISPLAY_SRCML_SRC_VERSION>), "Display source version and exit")
            ("show-timestamp", prog_opts::bool_switch()->notifier(&option_command<SRCML_COMMAND_DISPLAY_SRCML_TIMESTAMP>), "Display timestamp and exit")
            ("show-hash", prog_opts::bool_switch()->notifier(&option_command<SRCML_COMMAND_DISPLAY_SRCML_HASH>), "Display hash and exit")
            ("show-encoding", prog_opts::bool_switch()->notifier(&option_command<SRCML_COMMAND_DISPLAY_SRCML_ENCODING>), "Display xml encoding and exit")
            ("show-unit-count", prog_opts::bool_switch()->notifier(&option_command<SRCML_COMMAND_UNITS>), "Display number of srcML files and exit")
            ("filename,f", prog_opts::value<std::string>()->notifier(&option_field<&srcml_request_t::att_filename>)->value_name("FILENAME"), "Set the filename attribute")
            ("url", prog_opts::value<std::string>()->notifier(&option_field<&srcml_request_t::att_url>)->value_name("URL"), "Set the url attribute")
            ("src-version,s", prog_opts::value<std::string>()->notifier(&option_field<&srcml_request_t::att_version>)->value_name("VERSION"), "Set the version attribute")
            ("hash", prog_opts::bool_switch()->notifier(&option_markup<SRCML_HASH>), "Add hash to srcml output")
            ("timestamp", prog_opts::bool_switch()->notifier(&option_command<SRCML_COMMAND_TIMESTAMP>), "Add timestamp to srcml output")
            ("prefix,p", prog_opts::value<std::string>()->notifier(&option_field<&srcml_request_t::xmlns_prefix_query>)->value_name("URI"), "Display prefix of namespace given by URI and exit")
           ;

        srcml2src_options.add_options()
            ("output-src,S", prog_opts::bool_switch()->notifier(&option_command<SRCML_COMMAND_SRC>), "Output in text instead of XML")
            ("to-dir", prog_opts::value<std::string>()->notifier(&option_to_dir)->value_name("DIRECTORY"), "Extract source-code files from srcML to a DIRECTORY in the filesystem")
            ;

        query_transform.add_options()
            ("unit,U", prog_opts::value<int>()->notifier(&option_field<&srcml_request_t::unit>)->value_name("NUM"), "Extract individual unit NUM from srcML")
            ("xpath", prog_opts::value< std::vector<std::string> >()->value_name("XPATH"), "Apply XPATH expression to each individual unit")
            ("attribute", prog_opts::value< std::vector<std::string> >()->value_name("PREFIX:URI=\"VALUE\""), "Add attribute PREFIX:URI=\"VALUE\" to element results of xpath query")
            ("element", prog_opts::value< std::vector<std::string> >()->value_name("PREFIX:URI"), "Wrap results of XPath query with element PREFIX:URI")
            ("xslt", prog_opts::value< std::vector<std::string> >()->value_name("URI"), "Apply the XSLT transformation at the given URI to each individual unit")
            ("xslt-param", prog_opts::value< std::vector<std::string> >()->value_name("NAME=\"VALUE\""), "Passes a string parameter NAME with VALUE to the XSLT program where VALUE is a UTF-8 encoding string")
            ("relaxng", prog_opts::value< std::vector<std::string> >()->value_name("URI"), "Output individual units that match the RelaxNG pattern at the given URI")
            ("revision", prog_opts::value<size_t>()->notifier(&option_field<&srcml_request_t::revision>), "Extract the given revision (0 = original, 1 = modified)")
            ;

        positional_options.add_options()
            ("input-files", prog_opts::value< std::vector<std::string> >()->notifier(&positional_args), "Input files")
            ;
        
        // Work arounds for dealing with implicit option properly
        implicit_value_handlers.add_options()
            ("text-equals-null", prog_opts::value< std::vector<std::string> >()->notifier(&raw_null_text_arg), "Work around for null text")
            ;

        deprecated_options.add_options()
            ("units,n", prog_opts::bool_switch()->notifier(&option_command_deprecated<SRCML_COMMAND_UNITS>), "Display number of srcML files and exit")
            ("expression,e", prog_opts::bool_switch()->notifier(&option_command_deprecated<SRCML_COMMAND_EXPRESSION>), "Expression mode for translating a single expression not in a statement")
            ;

        debug_options.add_options()
            ("dev", prog_opts::bool_switch()->notifier(&option_command<SRCML_DEBUG_MODE>), "Enable developer debug mode.")
            ("timing", prog_opts::bool_switch()->notifier(&option_command<SRCML_TIMING_MODE>), "Enable developer timing mode.")
            ;
            
        experimental_options.add_options()
            ("update", prog_opts::bool_switch()->notifier(&option_command<SRCML_COMMAND_UPDATE>), "Output and update existing srcml")
            ("interactive,c", prog_opts::bool_switch()->notifier(&option_command<SRCML_COMMAND_INTERACTIVE>), "Immediate output while parsing, default for keyboard input")
            ("xml-processing", prog_opts::value<std::string>()->notifier(&option_field<&srcml_request_t::xml_processing>), "Add XML processing instruction")
            ("pretty", prog_opts::value<std::string>()->implicit_value("")->notifier(&option_field<&srcml_request_t::pretty_format>), "Custom formatting for output")
            ("external", prog_opts::value<std::string>()->notifier(&option_field<&srcml_request_t::external>), "Run a user defined external script or application on srcml client output")
            ("line-ending", prog_opts::value<std::string>()->notifier(&option_field<&srcml_request_t::line_ending>), "Set the line endings for a desired environment \"Windows\" or \"Unix\"")
            ("unstable-order", prog_opts::bool_switch()->notifier(&option_command<SRCML_COMMAND_OUTPUT_UNSTABLE_ORDER>), "Enable non-strict output ordering")
            ;

        // Group src2srcml Options
        //src2srcml.add(src2srcml_options).add(markup_options).add(xml_form).add(metadata_options);
        src2srcml.add(src2srcml_options).add(markup_options).add(xml_form).add(metadata_options);

        // Group srcml2src Options
        //srcml2src.add(srcml2src_options).add(query_transform);
        srcml2src.add(srcml2src_options).add(query_transform);

        // Group all Options
        all.add(general).add(src2srcml_options).add(markup_options).add(xml_form).add(metadata_options).add(srcml2src_options).
            add(query_transform).add(positional_options).add(implicit_value_handlers).add(deprecated_options).add(debug_options).add(experimental_options);

        // Positional Args
        input_file.add("input-files", -1);

        // Assign the CLI args to the map
        prog_opts::variables_map cli_map;

        const auto& cliopts = prog_opts::command_line_parser(argc, argv).options(all).
                         positional(input_file).extra_parser(custom_parser).run();

        auto parsedOptions = cliopts.options;

        // loop the cli options in the order they were processed/received
        for (const auto& option : parsedOptions) {
            if (option.string_key == "relaxng" || option.string_key == "xpath" || option.string_key == "xslt" || option.string_key == "xslt-param"
             || option.string_key == "element" || option.string_key == "attribute") {

                if (option.string_key == "xpath")
                    srcml_request.xpath_query_support.push_back(std::make_pair(boost::none,boost::none));

                for (const auto& vals : option.value) {
                    if (option.string_key == "element" && srcml_request.xpath_query_support.size() < 1) {

                        SRCMLstatus(ERROR_MSG, "srcml: element option must follow an --xpath option");
                        exit(SRCML_STATUS_INVALID_ARGUMENT);
                    }
                    if (option.string_key == "attribute" && srcml_request.xpath_query_support.size() < 1) {

                        SRCMLstatus(ERROR_MSG, "srcml: attribute option must follow an --xpath option");
                        exit(SRCML_STATUS_INVALID_ARGUMENT);
                    }

                    if (option.string_key == "element") {
                        srcml_request.xpath_query_support[srcml_request.xpath_query_support.size() - 1].first = clean_element_input(vals);
                    }
                    else if (option.string_key == "attribute") {
                        srcml_request.xpath_query_support[srcml_request.xpath_query_support.size() - 1].second = clean_attribute_input(vals);
                    }
                    else {
                        srcml_request.transformations.push_back(src_prefix_add_uri(option.string_key, vals));
                    }
                }
            }
        }

        prog_opts::store(cliopts , cli_map);
        prog_opts::notify(cli_map);

        // Check option conflicts
        conflicting_options(cli_map, "quiet", "verbose");
        conflicting_options(cli_map, "output", "to-dir");
        conflicting_options(cli_map, "cpp-text-else", "cpp-markup-else");
        conflicting_options(cli_map, "cpp-text-if0", "cpp-markup-if0");
        conflicting_options(cli_map, "output-src", "output-xml");

        // Check dependent options
        // Format: option_dependency(cli_map, [option], [option]);
        option_dependency(cli_map, "text", "language");

        // If input was from stdin, then artificially put a "-" into the list of input files
        if (srcml_request.input_sources.empty())
            positional_args(std::vector<std::string>(1, "stdin://-"));

        if (srcml_request.input_sources.size() == 1 && srcml_request.input_sources[0].isdirectory) {
            auto url = srcml_request.input_sources[0].resource;
            while (url.length() > 0 && (url[0] == '.' || url[0] == '/')) {
                url.erase(0,1);
            }
            srcml_request.att_url = url;
        }

        // If position option is used without tabs...set default tab of 8
        if ((*srcml_request.markup_options & SRCML_OPTION_POSITION && srcml_request.tabs == 0) || srcml_request.tabs == 0)
            srcml_request.tabs = 8;

#if defined(__GNUG__) && !defined(__MINGW32__)
        // automatic interactive use from stdin (not on redirect or pipe)
        if (isatty(STDIN_FILENO))
            option_command<SRCML_COMMAND_INTERACTIVE>(true);
#endif

        // check namespaces
        for (size_t i = 0; i < srcml_get_namespace_size(); ++i) {
            std::string prefix = srcml_get_namespace_prefix(i);
            std::string uri = srcml_get_namespace_uri(i);

            // A reserved prefix wasn't used or it was set to the same uri
            if (srcml_request.xmlns_namespaces.find(prefix) == srcml_request.xmlns_namespaces.end() || srcml_request.xmlns_namespaces[prefix] == uri) {
                continue;
            }

            // A reserved uri is set to a different prefix
            if (srcml_request.xmlns_namespace_uris.find(uri) != srcml_request.xmlns_namespaces.end() && srcml_request.xmlns_namespace_uris[uri] != prefix) {
                continue;
            }

            SRCMLstatus(ERROR_MSG, "srcml: prefix \"" + prefix + "\" assigned multiple URIs \"" + uri + "\", \"" + srcml_request.xmlns_namespaces[prefix] + "\"");
            exit(1); // TODO Need a real error code
        }

        // Some options imply others SRCML_COMMAND_XML
        if (srcml_request.command & SRCML_COMMAND_XML_FRAGMENT)
            option_command<SRCML_COMMAND_XML>(true);

        if (srcml_request.command & SRCML_COMMAND_XML_RAW)
            option_command<SRCML_COMMAND_XML>(true);
    }
    // Unknown Option
    catch(boost::program_options::unknown_option& e) {
        SRCMLstatus(ERROR_MSG, "srcml: %s", e.what());
        exit(3);
    }
    // Missing Option Value
    catch(boost::program_options::error_with_option_name& e) {
        std::string error_msg(e.what());
        
        /* This allows for --help with no value (currently a work around for implicit issues)
            We check the error message for a section to identify when --help is used without a value
            and call the function manually to print the appropriate help message. 
            Calls to option_help automatically exit the cli. */
        if (error_msg.find("'--help' is missing") != std::string::npos) {
                option_help("");
        }

        SRCMLstatus(ERROR_MSG, "srcml: %s", error_msg);
        exit(7);
    }
    // Catch all other issues with generic error
    catch(std::exception& e) {
        SRCMLstatus(ERROR_MSG, "srcml: %s", e.what());
        exit(1);
    }

    return srcml_request;
}

// Custom parser for xmlns: option
std::pair<std::string, std::string> custom_parser(const std::string& s) {
    if (s.find("--xmlns:") == 0)
        return std::make_pair(std::string("xmlns:"), s.substr(s.find(":")+1));
    
    // Divert --text="" to a hidden option that allows empty args (implicit work around)
    if (s.find("--text=") == 0) {
        auto val = s.substr(s.find("=") + 1);
        if (val == "")
            /* We have already determined that we have an empty string, but we need to pass a non-null value.
                The value doesn't matter as it is not used it just allows the program options to record the correct
                number of empty strings provided by one or more --text="" uses. */
            return std::make_pair(std::string("text-equals-null"), " ");
        else
            return std::make_pair(std::string("text"), val);
    }

    return std::make_pair(std::string(), std::string());
}

// Set to detect option conflicts
void conflicting_options(const prog_opts::variables_map& vm, const char* opt1, const char* opt2) {
    if (vm.count(opt1) && !vm[opt1].defaulted() && vm.count(opt2) && !vm[opt2].defaulted()) {
        SRCMLstatus(ERROR_MSG, "srcml: Conflicting options '%s' and '%s'.", opt1, opt2);
        exit(15);
    }
}

// Check for dependent options
void option_dependency(const prog_opts::variables_map& vm,
                        const char* option, const char* dependent_option) {

    if (vm.count(option) && !vm[option].defaulted()) {
        if (vm.count(dependent_option) == 0) {
            throw std::logic_error(std::string("Option '") + option
                                    + "' requires option '" + dependent_option + "'.");
        }
    }
}

bool is_transformation(const srcml_input_src& input) {
    std::string ext = input.extension;

    if (ext == ".rng") {
        srcml_request.transformations.push_back(src_prefix_add_uri("relaxng", input.filename));
        return true;
    }

    if (ext == ".xsl") {
        srcml_request.transformations.push_back(src_prefix_add_uri("xslt", input.filename));
        return true;
    }

    return false;
}

element clean_element_input(const std::string& element_input) {
    std::string vals = element_input;
    size_t elemn_index = vals.find(":");

    // Element requires a prefix
    if (elemn_index == std::string::npos) {
        exit(1);
    }

    element elem;
    elem.prefix = vals.substr(0, elemn_index);
    elem.name = vals.substr(elemn_index + 1);
    return elem;
}

attribute clean_attribute_input(const std::string& attribute_input) {
    std::string vals = attribute_input;
    size_t attrib_colon = vals.find(":");
    size_t attrib_equals = vals.find("=");

    // Attribute must have a value
    if (attrib_equals == std::string::npos) {
        SRCMLstatus(ERROR_MSG, "srcml: the attribute %s is missing a value", vals);
        exit(SRCML_STATUS_INVALID_ARGUMENT);
    }

    // Missing prefix requires an element with a prefix
    if (attrib_colon == std::string::npos && !(srcml_request.xpath_query_support[srcml_request.xpath_query_support.size() - 1].first)) {
        SRCMLstatus(ERROR_MSG, "srcml: the attribute %s is missing a prefix or an element with a prefix", vals);
        exit(SRCML_STATUS_INVALID_ARGUMENT);
    }

    attribute attrib;

    if (attrib_colon != std::string::npos) {
        attrib.prefix = vals.substr(0, attrib_colon);
        attrib.name = vals.substr(attrib_colon + 1, attrib_equals - attrib_colon - 1);
    } else {
        attrib.prefix = srcml_request.xpath_query_support[srcml_request.xpath_query_support.size() - 1].first->prefix;
        attrib.name = vals.substr(0, attrib_equals);
    }

    size_t attrib_value_start = attrib_equals + 1;

    // value may be wrapped with quotes that need to be removed
    if (vals[attrib_value_start] == '\'' || vals[attrib_value_start] == '"')
        ++attrib_value_start;

    size_t attrib_value_size = vals.size() - attrib_value_start;

    if (vals[attrib_value_start + attrib_value_size - 1] == '\'' || vals[attrib_value_start + attrib_value_size - 1] == '"')
        --attrib_value_size;

    attrib.value = vals.substr(attrib_value_start, attrib_value_size);

    return attrib;
}
