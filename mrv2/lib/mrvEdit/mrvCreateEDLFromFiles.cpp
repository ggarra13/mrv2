
#include <tlIO/System.h>

#include <tlCore/File.h>
#include <tlCore/FileInfo.h>
#include <tlCore/Path.h>
#include <tlCore/StringFormat.h>

#include <tlTimeline/Timeline.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/externalReference.h>
#include <opentimelineio/gap.h>
#include <opentimelineio/imageSequenceReference.h>
#include <opentimelineio/timeline.h>


#include "mrvCore/mrvI8N.h"
#include "mrvCore/mrvFile.h"

#include "mrvFl/mrvIO.h"

#include "mrvEdit/mrvCreateEDLFromFiles.h"
#include "mrvEdit/mrvEditUtil.h"

#include "mrvApp/mrvApp.h"


namespace
{
    const char* kModule = "edl";
}


namespace mrv
{
    using namespace tl;
    
    using otime::RationalTime;
    using otime::TimeRange;

    using otio::Clip;
    using otio::Composition;
    using otio::Gap;
    using otio::Timeline;
    using otio::Track;

    bool createEDLFromFiles(std::string& otioFile,
                            const std::vector<std::string> files)
    {
        otio::ErrorStatus errorStatus;
        
        const timeline::Options options;
        
        otio::SerializableObject::Retainer<otio::Timeline> otioTimeline =
            new otio::Timeline("EDL");
        
        auto videoTrack =
            new otio::Track("Video", std::nullopt, otio::Track::Kind::video);
        
        auto audioTrack =
            new otio::Track("Audio", std::nullopt, otio::Track::Kind::audio);

            
        auto context = App::app->getContext();
        auto ioSystem = context->getSystem<io::System>();
            
        for (const auto& file : files)
        {
            tl::file::Path path(file);
            if (!path.isAbsolute())
            {
                path = tl::file::Path(tl::file::getCWD() + file);
            }
            
            if (mrv::file::isOTIO(path))
            {
                LOG_ERROR(_("Cannot currently concatenate an .otio file."));
                return false;
            }

            if (file::isDirectory(file))
            {
                LOG_ERROR(_("Cannot currently concatenate a directory."));
                return false;
            }
            
            if (!mrv::file::isReadable(file))
            {
                std::string err =
                    string::Format(
                        _("Filename '{0}' does not exist or does not "
                          "have read permissions."))
                        .arg(file);
                LOG_ERROR(err);
                continue;
            }
                
            // Is the input a sequence?
            const bool isSequence = mrv::file::isSequence(path);

            if (isSequence)
            {
                if (!path.isSequence())
                {
                    // Check for other files in the sequence.
                    std::vector<tl::file::FileInfo> list;
                    tl::file::ListOptions listOptions;
                    listOptions.sequenceExtensions = { path.getExtension() };
                    listOptions.maxNumberDigits = options.pathOptions.maxNumberDigits;
                    tl::file::list(path.getDirectory(), list, listOptions);
                    const auto i = std::find_if(
                        list.begin(),
                        list.end(),
                        [path](const tl::file::FileInfo& value)
                            {
                                return value.getPath().sequence(path);
                            });
                    if (i != list.end())
                    {
                        path = i->getPath();
                    }
                }
            }
            
            // Is the input a video or audio file?
            if (auto read = ioSystem->read(path, options.ioOptions))
            {
                const auto info = read->getInfo().get();

                otime::RationalTime startTime = time::invalidTime;
                otio::ErrorStatus errorStatus;

                // Read the video.
                if (!info.video.empty())
                {
                    startTime = info.videoTime.start_time();
                    auto videoClip = new otio::Clip;
                    videoClip->set_source_range(info.videoTime);
                    if (isSequence)
                    {
                        auto mediaReference = new otio::ImageSequenceReference(
                            path.getProtocol() + path.getDirectory(),
                            path.getBaseName(),
                            path.getExtension(),
                            info.videoTime.start_time().value(),
                            1,
                            info.videoTime.duration().rate(),
                            path.getPadding());
                        mediaReference->set_available_range(info.videoTime);
                        videoClip->set_media_reference(mediaReference);
                    }
                    else
                    {
                        videoClip->set_media_reference(new otio::ExternalReference(
                                                           path.get(-1, tl::file::PathType::Full),
                                                           info.videoTime));
                    }
                    videoTrack->append_child(videoClip, &errorStatus);
                    if (otio::is_error(errorStatus))
                    {
                        throw std::runtime_error("Cannot append video child");
                    }
                }

                if (info.audio.isValid())
                {
                    if (startTime.is_invalid_time())
                    {
                        startTime = info.audioTime.start_time();
                    }

                    auto audioClip = new otio::Clip;
                    audioClip->set_source_range(info.audioTime);
                    audioClip->set_media_reference(new otio::ExternalReference(
                                                       path.get(-1, tl::file::PathType::Full),
                                                       info.audioTime));
                    audioTrack->append_child(audioClip, &errorStatus);
                    if (otio::is_error(errorStatus))
                    {
                        throw std::runtime_error("Cannot append audio child");
                    }
                }
                else
                {
                    auto audioGap = new otio::Gap;
                    audioGap->set_source_range(info.videoTime);
                    audioTrack->append_child(audioGap, &errorStatus);
                    if (otio::is_error(errorStatus))
                    {
                        throw std::runtime_error("Cannot append audio gap child");
                    }
                }
            }
        }
        
        auto stack = new otio::Stack;

        stack->append_child(videoTrack, &errorStatus);
        if (otio::is_error(errorStatus))
        {
            std::string error =
                string::Format(_("Could not append video track {0}"))
                .arg(errorStatus.full_description);
            throw std::runtime_error(error);
        }
        
        stack->append_child(audioTrack, &errorStatus);
        if (otio::is_error(errorStatus))
        {
            std::string error =
                string::Format(_("Could not append audio track {0}"))
                .arg(errorStatus.full_description);
            throw std::runtime_error(error);
        }

        otioTimeline->set_tracks(stack);

        otioFile = otioFilename(App::ui);

        otioTimeline->to_json_file(otioFile, &errorStatus);
        if (otio::is_error(errorStatus))
        {
            std::string error =
                string::Format(_("Could not save .otio file: {0}"))
                .arg(errorStatus.full_description);
            throw std::runtime_error(error);
        }
        
        return true;
    }


}
