from globals import libsrcml
from ctypes import c_int, c_void_p, c_char_p, pointer
from exception import *

# int srcml_parse_unit_filename(struct srcml_unit* unit, const char* src_filename);
libsrcml.srcml_parse_unit_filename.restype = c_int
libsrcml.srcml_parse_unit_filename.argtypes = [c_void_p, c_char_p]

# int srcml_parse_unit_memory  (struct srcml_unit*, const char* src_buffer, size_t buffer_size);
libsrcml.srcml_parse_unit_memory.restype = c_int
libsrcml.srcml_parse_unit_memory.argtypes = [c_void_p, c_char_p, c_int]

# int srcml_unparse_unit_filename(struct srcml_unit*, const char* src_filename);
libsrcml.srcml_unparse_unit_filename.restype = c_int
libsrcml.srcml_unparse_unit_filename.argtypes = [c_void_p, c_char_p]

# int srcml_unparse_unit_memory  (struct srcml_unit*, char** src_buffer, int * src_size);
libsrcml.srcml_unparse_unit_memory.restype = c_int
libsrcml.srcml_unparse_unit_memory.argtypes = [c_void_p, c_void_p, c_void_p]

# struct srcml_unit* srcml_create_unit(struct srcml_archive* archive);
libsrcml.srcml_create_unit.restype = c_void_p
libsrcml.srcml_create_unit.argtypes = [c_void_p]

# int srcml_free_unit(struct srcml_unit*);
libsrcml.srcml_free_unit.restype = None
libsrcml.srcml_free_unit.argtypes = [c_void_p]

# int srcml_unit_set_language (struct srcml_unit*, const char* language);
libsrcml.srcml_unit_set_language.restype = c_int
libsrcml.srcml_unit_set_language.argtypes = [c_void_p, c_char_p]

# int srcml_unit_set_filename (struct srcml_unit*, const char* filename);
libsrcml.srcml_unit_set_filename.restype = c_int
libsrcml.srcml_unit_set_filename.argtypes = [c_void_p, c_char_p]

# int srcml_unit_set_directory(struct srcml_unit*, const char* directory);
libsrcml.srcml_unit_set_directory.restype = c_int
libsrcml.srcml_unit_set_directory.argtypes = [c_void_p, c_char_p]

# int srcml_unit_set_version  (struct srcml_unit*, const char* version);
libsrcml.srcml_unit_set_version.restype = c_int
libsrcml.srcml_unit_set_version.argtypes = [c_void_p, c_char_p]

# const char* srcml_unit_get_language (const struct srcml_unit*);
libsrcml.srcml_unit_get_language.restype = c_char_p
libsrcml.srcml_unit_get_language.argtypes = [c_void_p]

# const char* srcml_unit_get_filename (const struct srcml_unit*);
libsrcml.srcml_unit_get_filename.restype = c_char_p
libsrcml.srcml_unit_get_filename.argtypes = [c_void_p]

# const char* srcml_unit_get_directory(const struct srcml_unit*);
libsrcml.srcml_unit_get_directory.restype = c_char_p
libsrcml.srcml_unit_get_directory.argtypes = [c_void_p]

# const char* srcml_unit_get_version  (const struct srcml_unit*);
libsrcml.srcml_unit_get_version.restype = c_char_p
libsrcml.srcml_unit_get_version.argtypes = [c_void_p]

# const char* srcml_unit_get_xml      (const struct srcml_unit*);
libsrcml.srcml_unit_get_xml.restype = c_char_p
libsrcml.srcml_unit_get_xml.argtypes = [c_void_p]

# srcml_unit wrapper
class srcml_unit :

    def __init__(self, archive, unit = 0) :
        self.unit = unit
        if self.unit == 0 :
            self.unit = libsrcml.srcml_create_unit(archive.archive)

    def parse_filename(self, src_filename) :
        libsrcml.srcml_parse_unit_filename(self.unit, src_filename)

    def parse_memory(self, src_buffer) :
        libsrcml.srcml_parse_unit_memory(self.unit, src_buffer, len(src_buffer))

    def unparse_filename(self, src_filename) :
        libsrcml.srcml_parse_unit_filename(self.unit, src_filename)

    def unparse_memory(self) :
        self.src_size = c_int()
        self.src_buffer = c_char_p()
        libsrcml.srcml_unparse_unit_memory(self.unit, pointer(self.src_buffer), pointer(self.src_size))

    def set_language(self, language) :
        libsrcml.srcml_unit_set_language(self.unit, language)

    def set_filename(self, filename) :
        libsrcml.srcml_unit_set_filename(self.unit, filename)

    def set_directory(self, directory) :
        libsrcml.srcml_unit_set_directory(self.unit, directory)

    def set_version(self, version) :
        libsrcml.srcml_unit_set_version(self.unit, version)

    def get_language(self) :
        return libsrcml.srcml_unit_get_language(self.unit)

    def get_filename(self) :
        return libsrcml.srcml_unit_get_filename(self.unit)

    def get_directory(self) :
        return libsrcml.srcml_unit_get_directory(self.unit)

    def get_version(self) :
        return libsrcml.srcml_unit_get_version(self.unit)

    def get_xml(self) :
        return libsrcml.srcml_unit_get_xml(self.unit)

    def src(self) :
        return self.src_buffer.value

    def __del__(self) :
        libsrcml.srcml_free_unit(self.unit)

