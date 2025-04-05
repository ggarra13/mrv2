// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#ifdef _WIN32

int setenv(const wchar_t* name, const wchar_t* value, int overwrite);
int unsetenv(const wchar_t* name);

#endif // _WIN32
