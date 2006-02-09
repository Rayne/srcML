/*
  srcMLOutput.cpp

  Copyright (C) 2002-2006  SDML (www.sdml.info)

  This file is part of the srcML translator.

  The srcML translator is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  The srcML translator is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with the srcML translator; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  XML output
*/

#include "srcMLOutput.h"
#include "srcMLToken.h"
#include "project.h"
#include "srcmlns.h"

#ifdef LIBXML_ENABLED
#include <libxml/xmlwriter.h>
#else
#include "xmlwriter.h"
#endif

#include "srcMLOutputPR.h"

const char* XML_DECLARATION_STANDALONE = "yes";
const char* XML_VERSION = "1.0";

// encoding handler
xmlCharEncodingHandlerPtr handler = 0;

// check if encoding is supported
bool srcMLOutput::checkEncoding(const char* encoding) {

  return xmlFindCharEncodingHandler(encoding) != 0;
}

xmlTextWriterPtr xout;

srcMLOutput::srcMLOutput(TokenStream* ints, 
			 const char* filename,
			 const char* language, 
			 const char* src_encoding,
			 const char* xml_enc,
			 int op)
  : input(ints), srcml_filename(filename), unit_language(language), unit_dir(""), unit_filename(""),
    unit_version(""), options(op), xml_encoding(xml_enc)
{
  // setup an output handler
  handler = xmlFindCharEncodingHandler(src_encoding);
  if (!handler)
    throw srcEncodingException();

  // fill the elements
  fillElementNames();

  if (isoption(OPTION_EXTENDED)) {
    // literal values
    ElementNames[SSTRING] = "literal";
    ElementNames[SCHAR] = "literal";
    ElementNames[SLITERAL] = "literal";
  }

  // only allow debug tags in debug
  if (!isoption(OPTION_DEBUG)) {
    ElementNames[SMARKER] = "";
    ElementNames[SERROR_PARSE]  = "";
    ElementNames[SERROR_MODE] = "";
  }

  // assign for special processing
  //  process_table[SUNIT] = &srcMLOutput::processUnit;
  process_table[BLOCKCOMMENT] = &srcMLOutput::processBlockComment;
  process_table[LINECOMMENT] = &srcMLOutput::processLineComment;
  process_table[SMARKER] = &srcMLOutput::processMarker;
  process_table[SPUBLIC_ACCESS_DEFAULT] = &srcMLOutput::processAccess;
  process_table[SPRIVATE_ACCESS_DEFAULT] = &srcMLOutput::processAccess;
  process_table[SSTRING] = &srcMLOutput::processString;
  process_table[SCHAR] = &srcMLOutput::processChar;
  process_table[SLITERAL] = &srcMLOutput::processLiteral;
  process_table[FORMFEED] = &srcMLOutput::processFormFeed;
  process_table[SINTERFACE] = &srcMLOutput::processInterface;

  // open the output text writer stream
  // "-" filename is standard output
  xout = xmlNewTextWriterFilename(srcml_filename, isoption(OPTION_COMPRESSED));

  // issue the xml declaration
  xmlTextWriterStartDocument(xout, XML_VERSION, xml_encoding, XML_DECLARATION_STANDALONE);
}

srcMLOutput::~srcMLOutput() {

  xmlTextWriterEndDocument(xout);

  xmlFreeTextWriter(xout);
}

void srcMLOutput::setTokenStream(TokenStream& ints) {

  input = &ints;
}

void srcMLOutput::consume(const char* directory, const char* filename, const char* version) {

  // store attributes so that first occurrence of unit element will be correct
  unit_dir = directory;
  unit_filename = filename;
  unit_version = version;

  // consume all input until EOF
  while (consume_next() != antlr::Token::EOF_TYPE) {

    // in interactive mode flush after each token is discovered
    if (isoption(OPTION_INTERACTIVE))
      xmlTextWriterFlush(xout);
  }
}

void srcMLOutput::consumeAll(const char* ofilename) {

  consume("", ofilename);

}

bool srcMLOutput::isoption(int flag) const {
  return (flag & options) > 0;
}

int srcMLOutput::consume_next() {


  const antlr::RefToken& token = input->nextToken();

  outputToken(token);

  return token->getType();
}

const char* srcMLOutput::token2name(const antlr::RefToken& token) const {

  return ElementNames[token->getType()];
}

const char* srcMLOutput::type2name(int token_type) const {

  return ElementNames[token_type];
}

// buffer of output utf8 characters
const int UTF8BUFFER_MAXSIZE = 5120;
unsigned char utf8buffer[UTF8BUFFER_MAXSIZE];

// amount of space for expanded characters.  assume a maximum of four bytes for every original single byte
const int UTF8BUFFER_SPACE = UTF8BUFFER_MAXSIZE / 4;

