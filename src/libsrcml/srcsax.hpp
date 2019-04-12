/**
 * @file srcsax.hpp
 *
 * @copyright Copyright (C) 2013-2014 srcML, LLC. (www.srcML.org)
 *
 * srcSAX is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * srcSAX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the srcML Toolkit; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef INCLUDED_SRCSAX_HPP
#define INCLUDED_SRCSAX_HPP

#include <srcsax_handler.hpp>

#include <libxml/parser.h>

/**
 * srcsax_context
 *
 * Context data structure passed between callbacks.
 */
class srcsax_context {
public:
    /** user provided data */
    void* data = nullptr;

    /** srcSAX handler callbacks */
    srcsax_handler* handler = nullptr;

    /** error callback need to figure this one out probably message and errorcode. or struct.  Might not need, but might be nice to avoid libxml2 stuff */
    void (*srcsax_error)(const char* message, int error_code);

    /** is the document an archive */
    int is_archive;

    /** the xml documents encoding */
    const char* encoding;

    /* Internal context handling NOT FOR PUBLIC USE */

    /** xml parser input buffer */
    xmlParserInputBufferPtr input = nullptr;

    /** internally used libxml2 context */
    xmlParserCtxtPtr libxml2_context = nullptr;
};

/* srcSAX context creation/open functions */
srcsax_context* srcsax_create_context_parser_input_buffer(xmlParserInputBufferPtr input);

/* srcSAX free function */
void srcsax_free_context(srcsax_context * context);

/* srcSAX parse function */
int srcsax_parse(srcsax_context * context);

/* srcSAX terminate parse function */
void srcsax_stop_parser(srcsax_context* context);

#endif
