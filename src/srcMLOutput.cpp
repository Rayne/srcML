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

#include "srcMLOutputPR.h"

#include <cstring>

const char* const XML_DECLARATION_STANDALONE = "yes";
const char* const XML_VERSION = "1.0";

// check if encoding is supported
bool srcMLOutput::checkEncoding(const char* encoding) {

  return xmlFindCharEncodingHandler(encoding) != 0;
}

srcMLOutput::srcMLOutput(TokenStream* ints, 
			 const char* filename,
			 const char* language, 
			 const char* xml_enc,
			 int op,
			 std::map<std::string, std::string>& curi
			 )
  : input(ints), xout(0), srcml_filename(filename), unit_language(language), unit_dir(0), unit_filename(0),
    unit_version(0), options(op), xml_encoding(xml_enc), uri(curi)
{
  // fill the prefixes
  for (int i = 0; i < END_ELEMENT_TOKEN; ++i)
    ElementPrefix[i] = (char*) uri[SRCML_SRC_NS_URI].c_str();

  // fill the elements
  fillElementNames();

  for (int i = SCPP_DIRECTIVE; i <= SCPP_ENDIF; ++i)
    ElementPrefix[i] = (char*) uri[SRCML_CPP_NS_URI].c_str();

  for (int i = SMARKER; i <= SERROR_MODE; ++i)
    ElementPrefix[i] = (char*) uri[SRCML_ERR_NS_URI].c_str();

  if (isoption(OPTION_LITERAL)) {
    // literal values
    ElementNames[SSTRING] = "literal";
    ElementNames[SCHAR] = "literal";
    ElementNames[SLITERAL] = "literal";
    ElementNames[SBOOLEAN] = "literal";

    ElementPrefix[SSTRING] = (char*) uri[SRCML_EXT_LITERAL_NS_URI].c_str();
    ElementPrefix[SCHAR] = ElementPrefix[SSTRING];
    ElementPrefix[SLITERAL] = ElementPrefix[SSTRING];
    ElementPrefix[SBOOLEAN] = ElementPrefix[SSTRING];
  }

  // only allow debug tags in debug
  if (!isoption(OPTION_DEBUG)) {
    ElementNames[SMARKER] = "";
    ElementNames[SERROR_PARSE]  = "";
    ElementNames[SERROR_MODE] = "";
  }

  if (isoption(OPTION_OPERATOR)) {
    ElementPrefix[SOPERATOR] = (char*) uri[SRCML_EXT_OPERATOR_NS_URI].c_str();
    ElementNames[SOPERATOR] = "operator";
  }

  if (isoption(OPTION_MODIFIER)) {
    ElementPrefix[SMODIFIER] = (char*) uri[SRCML_EXT_MODIFIER_NS_URI].c_str();
    ElementNames[SMODIFIER] = "modifier";
  }

  // assign for special processing
  //  process_table[SUNIT] = &srcMLOutput::processUnit;
  process_table[COMMENT_START] = &srcMLOutput::processBlockCommentStart;
  process_table[COMMENT_END] = &srcMLOutput::processEndBlockToken;

  process_table[LINECOMMENT_START] = &srcMLOutput::processLineCommentStart;
  process_table[LINECOMMENT_END] = &srcMLOutput::processEndLineToken;

#if DEBUG
  process_table[SMARKER] = &srcMLOutput::processMarker;
#endif
  process_table[SPUBLIC_ACCESS_DEFAULT] = &srcMLOutput::processAccess;
  process_table[SPRIVATE_ACCESS_DEFAULT] = &srcMLOutput::processAccess;
  process_table[SSTRING] = &srcMLOutput::processString;
  process_table[SCHAR] = &srcMLOutput::processChar;
  process_table[SLITERAL] = &srcMLOutput::processLiteral;
  process_table[SBOOLEAN] = &srcMLOutput::processBoolean;
  process_table[SINTERFACE] = &srcMLOutput::processInterface;
  process_table[CONTROL_CHAR] = &srcMLOutput::processEscape;

  // open the output text writer stream
  // "-" filename is standard output
  xout = xmlNewTextWriterFilename(srcml_filename, isoption(OPTION_COMPRESSED));

  // issue the xml declaration, but only if we want to
  if (isoption(OPTION_XMLDECL))
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

bool srcMLOutput::isoption(int flag) const {
  return (flag & options) > 0;
}

int srcMLOutput::consume_next() {

  const antlr::RefToken& token = input->nextToken();

  outputToken(token);

  return token->getType();
}

const char* srcMLOutput::token2name(const antlr::RefToken& token) const {

  return type2name(token->getType());
}

const char* srcMLOutput::type2name(int token_type) const {

  static char s[512];

  // no element name
  if (ElementNames[token_type][0] == '\0')
    return "";

  // non-default namespace name
  if (ElementPrefix[token_type][0] != '\0') {
    
    strcpy(s, ElementPrefix[token_type]);
    strcat(s, ":");
    strcat(s, ElementNames[token_type]);
    return s;
  }

  // default namespace name
  return ElementNames[token_type];
}

// output text
void srcMLOutput::processText(const std::string& str) {

  xmlTextWriterWriteRawLen(xout, BAD_CAST (unsigned char*) str.c_str(), str.size());
}

void srcMLOutput::processText(const antlr::RefToken& token) {
  processText(token->getText());
}

void srcMLOutput::processEscape(const antlr::RefToken& token) {

  const char* s = token2name(token);

  if (s[0] == 0)
    return;

  xmlTextWriterStartElement(xout, BAD_CAST s);
  
  xmlTextWriterWriteAttribute(xout, BAD_CAST "char", BAD_CAST token->getText().c_str());

  xmlTextWriterEndElement(xout);
}

void srcMLOutput::startUnit(const char* language, const char* dir, const char* filename, const char* version, bool outer) {

    // start of main tag
    xmlTextWriterStartElement(xout, BAD_CAST type2name(SUNIT));

    // outer units have namespaces
    if (outer && isoption(OPTION_NAMESPACEDECL)) {

      // figure out which namespaces are needed
      const char* const ns[] = { 

	                    // main srcML namespace declaration always used
	                    SRCML_SRC_NS_URI, 

			    // main cpp namespace declaration
			    isoption(OPTION_CPP)      ? SRCML_CPP_NS_URI : 0,

			    // optional debugging xml namespace
			    isoption(OPTION_DEBUG)    ? SRCML_ERR_NS_URI : 0,

			    // optional literal xml namespace
			    isoption(OPTION_LITERAL)  ? SRCML_EXT_LITERAL_NS_URI : 0,

			    // optional operator xml namespace
			    isoption(OPTION_OPERATOR) ? SRCML_EXT_OPERATOR_NS_URI : 0,

			    // optional modifier xml namespace
			    isoption(OPTION_MODIFIER) ? SRCML_EXT_MODIFIER_NS_URI : 0,
			  };

      // output the namespaces
      for (unsigned int i = 0; i < sizeof(ns) / sizeof(ns[0]); ++i) {
	if (!ns[i])
	  continue;

	std::string src_prefix = "xmlns";
	if (uri[ns[i]][0] != '\0') {
	  src_prefix += ":";
	  src_prefix += uri[ns[i]];
	}
	xmlTextWriterWriteAttribute(xout, BAD_CAST src_prefix.c_str(), BAD_CAST ns[i]);
      }
    }

    // list of attributes
    const char* attrs[][2] = {

      // language attribute
      { language, "language" },

      // directory attribute
      { dir, "dir" },

      // filename attribute
      { filename, "filename" },

      // version attribute
      { version, "version" },
    };

    // output attributes
    for (unsigned int i = 0; i < sizeof(attrs) / sizeof(attrs[0]); ++i) {
      if (!attrs[i][0])
	continue;

      xmlTextWriterWriteAttribute(xout, BAD_CAST attrs[i][1], BAD_CAST attrs[i][0]);
    }

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

void srcMLOutput::processBlockCommentStart(const antlr::RefToken& token) {
  static const char* BLOCK_COMMENT_ATTR = "block";
  static const char* JAVADOC_COMMENT_ATTR = "javadoc";

  const char* s = token2name(token);

  if (s[0] == 0)
    return;

  xmlTextWriterStartElement(xout, BAD_CAST s);

  xmlTextWriterWriteAttribute(xout, BAD_CAST "type",
     BAD_CAST (strcmp(unit_language, "Java") == 0 && token->getText().substr(0, 3) == "/**" ? JAVADOC_COMMENT_ATTR : BLOCK_COMMENT_ATTR));

  processText(token);
}

void srcMLOutput::processLineCommentStart(const antlr::RefToken& token) {
  static const char* LINE_COMMENT_ATTR = "line";

  const char* s = token2name(token);

  if (s[0] == 0)
    return;

  xmlTextWriterStartElement(xout, BAD_CAST s);

  xmlTextWriterWriteAttribute(xout, BAD_CAST "type", BAD_CAST LINE_COMMENT_ATTR);

  processText(token);
}

void srcMLOutput::processEndLineToken(const antlr::RefToken& token) {

  const char* s = token2name(token);

  if (s[0] == 0)
    return;

  xmlTextWriterEndElement(xout);

  processText(token);
}

void srcMLOutput::processEndBlockToken(const antlr::RefToken& token) {

  processText(token);

  const char* s = token2name(token);

  if (s[0] == 0)
    return;

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

void srcMLOutput::processBoolean(const antlr::RefToken& token) {

  processOptional(token, "type", "boolean");
}

#if DEBUG
void srcMLOutput::processMarker(const antlr::RefToken& token) {

  const char* s = token2name(token);

  if (s[0] == 0)
    return;

  xmlTextWriterStartElement(xout, BAD_CAST s);

  xmlTextWriterWriteAttribute(xout, BAD_CAST "location", BAD_CAST token->getText().c_str());

  xmlTextWriterEndElement(xout);
}
#endif

void srcMLOutput::processInterface(const antlr::RefToken& token) {

  processOptional(token, "type", "interface");
}

inline void srcMLOutput::outputToken(const antlr::RefToken& token) {

  // use the array of pointers to methods to call the correct output routine
  ((*this).*(process_table[token->getType()]))(token);
}

// element names array
const char* srcMLOutput::ElementNames[];
const char* srcMLOutput::ElementPrefix[];

// fill the element names array
void srcMLOutput::fillElementNames() {

  ElementNames[SUNIT] = "unit";
  ElementNames[COMMENT_START] = "comment";
  ElementNames[COMMENT_END] = ElementNames[COMMENT_START];
  ElementNames[LINECOMMENT_START] = ElementNames[COMMENT_START];
  ElementNames[LINECOMMENT_END] = ElementNames[COMMENT_START];

  // No op
  ElementNames[SNOP] = "";

  // literal values
  ElementNames[SSTRING] = "";
  ElementNames[SCHAR] = "";
  ElementNames[SLITERAL] = "";
  ElementNames[SBOOLEAN] = "";

  // operators
  ElementNames[SOPERATOR] = "";

  // type modifier
  ElementNames[SMODIFIER] = "";

  // sub-statement elements
  ElementNames[SNAME] = "name";
  ElementNames[SONAME] = "";
  ElementNames[SCNAME] = "name";
  ElementNames[STYPE] = "type";
  ElementNames[SCONDITION] = "condition";
  ElementNames[SBLOCK] = "block";
  ElementNames[SINDEX] = "index";

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
  ElementNames[SCPP_DIRECTIVE] = "directive";
  ElementNames[SCPP_FILENAME]  = "file";
  ElementNames[SCPP_INCLUDE]   = "include";
  ElementNames[SCPP_DEFINE]    = "define";
  ElementNames[SCPP_UNDEF]     = "undef";
  ElementNames[SCPP_LINE]      = "line";
  ElementNames[SCPP_IF]        = "if";
  ElementNames[SCPP_IFDEF]     = "ifdef";
  ElementNames[SCPP_IFNDEF]    = "ifndef";
  ElementNames[SCPP_ELSE]      = "else";
  ElementNames[SCPP_ELIF]      = "elif";
  ElementNames[SCPP_ENDIF]     = "endif";
  ElementNames[SCPP_THEN]      = "then";
  ElementNames[SCPP_PRAGMA]    = "pragma";
  ElementNames[SCPP_ERROR]     = "error";

  ElementNames[SMARKER]        = "marker";
  ElementNames[SERROR_PARSE]   = "parse";
  ElementNames[SERROR_MODE]    = "mode";

  // Java elements
  ElementNames[SEXTENDS]       = "extends";
  ElementNames[SIMPLEMENTS]    = "implements";
  ElementNames[SIMPORT]        = "import";
  ElementNames[SPACKAGE]       = "package";
  ElementNames[SINTERFACE]     = "class";

  // special characters
  ElementNames[CONTROL_CHAR]   = "escape";

  // 
  ElementNames[SEMPTY]         = "empty_stmt";

  // C++0x elements
  ElementNames[SCONCEPT]       = "concept";
  ElementNames[SCONCEPTMAP]    = "concept_map";
}