void srcMLOutput::processText(const std::string& str) {

  // extract c string from c++ string
  const char* inputbuffer = str.c_str();
  const unsigned int inputbuffer_size = (unsigned int) str.size();

#ifdef LIBXML_ENABLED

  // convert all of the input buffer to UTF-8 in chunks
  // conversion from libxml internal UTF-8 to output encoding is handled automatically
  unsigned int pos = 0;
  while (pos < inputbuffer_size) {

    int utf8buffer_size = UTF8BUFFER_MAXSIZE;
    int buffer_left = inputbuffer_size - pos;
    int partialinputbuffer_size = buffer_left < UTF8BUFFER_SPACE ? buffer_left : UTF8BUFFER_SPACE;
    int utf8buffer_newsize = handler->input(utf8buffer, &utf8buffer_size,
			    (const unsigned char*) (inputbuffer + pos), &partialinputbuffer_size);

    xmlTextWriterWriteRawLen(xout, BAD_CAST utf8buffer, utf8buffer_newsize);

    pos += partialinputbuffer_size;
  }
#else

  // output the buffer
  xmlTextWriterWriteRawLen(xout, BAD_CAST (unsigned char*) inputbuffer, inputbuffer_size);
#endif
}

void srcMLOutput::processText(const antlr::RefToken& token) {
  processText(token->getText());
}

void srcMLOutput::processFormFeed(const antlr::RefToken& token) {

  // start the element
  xmlTextWriterStartElement(xout, BAD_CAST token2name(token));

  xmlTextWriterEndElement(xout);
}

void srcMLOutput::startUnit(const char* language, const char* dir, const char* filename, const char* version, bool outer) {

    // start of main tag
    xmlTextWriterStartElement(xout, BAD_CAST "unit");

    // outer units have namespaces
    if (outer) {

      // main srcML namespace declaration
      xmlTextWriterWriteAttribute(xout, BAD_CAST "xmlns", BAD_CAST SRCML_SRC_NS_URI);

      // main cpp namespace declaration
      xmlTextWriterWriteAttribute(xout, BAD_CAST "xmlns:cpp", BAD_CAST SRCML_CPP_NS_URI);

      // optional debugging xml namespace
      if (isoption(OPTION_DEBUG))
	xmlTextWriterWriteAttribute(xout, BAD_CAST "xmlns:srcerr", BAD_CAST SRCML_ERR_NS_URI);
    }

    // language attribute
    if (language[0])
      xmlTextWriterWriteAttribute(xout, BAD_CAST "language", BAD_CAST language);

    // directory attribute
    if (dir[0])
      xmlTextWriterWriteAttribute(xout, BAD_CAST "dir", BAD_CAST dir);

    // filename attribute
    if (filename[0])
      xmlTextWriterWriteAttribute(xout, BAD_CAST "filename", BAD_CAST filename);

    // version attribute
    if (version[0])
      xmlTextWriterWriteAttribute(xout, BAD_CAST "version", BAD_CAST version);

    // leave space for nested unit
    if (outer && isoption(OPTION_NESTED))
      processText("\n\n");
}

void srcMLOutput::processUnit(const antlr::RefToken& token) {

  if (isstart(token)) {

    startUnit(unit_language, unit_dir, unit_filename, unit_version, !isoption(OPTION_NESTED));

  } else {

    // end the unit
    xmlTextWriterEndElement(xout);

    // leave a blank line before next nested unit even the last one
    if (isoption(OPTION_NESTED))
      processText("\n\n");
  }
}

void srcMLOutput::processAccess(const antlr::RefToken& token) {
  static const char* type_default = "default";

  if (!isstart(token)) {
    processToken(token);
    return;
  }

  // start the element
  xmlTextWriterStartElement(xout, BAD_CAST token2name(token));

  xmlTextWriterWriteAttribute(xout, BAD_CAST "type", BAD_CAST type_default);

  if (isempty(token))
    xmlTextWriterEndElement(xout);
}

void srcMLOutput::processToken(const antlr::RefToken& token) {

  const char* s = token2name(token);

  if (s[0] == 0)
    return;

  if (isstart(token) || isempty(token))
    xmlTextWriterStartElement(xout, BAD_CAST s);

  if (!isstart(token) || isempty(token))
    xmlTextWriterEndElement(xout);
}

void srcMLOutput::processLineComment(const antlr::RefToken& token) {

  const char* s = token2name(token);

  if (s[0] == 0)
    return;

  xmlTextWriterStartElement(xout, BAD_CAST s);

  xmlTextWriterWriteAttribute(xout, BAD_CAST "type", BAD_CAST "line");

  processText(token);

  xmlTextWriterEndElement(xout);
}

