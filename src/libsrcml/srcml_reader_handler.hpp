/**
 * @file srcml_reader_handler.hpp
 *
 * @copyright Copyright (C) 2013-2014 srcML, LLC. (www.srcML.org)
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef INCLUDED_SRCML_READER_HANDLER_HPP
#define INCLUDED_SRCML_READER_HANDLER_HPP

#include <srcSAXHandler.hpp>
#include <sax2_srcsax_handler.hpp>

#include <srcml_types.hpp>
#include <srcml_macros.hpp>
#include <srcml.h>

#include <unit_utilities.hpp>

#include <libxml/parser.h>
#include <stdio.h>
#include <srcmlns.hpp>

#include <string>
#include <vector>
#include <stack>

#include <cstring>

#include <mutex>
#include <condition_variable>
#include <boost/optional.hpp>

#define ATTR_LOCALNAME(pos) (pos * 5)
#define ATTR_PREFIX(pos) (pos * 5 + 1)
#define ATTR_URI(pos) (pos * 5 + 2)
#define ATTR_VALUE_START(pos) (pos * 5 + 3)
#define ATTR_VALUE_END(pos) (pos * 5 + 4)

/**
 * srcsax_attribute
 *
 * Data structure for a srcML/xml attribute
 */
struct attribute_t {

    /** attribute name */
    boost::optional<std::string> localname;

    /** attribute namespace prefix */
    boost::optional<std::string> prefix;

    /** attribute namespace uri */
    boost::optional<std::string> uri;

    /** attribute value */
    boost::optional<std::string> value;
};

/**
 * srcml_reader_handler
 *
 * Inherits from srcMLHandler to provide hooks into
 * SAX2 parsing. Provides starting and stoping using
 * threads.  Collects attributes, namespaces and srcML
 * from units.
 */
class srcml_reader_handler : public srcSAXHandler {

private :

    /** mutex to halt both threads on */
    std::mutex mutex;

    /** sax stop/start condition */
    std::condition_variable cond;

    /** collected root language */
    srcml_archive* archive = nullptr;

    /** collected unit language */
    srcml_unit* unit = nullptr;

    /** has reached end of parsing*/
    bool is_done = false;
    /** has passed root*/
    bool read_root = false;
    /** stop after collecting unit attribute*/
    bool collect_unit_header = false;
    /** collect srcML as parse*/
    bool collect_unit_body = false;

    /** terminate */
    bool terminate = false;

    /** indicate if we need to wait on the root */
    bool wait_root = true;

    /** skip internal unit elements */
    bool skip = false;

    /**
     * meta_tag
     *
     * Store a meta tag for later output.
     */
     struct meta_tag {

        /** metatags localname */
        std::string localname;

        /** metatags prefix */
       boost::optional<std::string> prefix;

        /** meta tags attributes */
        std::vector<const xmlChar*> attributes;

        /**
         * meta_tag
         * @param localname the meta tag name
         * @param prefix the meta tag prefix
         * @param num_attributes the number attributes on the meta tag
         * @param attributes the attributes on the meta tag
         *
         * Construct meta_tag from SAX data.
         */
        meta_tag(const char* localname, const char* prefix, int num_attributes, const xmlChar** attributes)
            : localname(localname) {

            if(prefix) this->prefix = std::string(prefix);

            this->attributes.reserve(num_attributes * 5);
            for (int pos = 0; pos < num_attributes * 5; ++pos) {
                this->attributes[pos] = attributes[pos];
            }
        }

        /**
         * meta_tag
         * @param other another meta_tag
         *
         * Copy constructor.
         */
        meta_tag(const meta_tag& other) {

            localname = other.localname;
            prefix = other.prefix;
            attributes = other.attributes;
        }

        /**
         * operator=
         * @param other another meta_tag
         *
         * Overloaded assignment operator
         * Returns the assigned to meta_tag
         */
        meta_tag& operator=(meta_tag& other) {

            swap(other);

            return *this;
        }

