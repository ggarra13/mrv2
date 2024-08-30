

namespace mrv
{

    //! Create a temporary OTIO timeline (EDL) on tmp dir from a list of
    //! files.
    bool createEDLFromFiles(std::string& otioFile,
                            const std::vector<std::string> files);

}