void srcMLOutput::processBlockComment(const antlr::RefToken& token) {
  static const char* BLOCK_COMMENT_ATTR = "block";
  static const char* JAVADOC_COMMENT_ATTR = "javadoc";

  const char* s = token2name(token);

  if (s[0] == 0)
    return;

  xmlTextWriterStartElement(xout, BAD_CAST s);

  xmlTextWriterWriteAttribute(xout, BAD_CAST "type",
     BAD_CAST (strcmp(unit_language, "Java") == 0 && token->getText().substr(0, 3) == "/**" ? JAVADOC_COMMENT_ATTR : BLOCK_COMMENT_ATTR));

  processText(token);

  xmlTextWriterEndElement(xout);
}

void srcMLOutput::processOptional(const antlr::RefToken& token, const char* attr_name, const char* attr_value) {

  const char* s = token2name(token);

  if (s[0] == 0)
    return;

  if (isstart(token)) {
    xmlTextWriterStartElement(xout, BAD_CAST s);
    xmlTextWriterWriteAttribute(xout, BAD_CAST attr_name, BAD_CAST attr_value);
  } else
    xmlTextWriterEndElement(xout);
}

void srcMLOutput::processString(const antlr::RefToken& token) {

  processOptional(token, "type", "string");
}

void srcMLOutput::processChar(const antlr::RefToken& token) {

  processOptional(token, "type", "char");
}

void srcMLOutput::processLiteral(const antlr::RefToken& token) {

  processOptional(token, "type", "number");
}

void srcMLOutput::processMarker(const antlr::RefToken& token) {

  const char* s = token2name(token);

  if (s[0] == 0)
    return;

  xmlTextWriterStartElement(xout, BAD_CAST s);

  xmlTextWriterWriteAttribute(xout, BAD_CAST "location", BAD_CAST token->getText().c_str());

  xmlTextWriterEndElement(xout);
}

void srcMLOutput::processInterface(const antlr::RefToken& token) {
  static const char* INTERFACE_ATTR = "interface";

  const char* s = token2name(token);

  if (s[0] == 0)
    return;

  if (isstart(token) || isempty(token)) {
    xmlTextWriterStartElement(xout, BAD_CAST s);

    xmlTextWriterWriteAttribute(xout, BAD_CAST "type", BAD_CAST INTERFACE_ATTR);
  }

  if (!isstart(token) || isempty(token))
    xmlTextWriterEndElement(xout);

}

inline void srcMLOutput::outputToken(const antlr::RefToken& token) {

  // use the array of pointers to methods to call the correct output routine
  ((*this).*(process_table[token->getType()]))(token);
}

// element names array
char* srcMLOutput::ElementNames[];

