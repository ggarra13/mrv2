// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>

namespace mrv
{
    /**
     * Parse a host name like tcp://server.com:1234
     *
     * @param host Returns the host without any tcp:// prefix.
     * @param port Returns the port if it is encoded in the hostname or
     *             an empty string if not.
     */
    void parse_hostname(std::string& host, std::string& port);

    /**
     * Store the port number in the preferences.
     *
     * @param port server or client port.
     */
    void store_port(const uint16_t port);
} // namespace mrv
