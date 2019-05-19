/**
 * @file src_input_s3.cpp
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

#include <src_input_s3.hpp>

#ifdef LINKING_WITH_AWS_SDK

#include <srcml_options.hpp>
#include <src_input_libarchive.hpp>
#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/core/client/ClientConfiguration.h>
#include <string>
#include <src_input_libarchive.hpp>
#include <src_prefix.hpp>
#include <ctype.h>
#include <cstring>

static int hex2decimal(unsigned char c) {

    switch (c) {
        case 'a':
        case 'A': return 10;
        case 'b':
        case 'B': return 11;
        case 'c':
        case 'C': return 12;
        case 'd':
        case 'D': return 13;
        case 'e':
        case 'E': return 14;
        case 'f':
        case 'F': return 15;
        default: return c - '0';
    }
}

static bool isodigit(char c) {
    return c >= '0' && c <= '7';
}

// Convert input to a ParseRequest and assign request to the processing queue
int src_input_s3(ParseQueue& queue,
                    srcml_archive* srcml_arch,
                    const srcml_request_t& srcml_request,
                    const srcml_input_src& input) {

    std::string bucketname;
    std::string objectname;
    bucketname = input.resource.substr(0, input.resource.find('/'));
    objectname = input.resource.substr(bucketname.length() + 1);
    
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {

      Aws::Client::ClientConfiguration clientConfig;
      clientConfig.region = Aws::Region::US_EAST_1;

      // Assign these values before running the program
      const Aws::String bucket_name = bucketname.c_str();
      const Aws::String object_name = objectname.c_str();

      // Set up the request
      Aws::S3::S3Client s3_client(clientConfig);
      Aws::S3::Model::GetObjectRequest object_request;
      object_request.SetBucket(bucket_name);
      object_request.SetKey(object_name);

      // Get the object
      auto get_object_outcome = s3_client.GetObject(object_request);
      if (get_object_outcome.IsSuccess())
      {
          // Get an Aws::IOStream reference to the retrieved file
          auto &retrieved_file = get_object_outcome.GetResultWithOwnership().GetBody();

          std::string file = "";
          char file_data[512] = { 0 };
          while (retrieved_file.getline(file_data, 511)) {
            std::string line(file_data);
            file.append(line+"\n");
          }

          // fprintf(stderr, "%s", file.c_str());

          const char* ptext = file.c_str();

    // process text, which may have more than one input due to use of ASCII NUL ('\0')
    int count = 0;
    while (ptext) {

        // form the parsing request
        std::shared_ptr<ParseRequest> prequest(new ParseRequest);

        if (option(SRCML_COMMAND_NOARCHIVE))
            prequest->disk_dir = srcml_request.output_filename;

        prequest->filename = srcml_request.att_filename;
        // fprintf(stderr, "%s", prequest->filename->c_str());
        prequest->url = srcml_request.att_url;
        prequest->version = srcml_request.att_version;
        prequest->srcml_arch = srcml_arch;
        if (const char* l = srcml_archive_check_extension(srcml_arch, input.resource.c_str()))
          prequest->language = l;
        //             prequest->language = l;
        // if (prequest->language.empty()) {
        //     if(!prequest->filename->empty()) {
        //         if (const char* l = srcml_archive_check_extension(srcml_arch, input.resource.c_str()))
        //             prequest->language = l;
        //     }
        //     else {
        //         if (const char* l = srcml_archive_check_extension(srcml_arch, prequest->filename->c_str()))
        //             prequest->language = l;
        //     }
        // }
        // if (input.extension == ".cpp") {
        //   prequest->language = "C++";
        // } 
        //prequest->language = srcml_request.att_language ? *srcml_request.att_language : "";

        //fprintf(stderr, "%s", prequest->language.c_str());

        prequest->status = 0; //!language.empty() ? 0 : SRCML_STATUS_UNSET_LANGUAGE;

        prequest->loc = 0;

        // fill up the parse request buffer
        if (!prequest->status) {
            // copy from the text directly into a buffer
            // perform newline and tab expansion
            // TODO: Make test cases for each part
            // TODO: Support \nnn, \xnnn, \unnn, \Unnnnnnnn

            while (ptext) {

                // find up to an escape
                const char* epos = strchr(ptext, '\\');
                if (!epos) {
                    break;
                }
                // append up to the special char
                prequest->buffer.insert(prequest->buffer.end(), ptext, epos);

                // append the special character
                ++epos;
                switch (*epos) {
                case 'n':
                    prequest->buffer.push_back('\n');
                    ++prequest->loc;
                    break;
                case 't':
                    prequest->buffer.push_back('\t');
                    break;
                case 'f':
                    prequest->buffer.push_back('\f');
                    break;
                case 'a':
                    prequest->buffer.push_back('\a');
                    break;
                case 'b':
                    prequest->buffer.push_back('\b');
                    break;
                /* \e not directly supported in C, but echo command does */
                case 'e':
                    prequest->buffer.push_back('\x1B');
                    break;
                case 'r':
                    prequest->buffer.push_back('\r');
                    break;
                case 'v':
                    prequest->buffer.push_back('\v');
                    break;
                // byte with hex value from 1 to 2 charcters
                case 'x':
                {
                    int value = 0;
                    int offset = 0;
                    while (offset < 2 && isxdigit(*(epos + offset + 1))) {
                        value = hex2decimal(*(epos + offset + 1)) + 16 * value;
                        ++offset;
                    }
                    if (offset == 0) {
                        prequest->buffer.push_back('\\');
                        prequest->buffer.push_back('x');
                        break;
                    }

                    if (value == 0) {
                        ptext = epos + offset + 1;

                        srcml_archive_enable_full_archive(srcml_arch);

                        goto end;
                    }

                    prequest->buffer.push_back(value);
                    if (value == '\n')
                        ++prequest->loc;
                    epos += offset;
                    break;
                }
                // byte with octal value from 1 to 3 characters
                // Note: GNU echo documentation says that the \0 is required, but
                // the actual implementation accepts \1, \2, ..., \7
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                {
                    int value = 0;
                    int offset = *epos == '0' ? 1 : 0;
                    /* 
                        Spec 6.4.4.4 Character constants:

                        octal-escape-sequence:
                        \ octal-digit
                        \ octal-digit octal-digit
                        \ octal-digit octal-digit octal-digit

                        However, man echo (man gecho) states:

                        \0NNN  byte with octal value NNN (1 to 3 digits)

                        So, we will allow both
                    */
                    int maxlength = *epos == '0' ? 4 : 3;
                    while (offset < maxlength && isodigit(*(epos + offset))) {
                        value = (*(epos + offset) - '0') + 8 * value;
                        ++offset;
                    }
                    if (offset == 0) {
                        prequest->buffer.push_back('\\');
                        prequest->buffer.push_back('0');
                        break;
                    }

                    if (value == 0) {
                        ptext = epos + offset;

                        srcml_archive_enable_full_archive(srcml_arch);

                        goto end;
                    }

                    prequest->buffer.push_back(value);
                    if (value == '\n')
                        ++prequest->loc;
                    epos += offset - 1;
                    break;
                }
                default:
                    prequest->buffer.push_back('\\');
                    prequest->buffer.push_back(*(epos));
                }
                ptext = epos + 1;
            }

            // finished with no '\\' remaining, so flush buffer
            prequest->buffer.insert(prequest->buffer.end(), ptext, ptext + strlen(ptext));
            ptext = 0;
        }

        // schedule for parsing
        end:    count += 1;

        queue.schedule(prequest);
      }

      return count;

      }
      else
      {
          auto error = get_object_outcome.GetError();
          std::cout << "ERROR: " << error.GetExceptionName() << ": " << error.GetMessage() << std::endl;
      }

    }
    Aws::ShutdownAPI(options);

    return 1;
}

#endif