from PIL import Image, ImageDraw
import json

import folder_paths
import logging
import numpy as np
import os
import time
import torch

# Function to scale points
def _scale_points(points, scale_x, scale_y):
    return [(x * scale_x, y * scale_y) for x, y in points]

# Function to draw a circle with a specific width
def _draw_circle(draw, center, radius, width, fill_color, outline_color):
    
    # Draw the outer circle
    draw.ellipse(
        (center[0] - radius, center[1] - radius, center[0] + radius, center[1] + radius),
        fill=fill_color,
        outline=outline_color,
        width=width
    )
    
class mrv2AnnotationsImageNode:
    @classmethod
    def INPUT_TYPES(s):
        input_directory = folder_paths.get_input_directory()
        files = [f for f in os.listdir(input_directory) \
                 if os.path.isfile(os.path.join(input_directory, f))]
        return {"required":
                { "annotations": (sorted(files), {"image_upload": True})},
                }

    
    RETURN_TYPES = ("IMAGE", "MASK")
    OUTPUT_TOOLTIPS = ("The decoded image and mask.",)
    
    FUNCTION = "create_mask"

    CATEGORY = "mrv2/mask"
    DESCRIPTION = "Loads a mrv2 annotation .json file and creates a mask for it."

    @classmethod
    def VALIDATE_INPUTS(s, annotations):
        if not folder_paths.exists_annotated_filepath(annotations):
            return "Invalid image file: {}".format(annotations)

        return True
    
    @classmethod
    def IS_CHANGED(s, annotations):
        return time.time()
    
    def create_mask(self, annotations):
        input_directory = folder_paths.get_input_directory()
        annotations_path = os.path.join(input_directory, annotations)

        logging.debug(f"Reading annotation {annotations_path}")
        # Load the JSON file
        with open(annotations_path) as f:
            data = json.load(f)

        # Outputs
        output_images = []
        output_masks = []
        
        # Image size
        image_width, image_height = data['render_size']

        scale_factor = 4  # Drawing on a canvas 4x larger for antialiasing
        high_res_width = image_width * scale_factor
        high_res_height = image_height * scale_factor
        
        # Create a high-resolution black image
        image = Image.new('L', (high_res_width, high_res_height), 0)
        draw = ImageDraw.Draw(image)

        # Process the paths from the JSON
        for annotation in data['annotations']:
            for shape in annotation['shapes']:
                shape_type = shape['type']
                pen_size = int(shape.get('pen_size', 1.0) * scale_factor)
                points = None
                if shape.get('pts', None):
                    points = [(point['x'], image_height - point['y'])
                              for point in shape['pts']]

                    # Scale points to the higher-resolution canvas
                    points = _scale_points(points, scale_factor, scale_factor)
            
                if shape_type == 'DrawPath' or shape_type == 'Rectangle':
                    # Draw the path (white color, width defined by 'pen_size')
                    draw.line(points, fill=255, width=int(pen_size))
                elif shape_type == 'ErasePath':
                    # Draw the path (white color, width defined by 'pen_size')
                    draw.line(points, fill=0, width=int(pen_size * 1.25))
                elif shape_type == 'Arrow':
                    left_side = [points[1], points[2]]
                    draw.line(left_side, fill=255, width=int(pen_size))
                    right_side = [points[1], points[4]]
                    draw.line(right_side, fill=255, width=int(pen_size))
                    root = [points[0], points[1]]
                    draw.line(root, fill=255, width=int(pen_size))
                elif shape_type == 'GL2Text' or shape_type == "Text":
                    continue
                elif shape_type == 'Circle' or shape_type == 'FilledCircle':
                    # Center and radius for the circle
                    center = shape['center']     # Center of the image
                    center[1] = image_height - center[1]
                    radius = int(shape['radius'] * scale_factor)  # Outer radius
                    
                    center = _scale_points([center], scale_factor, scale_factor)[0]
            
                    # Draw the circle with the specified stroke width
                    fill_color = None
                    outline_color = 255
                    if shape_type == 'FilledCircle':
                        fill_color = 255
                        outline_color = None
                    _draw_circle(draw, center, radius, pen_size, fill_color, outline_color)
                elif shape_type == 'Rectangle' or shape_type == 'FilledRectangle':
                    box = [(points[0][0], points[0][1]), (points[2][0], points[2][1])]
                    if shape_type == 'Rectangle':
                        draw.rectangle(box, fill=None, outline=255)
                    else:
                        draw.rectangle(box, fill=255, outline=None)
                elif shape_type == 'Polygon':
                    draw.polygon(points, fill=None, outline=255)
                elif shape_type == 'FilledPolygon':
                    draw.polygon(points, fill=255, outline=None)
                else:
                    logging.error(f"Unknown shape type {shape_type}")
                    
        # Downscale to the target resolution
        mask = image.resize((image_width, image_height),
                            Image.Resampling.LANCZOS)
        image = mask.convert('RGB')
        
        # Convert the PIL mask to a NumPy array
        mask = np.array(mask).astype(np.float32) / 255.0
        image = np.array(image).astype(np.float32) / 255.0
        
        # Convert the NumPy array to torch data
        image = torch.from_numpy(image)[None,]
        mask = torch.from_numpy(mask).unsqueeze(0)

        output_images.append(image)
        output_masks.append(mask)
        
        if len(output_images) > 1:
            output_image = torch.cat(output_images, dim=0)
            output_mask = torch.cat(output_masks, dim=0)
        else:
            output_image = output_images[0]
            output_mask = output_masks[0]

            
        return (output_image, output_mask)
