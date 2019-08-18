/*
 * Copyright (C) 1999 - 2019, Hermann Schinagl, hermann@schinagl.priv.at
 */

// Provide all neccessesary files to compile hardlink.lib

#include "hardlink_types.h"
#include "DbgHelpers.h"

#include <ntsysteminfo.h>

#include "LSESettings.h"

#include "AsyncContext.h"
#include "mmfobject.h"
#include "Progressbar.h"
#include "hardlinks.h"
#include "HardlinkUtils.h"
#include "NtBase.h"


#include "UACReuse.h"

#include "multilang.h"
