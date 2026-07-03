from file_transfer import AsyncTransferManager, TkProgressBridge
from file_transfer.backends.sftp import SFTPBackend, SFTPConfig

factory = lambda: SFTPBackend(SFTPConfig(host="srv", username="me", private_key_path="~/.ssh/id_rsa"))
manager = AsyncTransferManager(factory, max_workers=4)

# submit returns immediately; UI keeps running
handle = manager.submit_download("remote/video.mp4", "/local/video.mp4",
                                  resume=True, progress_callback=bridge.callback)

# later, if user clicks Cancel:
handle.cancel()