// fill the element names array
void srcMLOutput::fillElementNames() {

  ElementNames[SUNIT] = "unit";
  ElementNames[LINECOMMENT] = "comment";
  ElementNames[BLOCKCOMMENT] = ElementNames[LINECOMMENT];

  // No op
  ElementNames[SNOP] = "";

  // literal values
  ElementNames[SSTRING] = "";
  ElementNames[SCHAR] = "";
  ElementNames[SLITERAL] = "";

  // sub-statement elements
  ElementNames[SNAME] = "name";
  ElementNames[SONAME] = "";
  ElementNames[SCNAME] = "name";
  ElementNames[STYPE] = "type";
  ElementNames[SCONDITION] = "condition";
  ElementNames[SBLOCK] = "block";

  ElementNames[SEXPRESSION_STATEMENT] = "expr_stmt";
  ElementNames[SEXPRESSION] = "expr";

  ElementNames[SDECLARATION_STATEMENT] = "decl_stmt";
  ElementNames[SDECLARATION] = "decl";
  ElementNames[SDECLARATION_INITIALIZATION] = "init";

  ElementNames[SBREAK_STATEMENT] = "break";
  ElementNames[SCONTINUE_STATEMENT] = "continue";
  ElementNames[SGOTO_STATEMENT] = "goto";
  ElementNames[SLABEL_STATEMENT] = "label";

  ElementNames[STYPEDEF] = "typedef";
  ElementNames[SASM] = "asm";
  ElementNames[SMACRO_CALL] = "macro";
  ElementNames[SENUM] = "enum";

  ElementNames[SIF_STATEMENT] = "if";
  ElementNames[STHEN] = "then";
  ElementNames[SELSE] = "else";

  ElementNames[SWHILE_STATEMENT] = "while";
  ElementNames[SDO_STATEMENT] = "do";

  ElementNames[SSWITCH] = "switch";
  ElementNames[SCASE] = "case";
  ElementNames[SDEFAULT] = "default";

  ElementNames[SFOR_STATEMENT] = "for";
  ElementNames[SFOR_GROUP] = "";
  ElementNames[SFOR_INITIALIZATION] = "init";
  ElementNames[SFOR_CONDITION] = ElementNames[SCONDITION];
  ElementNames[SFOR_INCREMENT] = "incr";

  // functions
  ElementNames[SFUNCTION_DEFINITION]  = "function";
  ElementNames[SFUNCTION_DECLARATION] = "function_decl";
  ElementNames[SFUNCTION_SPECIFIER]   = "specifier";
  ElementNames[SRETURN_STATEMENT]     = "return";
  ElementNames[SFUNCTION_CALL]        = "call";
  ElementNames[SPARAMETER_LIST]       = "parameter_list";
  ElementNames[SPARAMETER]            = "param";
  ElementNames[SARGUMENT_LIST]        = "argument_list";
  ElementNames[SARGUMENT]             = "argument";

  // struct, union
  ElementNames[SSTRUCT] = "struct";
  ElementNames[SSTRUCT_DECLARATION]   = "struct_decl";
  ElementNames[SUNION]  = "union";
  ElementNames[SUNION_DECLARATION]    = "union_decl";

  // class
  ElementNames[SCLASS]                   = "class";
  ElementNames[SCLASS_DECLARATION]       = "class_decl";
  ElementNames[SPUBLIC_ACCESS]           = "public";
  ElementNames[SPUBLIC_ACCESS_DEFAULT]   = "public";
  ElementNames[SPRIVATE_ACCESS]          = "private";
  ElementNames[SPRIVATE_ACCESS_DEFAULT]  = "private";
  ElementNames[SPROTECTED_ACCESS]        = "protected";

  ElementNames[SMEMBER_INITIALIZATION_LIST]  = "member_list";
  ElementNames[SCONSTRUCTOR_DEFINITION]  = "constructor";
  ElementNames[SCONSTRUCTOR_DECLARATION] = "constructor_decl";
  ElementNames[SDESTRUCTOR_DEFINITION]   = "destructor";
  ElementNames[SDESTRUCTOR_DECLARATION]  = "destructor_decl";
  ElementNames[SDERIVATION_LIST]         = "super";
  ElementNames[SFRIEND]                  = "friend";
  ElementNames[SCLASS_SPECIFIER]         = "specifier";

  // extern definition
  ElementNames[SEXTERN] = "extern";

  // namespaces
  ElementNames[SNAMESPACE] = "namespace";
  ElementNames[SUSING_DIRECTIVE] = "using";

  // exception handling
  ElementNames[STRY_BLOCK]       = "try";
  ElementNames[SCATCH_BLOCK]     = "catch";
  ElementNames[STHROW_STATEMENT] = "throw";
  ElementNames[STHROW_SPECIFIER] = "throw";
  ElementNames[STHROW_SPECIFIER_JAVA] = "throws";

  // template
  ElementNames[STEMPLATE] = "template";
  ElementNames[STEMPLATE_ARGUMENT] = ElementNames[SARGUMENT];
  ElementNames[STEMPLATE_ARGUMENT_LIST] = ElementNames[SARGUMENT_LIST];
  ElementNames[STEMPLATE_PARAMETER] = ElementNames[SPARAMETER];
  ElementNames[STEMPLATE_PARAMETER_LIST] = ElementNames[SPARAMETER_LIST];

  // cpp
  ElementNames[SCPP_DIRECTIVE] = "cpp:directive";
  ElementNames[SCPP_FILENAME]  = "cpp:file";
  ElementNames[SCPP_INCLUDE]   = "cpp:include";
  ElementNames[SCPP_DEFINE]    = "cpp:define";
  ElementNames[SCPP_UNDEF]     = "cpp:undef";
  ElementNames[SCPP_LINE]      = "cpp:line";
  ElementNames[SCPP_IF]        = "cpp:if";
  ElementNames[SCPP_IFDEF]     = "cpp:ifdef";
  ElementNames[SCPP_IFNDEF]    = "cpp:ifndef";
  ElementNames[SCPP_ELSE]      = "cpp:else";
  ElementNames[SCPP_ELIF]      = "cpp:elif";
  ElementNames[SCPP_ENDIF]     = "cpp:endif";
  ElementNames[SCPP_THEN]      = "cpp:then";
  ElementNames[SCPP_PRAGMA]    = "cpp:pragma";
  ElementNames[SCPP_ERROR]     = "cpp:error";

  ElementNames[SMARKER]        = "srcerr:marker";
  ElementNames[SERROR_PARSE]   = "srcerr:parse";
  ElementNames[SERROR_MODE]    = "srcerr:mode";

  // Java elements
  ElementNames[SEXTENDS]       = "extends";
  ElementNames[SIMPLEMENTS]    = "implements";
  ElementNames[SIMPORT]        = "import";
  ElementNames[SPACKAGE]       = "package";
  ElementNames[SINTERFACE]     = "class";

  // special characters
  ElementNames[FORMFEED]       = "formfeed";
}
