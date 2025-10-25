// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvCore/mrvBackend.h"

#ifdef OPENGL_BACKEND
#include "mrvGL/mrvGLWindow.h"
#endif

#ifdef VULKAN_BACKEND
#include "mrvVk/mrvVkWindow.h"
#include "mrvVk/mrvVkShape.h"
#endif

#include "mrvVoice/mrvVoiceOver.h"

#include "mrvOptions/mrvStereo3DOptions.h"
#include "mrvOptions/mrvEnvironmentMapOptions.h"

#include "mrvCore/mrvActionMode.h"
#include "mrvCore/mrvColorAreaInfo.h"
#include "mrvCore/mrvString.h"

#include <tlTimeline/BackgroundOptions.h>
#include <tlTimeline/IRender.h>
#include <tlTimeline/PlayerOptions.h>

#include <tlDraw/Shape.h>

#include <tlCore/ValueObserver.h>


class ViewerUI;

namespace mrv
{
    using namespace tl;

    class TimelinePlayer;

    class LaserFadeData;

    class MultilineInput;

    class TimelinePlayer;

    namespace voice
    {
        class VoiceOver;
    }
    
    namespace BACKEND_NAMESPACE
    {

        class TimelineViewport : public BACKEND_SUPER_CLASS
        {
            TLRENDER_NON_COPYABLE(TimelineViewport);

        public:
            TimelineViewport(int X, int Y, int W, int H, const char* L);
            TimelineViewport(int W, int H, const char* L);
            ~TimelineViewport();

            //! Virtual FLTK methods.
            int handle(int event) override;
            void resize(int X, int Y, int W, int H) override;

            //! Store main ui.  We need this as the first viewport does not
            //! yet know about App::ui.
            void main(ViewerUI* m) noexcept;

            //! Handle scrubbing.
            void scrub(float multiplier = 1.F) noexcept;

            //! Stop playback while scrubbing check for audio
            void stopPlaybackWhileScrubbing() noexcept;

            //! Undo last shape and annotations if no more shapes.
            void undo();

            //! Redo last shape.
            void redo();

            //! Change cursor to another.
            void set_cursor(Fl_Cursor x) const noexcept;

            //! Set the action mode.
            void setActionMode(const ActionMode& mode) noexcept;

            //! Get the action mode.
            ActionMode getActionMode() noexcept;

            //! Get the color information of the selected area
            const area::Info& getColorAreaInfo() noexcept;

            //! Return the current video image in BGRA order after drawing it.
            const image::Color4f* image() const;

            //! Get the background options.
            const timeline::BackgroundOptions&
            getBackgroundOptions() const noexcept;

            //! Observe the background options.
            std::shared_ptr<observer::IValue<timeline::BackgroundOptions> >
            observeBackgroundOptions() const;

            //! Set the background options.
            void setBackgroundOptions(const timeline::BackgroundOptions& value);

            //! Set the OCIO options.
            void setOCIOOptions(const timeline::OCIOOptions&) noexcept;

            //! Set the OCIO options for monitor.
            void setOCIOOptions(
                unsigned monitorId, const timeline::OCIOOptions&) noexcept;

            const timeline::OCIOOptions&
            getOCIOOptions(unsigned monitorId) const noexcept;

            const timeline::OCIOOptions& getOCIOOptions() const noexcept;

            //! Set the LUT options.
            void setLUTOptions(const timeline::LUTOptions&) noexcept;

            timeline::LUTOptions& lutOptions() noexcept;

            //! Set the image options.
            void setImageOptions(
                const std::vector<timeline::ImageOptions>&) noexcept;

            //! Set the display options.
            void setDisplayOptions(
                const std::vector<timeline::DisplayOptions>&) noexcept;

            //! Set the comparison options.
            void setCompareOptions(const timeline::CompareOptions&) noexcept;

            //! Set the stereo 3D options.
            void setStereo3DOptions(const Stereo3DOptions&) noexcept;

            //! Set HDR options.
            void setHDROptions(const timeline::HDROptions&) noexcept;

            const timeline::HDROptions& getHDROptions() const noexcept;

            //! Set the timeline players.
            void setTimelinePlayer(TimelinePlayer*) noexcept;

