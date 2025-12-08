/*****************************************************************************
 * Copyright 2024 Serge Poltavski. All rights reserved.
 *
 * This file may be distributed under the terms of GNU Public License version
 * 3 (GPL v3) as defined by the Free Software Foundation (FSF). A copy of the
 * license should have been included with this file, or the project in which
 * this file belongs to. You may also find the details of GPL v3 at:
 * http://www.gnu.org/licenses/gpl-3.0.txt
 *
 * If you have any questions regarding the use of this file, feel free to
 * contact the author of this file, or the owner of the project in which
 * this file belongs to.
 *****************************************************************************/
#ifndef APP_VERSION_H
#define APP_VERSION_H

namespace ceam {

const char* app_version();
int app_version_major();
int app_version_minor();
int app_version_patch();
const char* app_git_version();

int app_file_format_version();

} // namespace ceam

#endif // APP_VERSION_H