        /**
         * swap
         * @param other another meta_tag
         *
         * swap the contents of the meta tags.
         */
        void swap(meta_tag & other) {

            std::swap(localname, other.localname);
            std::swap(prefix, other.prefix);
            std::swap(attributes, other.attributes);
        }

        /**
         * ~meta_tag
         *
         * Destructor
         */
        ~meta_tag() {}

        /**
         * get_prefix
         *
         * Return prefix as c string.
         */
       const char * get_prefix() const {
     if(prefix) return prefix->c_str();
     return 0;

       }


     };

    /** save meta tags to use when non-archive write unit */
    std::vector<meta_tag> meta_tags;

public :

    /** Give access to members for srcml_sax2_reader class */
    friend class srcml_sax2_reader;

    /**
     * srcml_reader_handler
     *
     * Constructor.  Sets up mutex, conditions and state.
     */
    srcml_reader_handler() {
//        srcml_archive_disable_option(archive, SRCML_OPTION_HASH);
    }

    /**
     * ~srcml_reader_handler
     *
     * Destructor, deletes mutex and conditions.
     */
    ~srcml_reader_handler() {
     }

    /**
     * stop_parser
     *
     * Stop the parser for threading
     */
    void stop_parser() {

        is_done = true;
        srcSAXHandler::stop_parser();
    }

    /**
     * wait
     *
     * Allows calling thread to wait until reached
     * end of unit.
     */
    void wait() {

        std::unique_lock<std::mutex> lock(mutex);

        if (is_done)
            return;

        if (wait_root)
            cond.wait(lock);
    }

    /**
     * resume
     *
     * Resume SAX2 execution.
     */
    void resume() {

        std::unique_lock<std::mutex> lock(mutex);

        cond.notify_one();
    }

    /**
     * resume_and_wait
     *
     * Atomic resume SAX2 execution then wait.
     */
    void resume_and_wait() {

        std::unique_lock<std::mutex> lock(mutex);
        cond.notify_one();
        if (is_done)
            return;

        cond.wait(lock);
    }

    /**
     * done
     *
     * Mark is done
     */
    void done() {

        is_done = true;
        
        cond.notify_one();
    }