            //! Get one of the timeline players.  Index is not checked.
            mrv::TimelinePlayer* getTimelinePlayer() const noexcept;

            //! Return if safe areas are active.
            bool getSafeAreas() const noexcept;

            //! Return if data window is active.
            bool getDataWindow() const noexcept;

            //! Return if display window is active.
            bool getDisplayWindow() const noexcept;

            //! Return if ignoring display window is active.
            bool getIgnoreDisplayWindow() const noexcept;

            //! Get pixel aspect ratio of image.
            float getPixelAspectRatio() const noexcept;

            //! Set the crop mask.
            void setSafeAreas(bool) noexcept;

            //! Set data window.
            void setDataWindow(bool) noexcept;

            //! Set display window.
            void setDisplayWindow(bool) noexcept;

            //! Set ignore of display window.
            void setIgnoreDisplayWindow(bool) noexcept;

            //! Set pixel aspect ratio of image.
            void setPixelAspectRatio(const float x) noexcept;

            //! Clear the help text after 1 second has elapsed.
            void clearHelpText();

            //! Set help HUD text
            void setHelpText(const std::string&);

            //! @{ HUD controls

            bool getHudActive() const;
            void setHudActive(const bool active);
            void setHudDisplay(const HudDisplay value);
            void toggleHudDisplay(const HudDisplay value);

            //! }@

            HudDisplay getHudDisplay() const noexcept;

            //! Return the crop mask
            float getMask() const noexcept;

            //! Set the crop mask
            void setMask(float f) noexcept;

            //! Get the view position.
            const math::Vector2i& viewPos() const noexcept;

            //! Get the view zoom.
            float viewZoom() const noexcept;

            //! Get whether the view is framed.
            bool hasFrameView() const noexcept;

            //! Set the view position and zoom.
            void setViewPosAndZoom(const tl::math::Vector2i&, float) noexcept;

            //! Set the view zoom.
            void setViewZoom(
                float, const math::Vector2i& focus = math::Vector2i()) noexcept;

            //! Get the environment map options (should not return a reference)
            EnvironmentMapOptions getEnvironmentMapOptions() const noexcept;

            //! Set the environment map options
            void
            setEnvironmentMapOptions(const EnvironmentMapOptions& o) noexcept;

            //! Resize the window to screen
            void resizeWindow() noexcept;

            //! Set auto resizing of the window.
            void setResizeWindow(bool active) noexcept;

            //! Set auto frame the view.
            void setFrameView(bool active) noexcept;

            //! Frame the view.
            void frameView() noexcept;

            //! Center the view without changing the zoom.
            void centerView() noexcept;

            //! Set the view zoom to 1:1.
            void viewZoom1To1() noexcept;

            //! Playback controls
            void startFrame() noexcept;

            void framePrev() noexcept;

            void playBackwards() noexcept;

            void stop() noexcept;

            void frameNext() noexcept;

            void playForwards() noexcept;

            void togglePlayback() noexcept;

            void endFrame() noexcept;

            void setPlayback(const timeline::Playback) noexcept;

            //! Set the Annotation previous ghost frames.
            void setGhostPrevious(int);

            //! Set the Annotation previous ghost frames.
            void setGhostNext(int);

            //! Set the missing frame type.
            void setMissingFrameType(const MissingFrameType);

            //
            const std::vector<tl::timeline::VideoData>&
            getVideoData() const noexcept;

            // Callbacks
            int acceptMultilineInput() noexcept;

            void cacheChangedCallback() const noexcept;

            void currentTimeChanged(const otime::RationalTime&) const noexcept;

            void currentVideoCallback(
                const std::vector<tl::timeline::VideoData>&) noexcept;

            //! Set the OCIO configuration from the GUI.
            void updateOCIOOptions() noexcept;

            //! Update the display options from the GUI.
            void updateDisplayOptions() noexcept;

            //! Update the video layer from the GUI.
            void updateVideoLayers(int idx = 0) noexcept;

            //! This signal is emitted when the position and zoom change.
            void
            viewPosAndZoomChanged(const tl::math::Vector2i&, float) noexcept;

            //! This signal is emitted when the view is framed.
            void frameViewActivated() noexcept;

