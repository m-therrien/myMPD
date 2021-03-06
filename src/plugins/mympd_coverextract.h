/* myMPD
   (c) 2018-2019 Juergen Mang <mail@jcgames.de>
   This project's homepage is: https://github.com/jcorporation/mympd
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef MYMPD_COVEREXTRACT_H
#define MYMPD_COVEREXTRACT_H
#define MYMPD_COVEREXTRACT_VERSION "0.2.0"
    #ifdef __cplusplus
    extern "C" {
    #endif

    extern bool coverextract(const char *media_file_ptr, const char *cache_dir_ptr, char *image_filename, const int image_filename_len, char *image_mime_type, const int image_mime_type_len, const bool extract);

    #ifdef __cplusplus
    }
    #endif
#endif