    /**
     * stop
     *
     * Stops SAX2 parsing Completely.  Parsing
     * Can not be restarted.
     */
    void stop() {

        {
            std::unique_lock<std::mutex> lock(mutex);

            terminate = true;
        }

        resume();
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

    /**
     * startDocument
     *
     * SAX handler function for start of document.
     * Overide for desired behaviour.
     */
    virtual void startDocument() {

        srcml_archive_set_xml_encoding(archive, encoding ? encoding : "UTF-8");
    }

    /**
     * startRoot
     * @param localname the name of the element tag
     * @param prefix the tag prefix
     * @param URI the namespace of tag
     * @param num_namespaces number of namespaces definitions
     * @param namespaces the defined namespaces
     * @param num_attributes the number of attributes on the tag
     * @param attributes list of attributes
     *
     * Overidden startRoot to handle collection of root attributes. Stop before continue
     */
    virtual void startRoot(const char* localname, const char* prefix, const char* URI,
                           int num_namespaces, const xmlChar** namespaces, int num_attributes,
                           const xmlChar** attributes) {

#ifdef SRCSAX_DEBUG
        fprintf(stderr, "HERE: %s %s %d '%s'\n", __FILE__, __FUNCTION__, __LINE__, (const char *)localname);
#endif

        if (!is_archive)
            srcml_archive_disable_option(archive, SRCML_OPTION_ARCHIVE);

        // collect attributes
        for (int pos = 0; pos < num_attributes; ++pos) {
            std::string attribute = (const char*) attributes[pos * 5];
            std::string value((const char *)attributes[pos * 5 + 3], attributes[pos * 5 + 4] - attributes[pos * 5 + 3]);

            // Note: these are ignore instead of placing in attributes.
            if (attribute == "timestamp")
                ;
            else if (attribute == "language")
                ;
            else if (attribute == "revision")
                archive->revision = value;
            else if (attribute == "filename")
                ;
            else if (attribute == "url") {
                srcml_archive_set_url(archive, value.c_str());

            }
            else if (attribute == "version")
                srcml_archive_set_version(archive, value.c_str());
            else if (attribute == "tabs")
                archive->tabstop = atoi(value.c_str());
            else if (attribute == "options") {

                while(!value.empty()) {

                    std::string::size_type pos = value.find(",");
                    std::string option = value.substr(0, pos);
                    if (pos == std::string::npos)
                        value = "";
                    else
                        value = value.substr(value.find(",") + 1);

                    if (option == "XMLDECL")
                        archive->options |= SRCML_OPTION_XML_DECL;
                    else if (option == "NAMESPACEDECL")
                        archive->options |= SRCML_OPTION_NAMESPACE_DECL;
                    else if (option == "CPP_TEXT_ELSE")
                        archive->options |= SRCML_OPTION_CPP_TEXT_ELSE;
                    else if (option == "CPP_MARKUP_IF0")
                        archive->options |= SRCML_OPTION_CPP_MARKUP_IF0;
                    else if (option == "LINE")
                        archive->options |= SRCML_OPTION_LINE;
                }

            } else if (attribute == "hash") 
                ;
            else {

                archive->attributes.push_back(attribute);
                archive->attributes.push_back(value);
            }
        }

        // collect namespaces
        for (int pos = 0; pos < num_namespaces; ++pos) {

            std::string prefix = (const char*) namespaces[pos * 2] ? (const char*) namespaces[pos * 2] : "";
            std::string uri = (const char*) namespaces[pos * 2 + 1] ? (const char*) namespaces[pos * 2 + 1] : "";

            srcml_uri_normalize(uri);

            srcml_archive_register_namespace(archive, prefix.c_str(), uri.c_str());
        }

#ifdef SRCSAX_DEBUG
        fprintf(stderr, "HERE: %s %s %d '%s'\n", __FILE__, __FUNCTION__, __LINE__, (const char *)localname);
#endif
    }

    /**
     * startUnit
     * @param localname the name of the element tag
     * @param prefix the tag prefix
     * @param URI the namespace of tag
     * @param num_namespaces number of namespaces definitions
     * @param namespaces the defined namespaces
     * @param num_attributes the number of attributes on the tag
     * @param attributes list of attributes
     *
     * Overidden startUnit to handle collection of Unit attributes and tag. Stop before continue
     * if collecting attributes.
     */
    virtual void startUnit(const char* localname, const char* prefix, const char* URI,
                           int num_namespaces, const xmlChar** namespaces, int num_attributes,
                           const xmlChar** attributes) {

#ifdef SRCSAX_DEBUG
        fprintf(stderr, "HERE: %s %s %d '%s'\n", __FILE__, __FUNCTION__, __LINE__, (const char *)localname);
#endif

        // pause
        // @todo this may need to change because, meta tags have separate call now
        if (!read_root) {

            {                
                std::unique_lock<std::mutex> lock(mutex);

                if (terminate)
                    stop_parser();
                wait_root = false;
                cond.notify_one();
                cond.wait(lock);
                read_root = true;        
            }

            if (terminate) {

                stop_parser();
                return;
            }
        }

        // collect attributes
        unit_update_attributes(unit, num_attributes, attributes);

        auto ctxt = (xmlParserCtxtPtr) get_controller().getContext()->libxml2_context;
        auto state = (sax2_srcsax_handler*) ctxt->_private;

        state->collect_unit_body = collect_unit_body;

        if (collect_unit_header) {

            // pause
            std::unique_lock<std::mutex> lock(mutex);
            if (terminate)
                stop_parser();

            cond.notify_one();
            cond.wait(lock);
        }

        state->collect_unit_body = collect_unit_body;

        if (terminate)
            stop_parser();

#ifdef SRCSAX_DEBUG
        fprintf(stderr, "HERE: %s %s %d '%s'\n", __FILE__, __FUNCTION__, __LINE__, (const char *)localname);
#endif
    }

    /**
     * endRoot
     * @param localname tag name
     * @param prefix prefix for the tag
     * @param URI uri for tag
     *
     * Overidden endRoot to indicate done with parsing and
        free any waiting process.
     */
    virtual void endRoot(const char* localname, const char* prefix, const char* URI) {

#ifdef SRCSAX_DEBUG
        fprintf(stderr, "HERE: %s %s %d '%s'\n", __FILE__, __FUNCTION__, __LINE__, (const char *)localname);
#endif

        {
            std::unique_lock<std::mutex> lock(mutex);
            if (terminate)
                stop_parser();
            is_done = true;
            cond.notify_one();
        }

        if (terminate)
            stop_parser();

#ifdef SRCSAX_DEBUG
        fprintf(stderr, "HERE: %s %s %d '%s'\n", __FILE__, __FUNCTION__, __LINE__, (const char *)localname);
#endif
    }

    /**
     * endUnit
     * @param localname tag name
     * @param prefix prefix for the tag
     * @param URI uri for tag
     *
     * Overidden endUnit to collect srcml and stop parsing.  Clear collect srcML after pause.
     */
    virtual void endUnit(const char* localname, const char* prefix, const char* URI) {

#ifdef SRCSAX_DEBUG
        fprintf(stderr, "HERE: %s %s %d '%s'\n", __FILE__, __FUNCTION__, __LINE__, (const char *)localname);
#endif

        auto ctxt = (xmlParserCtxtPtr) get_controller().getContext()->libxml2_context;
        auto state = (sax2_srcsax_handler*) ctxt->_private;

        if (collect_unit_body) {

            unit->content_begin = state->content_begin;
            unit->content_end = state->content_end;
            unit->insert_begin = state->insert_begin;
            unit->insert_end = state->insert_end;
            unit->srcml = std::move(state->unitsrcml);
            unit->src = std::move(state->unitsrc);
            unit->loc = state->loc;

            // pause
            std::unique_lock<std::mutex> lock(mutex);
            if (terminate) stop_parser();
            cond.notify_one();
            cond.wait(lock);
        }

        if (terminate)
            stop_parser();

#ifdef SRCSAX_DEBUG
        fprintf(stderr, "HERE: %s %s %d '%s'\n", __FILE__, __FUNCTION__, __LINE__, (const char *)localname);
#endif
    }


    /**
     * metaTag
     * @param localname the name of the element tag
     * @param prefix the tag prefix
     * @param URI the namespace of tag
     * @param num_namespaces number of namespaces definitions
     * @param namespaces the defined namespaces
     * @param num_attributes the number of attributes on the tag
     * @param attributes list of attributes\
     *
     * SAX handler function for a meta tags.
     * Overide for desired behaviour.
     */
    virtual void metaTag(const char* localname, const char* prefix, const char* URI,
                           int num_namespaces, const xmlChar** namespaces, int num_attributes,
                           const xmlChar** attributes) {

        if (strcmp(localname, "macro-list") == 0) {

            std::string token;
            std::string type;

            for (int pos = 0; pos < num_attributes; ++pos) {

                if (strcmp((const char*) attributes[ATTR_LOCALNAME(pos)], "token") == 0)
                    token.append((const char*) attributes[ATTR_VALUE_START(pos)], attributes[ATTR_VALUE_END(pos)] - attributes[ATTR_VALUE_START(pos)]);
                else if (strcmp((const char*) attributes[ATTR_LOCALNAME(pos)], "type") == 0)
                    type.append((const char*) attributes[ATTR_VALUE_START(pos)], attributes[ATTR_VALUE_END(pos)] - attributes[ATTR_VALUE_START(pos)]);
            }

            if (token != "" && type != "") {

                archive->user_macro_list.push_back(token);
                archive->user_macro_list.push_back(type);
            }
        } else if (!is_archive) {

            meta_tags.push_back(meta_tag(localname, prefix, num_attributes, attributes));
        }
    }

    /**
     * processingInstruction
     * @param target the processing instruction target.
     * @param data the processing instruction data.
     *
     * Overrident processingInstruction to collect srcML.
     */
    virtual void processingInstruction(const char* target, const char* data) {
        
        srcml_archive_set_processing_instruction(archive, (const char*)target, (const char *)data);

    }

#pragma GCC diagnostic pop
};

#endif