            //! Set or unset the window to full screen and hide/show all bars.
            void setPresentationMode(bool active = true) noexcept;

            //! Set or unset the window in maximized state.
            void setMaximized() noexcept;

            //! Get the window to full screen and hide/show all bars.
            bool getPresentationMode() const noexcept;

            //! Retrieve the full sceen mode.
            bool getFullScreenMode() const noexcept;

            //! Set or unset the window to full screen but don't hide any bars.
            void setFullScreenMode(bool active = true) noexcept;

            //! Handle a drag and drop of files to load
            void dragAndDrop(const std::string& text) noexcept;

            //! Update the pixel bar's coordinates and color information.
            void updatePixelBar() noexcept;

#ifdef OPENGL_BACKEND
            //! Get the text widget if available.
            MultilineInput* getMultilineInput() const noexcept;
#endif

#ifdef VULKAN_BACKEND
            //! Get the text widget if available.
            std::shared_ptr<VKTextShape> getMultilineInput() const noexcept;
#endif

            //! Get the viewportSize
            math::Size2i getViewportSize() const noexcept;

            //! Get the render image size
            math::Size2i getRenderSize() const noexcept;

            //! Redraw both the primary and secondary windows.
            void redrawWindows() const;

            //! Refresh both the primary and secondary windows by clearing the
            //! associated resources.
            void refreshWindows();

            //! Refresh window by clearing the associated resources.
            virtual void refresh(){};
            
            //! Handle view spinning when in Environment Map mode.
            void handleViewSpinning() noexcept;

            //! Set selection area.
            void setSelectionArea(const math::Box2i& area) noexcept;

            //! Get show annotations toggle value.
            bool getShowAnnotations() const noexcept;

            //! Show annotations toggle
            void setShowAnnotations(const bool value) noexcept;

            //! Set whether to render the video or not (annotations are still
            //! rendered).
            void setShowVideo(bool value) noexcept;

            //! Laser fading annotation
            static void laserFade_cb(LaserFadeData*);
            
            //! Laser fading annotation
            void laserFade(LaserFadeData*);

            //! Update the undo/redo buttons to be active or not.
            void updateUndoRedoButtons() const noexcept;

            //! Edit a text shape.
            void editText(
                const std::shared_ptr< draw::Shape >&,
                const int index) noexcept;

            //! Update the playback buttons.
            void updatePlaybackButtons() noexcept;

            //! Return the image rotation.
            float getRotation() const noexcept;

            //! Set the image rotation.
            void setRotation(float) noexcept;

            //! Update the coordinates.
            void updateCoords() const noexcept;

            //! Show an image.
            void showImage(const std::shared_ptr<image::Image>& image);

            //! Get current frame/video tags
            image::Tags getTags() const noexcept;

            //! Record the mouse position callback.
            void recordMousePosition();

            //! Play the mouse position callback.
            void playMousePosition();

            //! Delete the selected URL link.
            void linkDelete();
            
            //! Delete the voice over.
            void voiceOverDelete();

            //! Clear the recording (audio and mouse positions) of a voice over.
            void voiceOverClear();

            //! Append audio and mouse movements to current voice over.
            void voiceOverAppend();
            
        protected:
            void _init();

            void _updateDevices() const noexcept;

            virtual void _readPixel(image::Color4f& rgba) = 0;
            math::Vector2i _getViewportCenter() const noexcept;

            math::Vector2i _getFocus(int X, int Y) const noexcept;
            math::Vector2i _getFocus() const noexcept;
            math::Vector2i _getRaster() const noexcept;

            math::Vector2f _getFocusf(int X, int Y) const noexcept;
            math::Vector2f _getFocusf() const noexcept;
            math::Vector2f _getRasterf(int X, int Y) const noexcept;
            math::Vector2f _getRasterf() const noexcept;

            //! Get the normalized rotation between 0 and 360
            //! full rotation of the image (user rotation + video rotation)
            float _getRotation() const noexcept;

            //! Get the raster matrix.
            math::Matrix4x4f _rasterProjectionMatrix() const noexcept;
            
            //! Get the render projection matrix.
            math::Matrix4x4f _renderProjectionMatrix() const noexcept;

            //! Get the full projection matrix.
            math::Matrix4x4f _projectionMatrix() const noexcept;

