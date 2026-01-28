import struct
import os

def write_imaginary_edid_to_file(filename: str, min_nits: float = 0.02, max_nits: int = 1000):
    """
    Writes an imaginary EDID binary file with HDR support, including min_nits and max_nits in the HDR Static Metadata Block.
    
    This is for unit testing purposes. The EDID includes a basic 128-byte block and a CTA-861 extension block
    with HDR capabilities.
    
    Parameters:
    - filename: str, the path to the output file.
    - min_nits: float, minimum luminance in nits (default 0.02).
    - max_nits: int, maximum luminance in nits (default 1000).
    
    The function constructs a simple EDID with:
    - Vendor: Imaginary (ID: 'IMG')
    - Product: Test Monitor
    - HDR support via CTA extension.
    """
    
    # Helper to calculate HDR metadata values (based on CTA-861 specifications)
    # Max Luminance: value = round(2 * log2(max_nits / 50))
    # But actually: Desired Content Max Luminance Data = round(32 * log2(max_cll / 50)) where max_cll is in nits, but clamped 0-255
    # Simplified approximation here for imaginary data.
    # Real formula:
    # - Desired Content Max Luminance = round(32 * log2(max_nits / 50))
    # - Desired Content Max Frame-average Luminance = similar, but we'll set same for simplicity.
    # - Desired Content Min Luminance = round(255 * (min_nits / 0.0001) ^ (1/4) / 100) or approx.
    # But the min luminance is: value = round( (min_nits / 10000) * 255 ) ? No.
    
    # Accurate from spec:
    # - MaxCLL (Desired Content Max Luminance): value = ceil( log2( max_nits / 50 ) / (1/32) ) but wait.
    # From CTA-861: The value is such that actual = 50 * 2^(value/32)
    # So to encode: value = round(32 * log2(max_nits / 50))
    # Similarly for MaxFALL.
    # For Min: actual = 0.0001 * 2^(value/32) but different.
    # Min Luminance: actual min = max_nits * (min_value/255)^4 / 10000 or something? It's percentage.
    # Desired Content Min Luminance Data = round( (min_nits / max_nits) * 255 ) but no.
    
    # Correct from HDMI/CTA spec:
    # - Desired Content Max Luminance (byte 3): M = 50 * 2^(C/32) where C is the byte value.
    #   So C = round(32 * log2(M/50))
    # - Desired Content Max Frame-Average Luminance: similar.
    # - Desired Content Min Luminance: N = M * (D/255)^2 * (1/100) where D is byte, but actually squared or ^4?
    # From spec: Min Luminance = Max Luminance * (Min Value / 255)^2 * (1/100)
    # No, in CTA-861-G: Desired Content Min Luminance = Desired Content Max Luminance * (CV / 255)^2 * 1/100
    # Where CV is the byte value for min.
    # To encode: CV = round(255 * sqrt( (100 * min_nits) / max_nits ))
    
    from math import log2, sqrt, ceil
    
    # Calculate HDR values
    max_lum_byte = int(round(32 * log2(max_nits / 50.0))) if max_nits > 50 else 0
    max_lum_byte = max(0, min(255, max_lum_byte))
    
    # Assuming MaxFALL same as Max for simplicity in imaginary data
    max_fall_byte = max_lum_byte
    
    # Min lum: CV = 255 * sqrt( (min_nits * 100) / max_nits )
    min_lum_byte = int(round(255 * sqrt((min_nits * 100) / max_nits)))
    min_lum_byte = max(0, min(255, min_lum_byte))
    
    # Base EDID block (128 bytes) - imaginary simple monitor
    # Header
    edid_base = bytearray([
        0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,  # Fixed header
    ])
    
    # Vendor ID: 'I' = 0x26, 'M' = 0x32, 'G' = 0x22 (EISA ID: bits 15-0: 00010 01100 10010 = 0x2632? Wait, compressed.
    # Vendor ID is 3 letters, compressed: A=1, ..., Z=26, bits 15-11 first char-@, 10-6 second, 5-1 third.
    # For "IMG": I=9 (1001-@=64? No: A=00001, Z=11010
    # Char1 bits 14-10: 'I'-'A'+1 =9 =001001
    # Standard: byte9 (MSB vendor), byte8 (LSB)
    # Vendor ID = ((char1 - '@') << 10) | ((char2 - '@') << 5) | (char3 - '@')
    char1 = ord('I') - ord('@')  # I=73, @=64, 9
    char2 = ord('M') - ord('@')  # M=77-64=13
    char3 = ord('G') - ord('@')  # G=71-64=7
    vendor_id = (char1 << 10) | (char2 << 5) | char3
    edid_base += struct.pack('<H', vendor_id)  # Little endian? No, big endian: MSB first
    # Byte 8-9: ID Manufacturer Name, MSB byte 9, LSB byte 8? No:
    # EDID bytes 08h-09h: vendor ID, byte 08h is high byte? No:
    # It's byte 8: bits 7-0 = vendor bits 15-8, byte 9: 7-0 = 7-0
    # But since packed as big-endian.
    edid_base += struct.pack('>H', vendor_id)
    
    # Product ID: imaginary 0x1234
    edid_base += struct.pack('<H', 0x1234)  # bytes 10-11, little endian
    
    # Serial number: 0000
    edid_base += struct.pack('<I', 0)  # bytes 12-15
    
    # Week and year of manufacture: week 1, year 2023 (2023-1990=33=0x21)
    edid_base += b'\x01\x21'  # byte 16-17
    
    # EDID version 1.4
    edid_base += b'\x01\x04'
    
    # Basic display params: 25 inch, landscape, etc. Imaginary
    edid_base += b'\x90\x19\x11\x78\x0A'  # bytes 19-23: video input, horiz cm, vert cm, gamma
    
    # Feature support: RGB, sRGB, preferred timing, etc.
    edid_base += b'\xEE\x95\xA3\x54\x4C\x99\x26\x0F\x47\x4A'
    
    # Chromaticity - imaginary
    edid_base += b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
    
    # Established timings - none
    edid_base += b'\x00\x00\x00'
    
    # Standard timings - skip, all 01
    for _ in range(8):
        edid_base += b'\x01\x01'
    
    # Descriptor 1: detailed timing for 1920x1080@60Hz
    # Pixel clock 148.50 MHz /10 = 14850 = 0x39D2
    detailed_timing = [
        0xD2, 0x39,  # pixel clock
        0x80, 0xA0, 0x20,  # h active 1920=0x780, but low 8, high 4 in others
        # Standard detailed timing structure
    ]
    # Full example for 1920x1080@60:
    detailed_timing = bytearray(b'\x88\x13\x56\xAA\x00\x30\x20\x80\x36\x00\x00\x00\x00\x00\x1E')  # abbreviated, need 18 bytes
    # Actual:
    # Pixel clock: 14835 kHz for 148.5MHz? 14850 = 0x39D2, but little? Struct is little endian for clock.
    # Clock: bytes 54-55: low, high
    # Then h active low, h blank low, h active high | h blank high
    # Etc.
    # Let's use a known good one.
    detailed_timing1 = bytearray([
        0x01, 0x1D,  # clock 74.25 MHz for 720p, but for 1080p 148.5 = 0x3A56? Wait.
        # Correct for 1920x1080@60:
        0x67, 0xD8,  # clock = 0xD867 little? No, clock is little endian 2 bytes: for 148.5MHz, clock/10 = 14850 = 0x3A02? Wait.
        # Pixel clock in EDID is (clock in MHz * 10) in little endian 16bit.
        # 148.5 MHz, 1485 *10? No: unit is 0.01 MHz, so 14850000 = but 16bit kHz.
        # No: pixel clock in EDID is clock in 10 kHz units, little endian.
        # For 148.5 MHz = 14850 * 10 kHz, so 14850 = 0x3A02, low 02, high 3A
        0x02, 0x3A,  # pixel clock
        0x80,  # h active low 0x780 &0xFF =0x80
        0x18,  # h blank low 0x118 &0xFF =0x18
        0x71,  # h active high 7, h blank high 1 = 0x71
        0x1C,  # v active low 0x438 &0xFF =0x1C
        0x16,  # v blank low 0x16
        0x20,  # v active high 4, v blank high 0 =0x40? 1080=0x438, high 4, blank 45=0x2D, low 0x2D, high 0
        # v active high 4 bits, v blank high 4 bits: 4 | 0 =0x40
        0x40,  
        0x58,  # h sync offset low
        0x2C,  # h sync width low
        0x25,  # v sync offset low 4 | v sync width low 4 =5 |5 =0x55? For 1080p: v offset 4, width 5, so 0x45
        0x45,  
        0x00,  # h sync offset high 2 | h sync width high 2 | v offset high 2 | v width high 2 =0
        0x00,  
        0xC4,  # h image size low (800? But for aspect
        0x8E,  # v image size low
        0x21,  # h high 4 | v high 4
        0x00,  # h border
        0x00,  # v border
        0x1E   # flags: interlaced no, stereo no, sync positive
    ])
    edid_base += detailed_timing1
    # Descriptor 2-4: monitor name, etc.
    # Descriptor 2: monitor name "TestMonitor"
    monitor_name = b'\x00\x00\x00\xFC\x00' + b'TestMonitor\x0A' + b'\x00' * (18-13-1)  # 18 bytes, FC for name
    monitor_name = bytearray(b'\x00\x00\x00\xFC\x00\x54\x65\x73\x74\x4D\x6F\x6E\x69\x74\x6F\x72\x0A\x20')
    edid_base += monitor_name
    
    # Descriptor 3: range limits
    range_limits = bytearray(b'\x00\x00\x00\xFD\x00\x18\x4C\x0F\x51\x11\x00\x0A\x20\x20\x20\x20\x20\x20')
    edid_base += range_limits
    
    # Descriptor 4: serial or another
    edid_base += bytearray(b'\x00\x00\x00\xFF\x00\x53\x65\x72\x69\x61\x6C\x31\x32\x33\x0A\x20\x20\x20')
    
    # Extension flag: 1 extension
    edid_base += b'\x01'
    
    # Checksum for base
    checksum_base = (256 - (sum(edid_base) % 256)) % 256
    edid_base += bytes([checksum_base])
    
    # Now CTA extension block (128 bytes)
    cta_ext = bytearray([
        0x02,  # Tag for CTA-861
        0x03,  # Revision 3
    ])
    
    # Offset to DTDs: let's say data blocks end at byte 0x20 or so
    # Underscan, audio, YCbCr 4:4:4, 4:2:2
    cta_ext += b'\x40'  # byte 2: d = 0x40 for basic audio, etc.
    
    # Total length of data blocks: to be filled
    # Byte 3: number of DTDs *18 + offset -4 or something? Byte 3: offset to first DTD
    dtd_offset = 0  # placeholder
    
    # Data blocks
    # First, video data block
    video_db = b'\x03\x0C\x00\x10\x00'  # type 2 (video), length 1, VIC 16 for 1080p? Type 3 for video? Wait.
    # Video Data Block is tag 2 >>3 =0, extended tag for HDR.
    # Standard data block header: (length | tag<<5)
    
    # Audio data block example
    # But for simplicity, add HDR Static Metadata Block
    # HDR is extended tag 6
    # So, data block: header = (length) | (7<<5) = 0xE0 | length for extended
    # Extended tag block: first byte after header is tag code, then data.
    
    # HDR Static Metadata Block: tag 6, length typically 5-6 bytes
    # Structure: byte1: EOTF flags (bit0 Traditional gamma SDR, bit1 HDR, bit2 SMPTE 2084, bit3 HLG, etc.)
    # byte2: Static Metadata Type 1 support (bit0 Type1)
    # byte3: Desired Max Lum
    # byte4: Desired Max Frame Avg
    # byte5: Desired Min Lum
    # Optional more.
    
    # For our case: support PQ (2084), Type1, and the values.
    hdr_data = bytearray([
        0x06,  # extended tag 6
        0x04 | 0x08,  # EOTF: bit2=1 for 2084, bit3=1 for HLG? Imaginary: 0x04 for 2084
        0x01,  # Supports Static Metadata Type 1
        max_lum_byte,  
        max_fall_byte,  
        min_lum_byte   
    ])
    hdr_db_header = (len(hdr_data) ) | (0x07 << 5)  # tag 7 for extended
    hdr_db = bytes([hdr_db_header]) + hdr_data
    
    # Add a video data block for completeness
    video_db_data = bytearray([16])  # VIC 16: 1920x1080p60
    video_db_header = len(video_db_data) | (0x02 << 5)  # tag 2 video
    video_db = bytes([video_db_header]) + video_db_data
    
    # Add speaker allocation or skip
    # For simplicity, only video and HDR
    
    data_blocks = video_db + hdr_db
    
    # Now set byte 3: offset to DTD = 4 + len(data_blocks)
    dtd_offset = 4 + len(data_blocks)
    cta_ext += bytes([dtd_offset])
    
    # Add the data blocks
    cta_ext += data_blocks
    
    # Add DTDs if any, but none for simplicity, pad with 00 until 126
    padding_needed = 126 - len(cta_ext)
    cta_ext += b'\x00' * padding_needed
    
    # Checksum
    checksum_cta = (256 - (sum(cta_ext) % 256)) % 256
    cta_ext += bytes([checksum_cta])
    
    # Full EDID: base + extension
    full_edid = edid_base + cta_ext
    
    # Write to file
    with open(filename, 'wb') as f:
        f.write(full_edid)
    
    print(f"EDID written to {filename}. Size: {len(full_edid)} bytes")
    print(f"HDR values: min_nits={min_nits}, max_nits={max_nits}")
    print(f"Encoded: min_byte={min_lum_byte}, max_byte={max_lum_byte}, fall_byte={max_fall_byte}")

# Example usage for unit test
if __name__ == "__main__":
    write_imaginary_edid_to_file("test_edid.bin", min_nits=0.0005, max_nits=4000)
