##
# @file rpm.cmake
#
# @copyright Copyright (C) 2013-2014 srcML, LLC. (www.srcML.org)
# 
# The srcML Toolkit is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# The srcML Toolkit is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with the srcML Toolkit; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

# set RPM package name
set(CPACK_RPM_PACKAGE_NAME "srcML")

# set architecture
set(CPACK_RPM_PACKAGE_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})

# package release number
set(CPACK_RPM_PACKAGE_RELEASE 1)

# set what it requires  These need to be updated or set for different operating systems
#set(CPACK_RPM_PACKAGE_REQUIRES "libxml2 >= 2.7.8, libxslt >= 1.1.26, libarchive >= 3.0.3")

# set what package provides
#set(CPACK_RPM_PACKAGE_PROVIDES "")

# set post install script
set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/post.sh")

# set install for manpage (Run cpack -G RPM)

if (OS_NAME STREQUAL "Fedora" OR OS_NAME STREQUAL "CentOS")
    set (CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION /usr/local /usr/local/share /usr/local/share/man /usr/local/share/man/man1)
    install(FILES "${CMAKE_SOURCE_DIR}/doc/manpage/srcml.1" DESTINATION "local/share/man/man1")
elseif (OS_NAME STREQUAL "openSUSE" OR OS_NAME STREQUAL "openSUSE Leap")
    set (CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION /usr/local /usr/local/man /usr/local/man/man1)
    install(FILES "${CMAKE_SOURCE_DIR}/doc/manpage/srcml.1" DESTINATION "local/man/man1")
endif()