            //! Get the matrix to pixel (raster) coordinates of image.
            math::Matrix4x4f _pixelMatrix() const noexcept;
            
            //! Get the matrix to pixel (raster) coordinates of image with
            //! panning and zooming.
            math::Matrix4x4f _pixelMatrixWithTransforms() const noexcept;

            //! Clip the selection area position taking into account
            //! rotation.
            void _clipSelectionArea(math::Vector2i& pos) const noexcept;

            //! Call redraw and a flush to force a redraw.
            void _refresh() noexcept;

            //! Get the annotation pen size taking renderSize into account.
            float _getPenSize() const noexcept;

            virtual void _pushAnnotationShape(const std::string& cmd) const = 0;

            void _redrawUndoRedoButtons() const;

            void _createAnnotationShape(const bool laser) const;
            void _updateAnnotationShape() const;
            void _addAnnotationShapePoint() const;
            void _endAnnotationShape() const;

            bool _isEnvironmentMap() const noexcept;
            void _updateZoom() const noexcept;

            void _showPixelBar() const noexcept;
            void _hidePixelBar() const noexcept;
            void _togglePixelBar() const noexcept;

            void _updatePixelBar() noexcept;
            void _updatePixelBar(image::Color4f& rgba) const noexcept;
            bool _shouldUpdatePixelBar() const noexcept;
            bool _isPlaybackStopped() const noexcept;
            bool _isSingleFrame() const noexcept;

            void _setVideoRotation(float value) noexcept;
            void _frameView() noexcept;
            void _handleCompareWipe() noexcept;
            void _handleCompareOverlay() noexcept;

            void _handlePushLeftMouseButton() noexcept;
            void _handlePushLeftMouseButtonShapes() noexcept;
            int  _handleReleaseLeftMouseButtonShapes() noexcept;
            
            void _handleDragLeftMouseButton() noexcept;
            void _handleDragLeftMouseButtonShapes() noexcept;
            void _handleDragSelection() noexcept;
            
            void _handleDragMiddleMouseButton() noexcept;

            void _updateCursor() const noexcept;

            void _updateViewRotation(const math::Vector2f& spin) noexcept;

            void
            _updateDisplayOptions(const timeline::DisplayOptions& d) noexcept;

            void _updateMonitorDisplayView(
                const int screen,
                const timeline::OCIOOptions& o) const noexcept;

            void _pushColorMessage(const std::string& command, float value);

            void _mallocBuffer() const noexcept;
            void _mapBuffer() const noexcept;
            void _unmapBuffer() const noexcept;

            void _setFullScreen(bool active) noexcept;

            void _getPixelValue(
                image::Color4f& rgba,
                const std::shared_ptr<image::Image>& image,
                const math::Vector2i& pos) const noexcept;
            void _calculateColorAreaRawValues(area::Info& info) const noexcept;

            void hsv_to_info(
                const image::Color4f& hsv, area::Info& info) const noexcept;
            image::Color4f rgba_to_hsv(
                int hsv_colorspace, image::Color4f& rgba) const noexcept;

            void _scrub(float change) noexcept;

            bool _hasSecondaryViewport() const noexcept;

            float _getZoomSpeedValue() const noexcept;

            void _getTags() noexcept;

            void _startVoiceRecording(const std::shared_ptr<voice::VoiceOver> voice);
            void _startVoicePlaying(const std::shared_ptr<voice::VoiceOver> voice);
            void _stopVoiceRecording(const std::shared_ptr<voice::VoiceOver> voice);
            void _stopVoicePlaying(const std::shared_ptr<voice::VoiceOver> voice);

            void _stopVoiceRecording();
            void _stopVoicePlaying();
            
            voice::MouseData currentMouseData;
            std::shared_ptr<voice::VoiceOver> currentVoiceOver;
            std::shared_ptr<tl::draw::Shape>  currentLink;
            
            TLRENDER_PRIVATE();
        };

    } // namespace vulkan

} // namespace mrv

#ifdef OPENGL_BACKEND
#  include "mrvGL/mrvTimelineViewportPrivate.h"
#endif

#ifdef VULKAN_BACKEND
#  include "mrvVk/mrvTimelineViewportPrivate.h"
#endif
