// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once


#ifdef _WIN32

int setenv(const char * name, const char * value, int overwrite);
int unsetenv(const char* name); 

#endif // _WIN32
