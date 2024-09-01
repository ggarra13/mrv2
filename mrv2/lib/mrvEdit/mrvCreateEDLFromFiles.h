// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.



namespace mrv
{

    //! Create a temporary OTIO timeline (EDL) on tmp dir from a list of
    //! files.
    bool createEDLFromFiles(std::string& otioFile,
                            const std::vector<std::string> files);

}
