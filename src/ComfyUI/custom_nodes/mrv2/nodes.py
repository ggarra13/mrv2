
from .save.mrv2_save_exr_image import mrv2SaveEXRImage
from .draw.mrv2_annotations_image import mrv2AnnotationsImageNode

NODE_CLASS_MAPPINGS = {
    "mrv2SaveEXRImage": mrv2SaveEXRImage,
    "mrv2AnnotationsImageNode": mrv2AnnotationsImageNode,
}

NODE_DISPLAY_NAME_MAPPINGS = {
    "mrv2SaveEXRImage": "mrv2 Save EXR Image",
    "mrv2AnnotationsImageNode": "mrv2 Annotations Image Node",
}
