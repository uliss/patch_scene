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
#include "app_version.h"
#include "patch_scene_version.h"

namespace ceam {

const char* app_version()
{
    return PATCH_SCENE_VERSION;
}

const char* app_git_version()
{
    return PATCH_SCENE_GIT_VERSION;
}

int app_version_major()
{
    return PATCH_SCENE_VERSION_MAJOR;
}

int app_version_minor()
{
    return PATCH_SCENE_VERSION_MINOR;
}

int app_version_patch()
{
    return PATCH_SCENE_VERSION_PATCH;
}

int app_file_format_version()
{
    return PATCH_SCENE_FILE_FORMAT_VERSION;
}

}
