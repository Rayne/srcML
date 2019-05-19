/**
 * @file create_srcml.hpp
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

#include <functional>
#include <create_srcml.hpp>
#include <srcml.h>
#include <srcml_options.hpp>
#include <ParseQueue.hpp>
#include <WriteQueue.hpp>
#include <src_input_libarchive.hpp>
#include <src_input_file.hpp>
#include <src_input_filesystem.hpp>
#include <src_input_filelist.hpp>
#include <src_input_text.hpp>
#include <src_input_s3.hpp>
#include <src_prefix.hpp>
#include <srcml_input_srcml.hpp>
#include <transform_srcml.hpp>
#include <TraceLog.hpp>
#include <input_file.hpp>
#include <curl_input_file.hpp>
#include <input_curl.hpp>
#include <iostream>
#include <input_archive.hpp>
#include <SRCMLStatus.hpp>
#include <ParserTest.hpp>
#include <cstring>

int srcml_handler_dispatch(ParseQueue& queue,
                          srcml_archive* srcml_arch,
                          const srcml_request_t& srcml_request,
                          const srcml_input_src& input,
                          const srcml_output_dest& destination) {

    // call appropriate handler
    if (input.state == SRCML) {

        srcml_input_src uninput = input;

        // input must go through libcurl pipe
        if (curl_supported(uninput.protocol) && uninput.protocol != "file" && !input_curl(uninput)) {
            SRCMLstatus(ERROR_MSG, "srcml: Unable to open srcml URL %s", uninput.filename);
            return -1;
        }

        // may have some compressions/archives
        // @todo What if there is multiple files in the archive?
        if (!uninput.compressions.empty() || !uninput.archives.empty())
            uninput.fd = input_archive(uninput);

        return srcml_input_srcml(queue, srcml_arch, srcml_request, uninput, srcml_request.revision);
    }

    if (input.protocol == "text") {

        return src_input_text(queue, srcml_arch, srcml_request, input);
    }

    if (input.protocol == "filelist") {

        srcml_archive_enable_full_archive(srcml_arch);

        return src_input_filelist(queue, srcml_arch, srcml_request, input, destination);
    }

    if (input.protocol == "file" && input.isdirectory) {

        return src_input_filesystem(queue, srcml_arch, srcml_request, input);
    }

    if (input.protocol == "file" && input.archives.empty() && input.compressions.empty()) {

        return src_input_file(queue, srcml_arch, srcml_request, input);
    }



    #ifdef LINKING_WITH_AWS_SDK
    if (input.protocol == "s3") {
        
        return src_input_s3(queue, srcml_arch, srcml_request, input);
    }
    #endif

    srcml_input_src uninput = input;

    // input must go through libcurl pipe
    if (curl_supported(uninput.protocol) && uninput.protocol != "file" && !input_curl(uninput)){
        SRCMLstatus(ERROR_MSG, "srcml: Unable to open srcml URL %s", uninput.filename);
        return -1;
    }

    return src_input_libarchive(queue, srcml_arch, srcml_request, uninput);
}

// create srcml from the current request
void create_srcml(const srcml_request_t& srcml_request,
                  const srcml_input_t& input_sources,
                  const srcml_output_dest& destination) {

    // create the output srcml archive
    srcml_archive* srcml_arch = srcml_archive_create();
    if (!srcml_arch) {
        SRCMLstatus(ERROR_MSG, "srcml: allocation error for srcml archive");
        exit(SRCML_STATUS_INVALID_ARGUMENT);
    }

    // open the output
    int nstatus = SRCML_STATUS_OK;
    if (!option(SRCML_COMMAND_NOARCHIVE)) {
        if (contains<int>(destination)) {

            nstatus = srcml_archive_write_open_fd(srcml_arch, *destination.fd);

        } else {

            nstatus = srcml_archive_write_open_filename(srcml_arch, destination.c_str());
        }
        if (nstatus != SRCML_STATUS_OK)
            return;
    }

    // set options for the output srcml archive

    // xml encoding
    if (srcml_request.att_xml_encoding && srcml_archive_set_xml_encoding(srcml_arch, srcml_request.att_xml_encoding->c_str()) != SRCML_STATUS_OK) {
            // while stored as an attribute, xml encoding is an XML attribute, not a srcML attribute
            SRCMLstatus(ERROR_MSG, "srcml: invalid xml encoding '%s'for srcml archive", *srcml_request.att_xml_encoding);
            exit(SRCML_STATUS_INVALID_ARGUMENT);
    }

    // source encoding
    if (srcml_request.src_encoding && srcml_archive_set_src_encoding(srcml_arch, srcml_request.src_encoding->c_str()) != SRCML_STATUS_OK) {
            SRCMLstatus(ERROR_MSG, "srcml: invalid source encoding '%s' for srcml archive", *srcml_request.src_encoding);
            exit(SRCML_STATUS_INVALID_ARGUMENT);
    }

    // for single input src archives (e.g., .tar), url attribute is the source url (if not already given)
    if (srcml_request.att_url) {
        std::string url = src_prefix_resource(*srcml_request.att_url);
        if (srcml_archive_set_url(srcml_arch, url.c_str()) != SRCML_STATUS_OK) {
            SRCMLstatus(ERROR_MSG, "srcml: invalid url attribute value '%s' for srcml archive", url);
            exit(SRCML_STATUS_INVALID_ARGUMENT);
        }
    } else if (input_sources.size() == 1 && input_sources[0].archives.size() > 0) {

        // Cleanup filename
        std::string url_name = src_prefix_resource(input_sources[0].filename);
        url_name = url_name.substr(url_name.find_first_not_of("./"));
        
        if (srcml_archive_set_url(srcml_arch, url_name.c_str()) != SRCML_STATUS_OK) {
            SRCMLstatus(ERROR_MSG, "srcml: invalid url '%s' for srcml archive", url_name);
            exit(SRCML_STATUS_INVALID_ARGUMENT);
        }
    }

    // version
    if (srcml_request.att_version && srcml_archive_set_version(srcml_arch, srcml_request.att_version->c_str()) != SRCML_STATUS_OK) {
            SRCMLstatus(ERROR_MSG, "srcml: invalid version attribute value for srcml archive");
            exit(SRCML_STATUS_INVALID_ARGUMENT);
    }

    // markup options
    // @todo Add error handling, and remove direct use of options
    if (srcml_request.markup_options) {

        if (*srcml_request.markup_options & SRCML_ARCHIVE)
            srcml_archive_enable_full_archive(srcml_arch);
        if (*srcml_request.markup_options & SRCML_OPTION_POSITION)
            srcml_archive_enable_option(srcml_arch, SRCML_OPTION_POSITION);
        if (*srcml_request.markup_options & SRCML_OPTION_CPP)
            srcml_archive_enable_option(srcml_arch, SRCML_OPTION_CPP);
        if (*srcml_request.markup_options & SRCML_OPTION_CPP_MARKUP_IF0)
            srcml_archive_enable_option(srcml_arch, SRCML_OPTION_CPP_MARKUP_IF0);
        if (*srcml_request.markup_options & SRCML_OPTION_CPP_TEXT_ELSE)
            srcml_archive_enable_option(srcml_arch, SRCML_OPTION_CPP_TEXT_ELSE);
        if (*srcml_request.markup_options & SRCML_OPTION_XML_DECL)
            srcml_archive_enable_option(srcml_arch, SRCML_OPTION_XML_DECL);
        if (*srcml_request.markup_options & SRCML_HASH)
            srcml_archive_enable_hash(srcml_arch);

//            SRCMLstatus(ERROR_MSG, "srcml: invalid options for srcml archive");
//            exit(SRCML_STATUS_INVALID_ARGUMENT);
    }

    // xml declaration
    if (*srcml_request.markup_options & SRCML_OPTION_XML_DECL) {
        srcml_archive_disable_option(srcml_arch, SRCML_OPTION_XML_DECL);
    } else {
        srcml_archive_enable_option(srcml_arch, SRCML_OPTION_XML_DECL);
    }

    // language
    auto language = srcml_request.att_language ? srcml_request.att_language->c_str() : SRCML_LANGUAGE_NONE;
    if (srcml_archive_set_language(srcml_arch, language) != SRCML_STATUS_OK) {
        SRCMLstatus(ERROR_MSG, "srcml: invalid language '%s' for srcml archive", language);
        exit(SRCML_STATUS_INVALID_ARGUMENT);
    }

    // tabstop
    if (srcml_archive_set_tabstop(srcml_arch, srcml_request.tabs) != SRCML_STATUS_OK) {
        SRCMLstatus(ERROR_MSG, "srcml: invalid tab stop for srcml archive", srcml_request.tabs);
        exit(SRCML_STATUS_INVALID_ARGUMENT);
    }

    // solo unit when:
    //   only one input
    //   no cli request to make it an archive
    //   not a directory (if local file)
    if (input_sources.size() == 1 && input_sources[0].protocol != "filelist" &&
        !(srcml_request.markup_options && (*srcml_request.markup_options & SRCML_ARCHIVE)) &&
        !input_sources[0].isdirectory && input_sources[0].archives.empty()) {

        srcml_archive_disable_full_archive(srcml_arch);
        
        // If --hash is used, force hash for single input
        if (*srcml_request.markup_options & SRCML_HASH) {
            if (srcml_archive_enable_hash(srcml_arch) != SRCML_STATUS_OK) {
                SRCMLstatus(ERROR_MSG, "srcml: unable to enable hash for srcml archive");
                exit(SRCML_STATUS_INVALID_ARGUMENT);
            }
        } else if (srcml_archive_disable_hash(srcml_arch) != SRCML_STATUS_OK) {
                SRCMLstatus(ERROR_MSG, "srcml: unable to disable hash for srcml archive");
                exit(SRCML_STATUS_INVALID_ARGUMENT);
        }

    } else {

        // if this is an archive, then no filename attribute is allowed
        if (srcml_request.att_filename) {
            fprintf(stderr, "Attribute filename cannot be set for a srcML archive. Use attribute uri instead.\n");
            exit(SRCML_STATUS_INVALID_ARGUMENT);
        }

        if (srcml_archive_enable_full_archive(srcml_arch) != SRCML_STATUS_OK) {
            SRCMLstatus(ERROR_MSG, "srcml: unable to enable full srcml archive");
            exit(SRCML_STATUS_INVALID_ARGUMENT);
        }

        if (srcml_archive_enable_hash(srcml_arch) != SRCML_STATUS_OK) {
            SRCMLstatus(ERROR_MSG, "srcml: unable to enable hash for srcml archive");
            exit(SRCML_STATUS_INVALID_ARGUMENT);
        }
    }

    // register file extension
    for (const auto& ext : srcml_request.language_ext) {
        auto pos = ext.find('=');
        const auto& extension = ext.substr(0, pos);
        const auto& language = ext.substr(pos+1);
        if (srcml_archive_register_file_extension(srcml_arch, extension.c_str(), language.c_str()) != SRCML_STATUS_OK) {
            SRCMLstatus(ERROR_MSG, "srcml: unable to register file extension '%s' for language '%s' for srcml archive", extension, language);
            exit(SRCML_STATUS_INVALID_ARGUMENT);
        }
    }

    // register xml namespaces
    for (const auto& ns : srcml_request.xmlns_namespaces) {
        const auto& prefix = ns.first;
        const auto& uri = ns.second;
        if (srcml_archive_register_namespace(srcml_arch, prefix.c_str(), uri.c_str()) != SRCML_STATUS_OK) {
            SRCMLstatus(ERROR_MSG, "srcml: unable to register namespace '%s:%s' for srcml archive", prefix, uri);
            exit(SRCML_STATUS_INVALID_ARGUMENT);
        }
    }

    // iterate through all transformations added during cli parsing
    int xpath_index = -1;
    for (const auto& trans : srcml_request.transformations) {

        std::string protocol;
        std::string resource;
        std::tie(protocol, resource) = src_prefix_split_uri(trans);

        if (protocol == "xpath") {
            if (apply_xpath(srcml_arch, srcml_arch, resource, srcml_request.xpath_query_support[++xpath_index], srcml_request.xmlns_namespaces) != SRCML_STATUS_OK) {
                SRCMLstatus(ERROR_MSG, "srcml: error with xpath transformation");
                exit(-1);
            }

        }

        if (protocol == "xslt") {
            if (apply_xslt(srcml_arch, resource) != SRCML_STATUS_OK) {
                SRCMLstatus(ERROR_MSG, "srcml: error with xslt transformation");
                exit(-1);
            }
        }
        
        if (protocol == "xslt-param") {

            // split resource into name and value
            auto pos = resource.find('=');
            std::string name = resource.substr(0, pos);
            std::string value = resource.substr(pos + 1);
            // @todo Should we always wrap?
            if (value[0] != '"') {
                value.insert(0, 1, '"');
                value.append("\"");
            }

            srcml_append_transform_param(srcml_arch, name.c_str(), value.c_str());
        }
/*
        // @todo Why is this not required?
        } else if (protocol == "relaxng") {
            if (apply_relaxng(in_arch, resource) != SRCML_STATUS_OK) {
                SRCMLstatus(ERROR_MSG, "srcml: error with relaxng transformation");
                exit(-1);
            }
        }
*/
    }

    // start tracing
    TraceLog log;

    // write queue for output of parsing
    WriteQueue write_queue(log, destination, !option(SRCML_COMMAND_OUTPUT_UNSTABLE_ORDER));

    // parsing queue
    ParseQueue parse_queue(srcml_request.max_threads, &write_queue);

    // convert input sources to srcml
    int status = 0;
    bool always_archive = option(SRCML_COMMAND_PARSER_TEST);    
    for (const auto& input : input_sources) {

        if (input.protocol == "filelist")
            always_archive = true;

        int numhandled = srcml_handler_dispatch(parse_queue, srcml_arch, srcml_request, input, destination);
        if (!numhandled)
            status = 1;
        if (numhandled == -1)
            status = -1;
    }

    // wait for the parsing queue to finish
    parse_queue.wait();

    // wait for the writing queue to finish
    write_queue.stop();

    if (SRCMLStatus::errors())
        status = -1;

    if (option(SRCML_COMMAND_CAT_XML)) {
        const char* endtag = "</unit>\n";
        srcml_archive_write_string(srcml_arch, endtag, (int) strlen(endtag));
    }

    if (option(SRCML_COMMAND_PARSER_TEST)) {
        ParserTest::report(srcml_arch);
    }

    if (status != -1 || always_archive) {
        srcml_archive_close(srcml_arch);
    }
    srcml_archive_free(srcml_arch);

    // @todo Why doesn't the srcml_archive_close() close this? Is that what libxml does?
    if (destination.fd)
        close(*destination.fd);

    if (SRCMLStatus::errors())
        exit(1);

    if (status) {

    }
}
