
import glob, os, platform, subprocess

import numpy as np

from comfy.utils import ProgressBar

import logging
import folder_paths
import socket
import time


import OpenEXR
import Imath

def remove_files(pattern):
  """
  Removes all files that match the pattern

  Args:
      pattern (str)
  """
  for filename in glob.glob(pattern):
    os.remove(filename)

    
def get_mrv2_executable():
    kernel = platform.system()
    exe = None
    if kernel == 'Linux':
        username = os.getlogin()
        exe = f'/home/{username}/bin/mrv2'
        if not os.path.exists(exe):
            exe = '/usr/bin/mrv2'
        elif not os.path.exists(exe):
            exe = "mrv2"
    elif kernel == 'Windows':
        import winreg
        exe = 'C:/Program Files/mrv2/bin/mrv2.exe'
        try:
            key_path = r"Applications\mrv2.exe\shell\Open\command"
            key = winreg.OpenKey(winreg.HKEY_CLASSES_ROOT,
                                 key_path)
            value, reg_type = winreg.QueryValueEx(key, '')
            exe = f'{value[:-5]}'
            exe = exe.replace('\\', '/')
        except WindowsError as e:
            logging.error('Error retrieving value:\n',e)
        finally:
            # Always close the opened key
            winreg.CloseKey(key)

        #
        # Look for default install locations
        #
        if not os.path.exists(exe):
            exe = "mrv2.exe"
    elif kernel == 'Darwin':
        exe = '/Applications/mrv2.app/Contents/MacOS/mrv2'
    else:
        raise RuntimeError(f'Unknown platform {kernel}')

    if exe == None:
        raise RuntimeError(f'Could not find mrv2 executable')
    return exe
    
def mrv2_process(exe, fullpath):
    process = subprocess.Popen([exe, fullpath])
    
class mrv2SaveEXRImage:

    kMRV2_PORT = 55121;

    kEXR_TYPES = ['half', 'float32', 'uint32']
    
    def __init__(self):
        self.output_dir = folder_paths.get_output_directory()
        self.type = "output"
        
    
    @classmethod
    def INPUT_TYPES(s):
        return {"required": 
                    {
                        "images": ("IMAGE", ),
                        "filename_prefix": ("STRING", {"default": "mrv2"}),
                        "exr_type" : (mrv2SaveEXRImage.kEXR_TYPES, ),
                        "pixel_aspect" : ("FLOAT",
                                          {"default": 1.0,
                                           "min": 0.001,
                                           "max": 4.0,
                                           "step": 0.1
                                           }
                                          ),
                    },
                "optional":
                {
                        "masks" : ("MASK", ),
                }
                }

    RETURN_TYPES = ()
    FUNCTION = "save_images"

    OUTPUT_NODE = True

    CATEGORY = "mrv2/save"

    @classmethod
    def IS_CHANGED(s, images):
        return time.time()
    
    def save_images(self, images, masks = [],
                    filename_prefix="mrv2",
                    exr_type = 'half',
                    pixel_aspect = 1.0):
        
        filename_path = folder_paths.get_save_image_path(filename_prefix, self.output_dir, images[0].shape[1], images[0].shape[0])
        
        full_output_folder, filename, counter, subfolder, filename_prefix = filename_path
        results = list()

        mrv2_pattern = f"{filename}_*.exr"
        mrv2_pattern = os.path.join(full_output_folder, mrv2_pattern)
        remove_files(mrv2_pattern)
        
        num_images = len(images)
        num_masks  = len(masks)
        if num_masks > 0 and num_images != num_masks:
            raise ValueError("Number of images do not match number of masks")

        mrv2_fullpath = None
        pbar = ProgressBar(num_images)
        
        for (batch_number, image) in enumerate(images):
            pbar.update(1)
                
            width = image.shape[1]
            height = image.shape[0]
            
            i = image.cpu().numpy()
            m = None

            # Assuming the input image is in format HWC
            # (height, width, channels)
            channel_count = i.shape[2]
            if channel_count == 1:
                channels = ['L']
            elif channel_count == 3:
                channels = ['R', 'G', 'B']
            else:
                raise ValueError("Image does not have the correct number of channels")

            if num_masks > 0:
                channels.append('A')
                m = masks[batch_number].cpu().numpy()
                
            # Prepare channel data
            channel_map = {}
            for c, channel in enumerate(channels):

                if channel == 'A':
                    img = m
                    c == 0
                else:
                    img = i
                    
                if exr_type == 'half':
                    channel_data = img[:, :, c].astype(np.float16).tobytes()
                elif exr_type == 'float32':
                    channel_data = img[:, :, c].astype(np.float32).tobytes()
                elif exr_type == 'uint32':
                    # Ensure values are within uint32 range
                    scaled_image = np.clip(img * 65535.0, 0, 2**32 - 1).astype(np.uint32)
                    channel_data = scaled_image[:, :, c].tobytes()
                else:
                    raise ValueError(f'Invalid EXR data type: {exr_type}')
                
                channel_map[channel] = channel_data
                
            header = OpenEXR.Header(width, height)
            header['channels'] = { c:
                                   Imath.Channel(
                                       Imath.PixelType(Imath.PixelType.HALF)
                                   ) for c in channels
                                  }

            # Set the pixel aspect ratio
            header['pixelAspectRatio'] = pixel_aspect

            file = f"{filename}_{counter:05}.exr"
            fullpath = os.path.join(full_output_folder, file)

            exr_out = OpenEXR.OutputFile(fullpath, header)
            exr_out.writePixels(channel_map)
            exr_out.close()
            
            counter += 1

        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect(('localhost', self.kMRV2_PORT))
                s.sendall(fullpath.encode())
        except ConnectionRefusedError:
            # No mrv2 with pipe running.  Start an instance of mrv2.
            
            # Get the name of the mrv2 executable.
            exe = get_mrv2_executable()

            # Start mrv2 with the fullpath to the sequence.
            mrv2_process(exe, fullpath)

        return { }

    
# NOTE: names should be globally unique
NODE_CLASS_MAPPINGS = {
    "mrv2SaveEXRImage": mrv2SaveEXRImage,
}

# A dictionary that contains the friendly/humanly readable titles for the nodes
NODE_DISPLAY_NAME_MAPPINGS = {
  "mrv2SaveEXRImage": "mrv2 Save EXR Node",
}
