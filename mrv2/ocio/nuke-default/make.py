#!/usr/bin/env python

import math, os, sys
import PyOpenColorIO as OCIO


print("OCIO",OCIO.GetVersion())

outputfilename = "config.ocio"

def WriteSPI1D(filename, fromMin, fromMax, data):
    f = open(filename,'w')
    f.write("Version 1\n")
    f.write("From %s %s\n" % (fromMin, fromMax))
    f.write("Length %d\n" % len(data))
    f.write("Components 1\n")
    f.write("{\n")
    for value in data:
        f.write("        %s\n" % value)
    f.write("}\n")
    f.close()

def Fit(value, fromMin, fromMax, toMin, toMax):
    if fromMin == fromMax:
        raise ValueError("fromMin == fromMax")
    return (value - fromMin) / (fromMax - fromMin) * (toMax - toMin) + toMin


###############################################################################


config = OCIO.Config()
config.setSearchPath('luts')

config.setRole(OCIO.ROLE_SCENE_LINEAR, "linear")
config.setRole(OCIO.ROLE_REFERENCE, "linear")
config.setRole(OCIO.ROLE_COLOR_TIMING, "Cineon")
config.setRole(OCIO.ROLE_COMPOSITING_LOG, "Cineon")
config.setRole(OCIO.ROLE_DATA,"raw")
config.setRole(OCIO.ROLE_DEFAULT,"raw")
config.setRole(OCIO.ROLE_COLOR_PICKING,"sRGB")
config.setRole(OCIO.ROLE_MATTE_PAINT,"sRGB")
config.setRole(OCIO.ROLE_TEXTURE_PAINT,"sRGB")


###############################################################################

cs = OCIO.ColorSpace(name='linear')
cs.setDescription("Scene-linear, high dynamic range. Used for rendering and compositing.")
cs.setBitDepth(OCIO.BIT_DEPTH_F32)
cs.setAllocation(OCIO.ALLOCATION_LG2)
cs.setAllocationVars([-15.0, 6.0])
config.addColorSpace(cs)


###############################################################################

def toSRGB(v):
    if v<0.04045/12.92:
        return v*12.92
    return 1.055 * v**(1.0/2.4) - 0.055

def fromSRGB(v):
    if v<0.04045:
        return v/12.92
    return ((v + .055) / 1.055) ** 2.4

# These samples and range have been chosen to write out this colorspace with
# a limited over/undershoot range, which also exactly samples the 0.0,1.0
# crossings

NUM_SAMPLES = 2**12+5
RANGE = (-0.125, 1.125)
data = []
for i in range(NUM_SAMPLES):
    x = i/(NUM_SAMPLES-1.0)
    x = Fit(x, 0.0, 1.0, RANGE[0], RANGE[1])
    data.append(fromSRGB(x))

# Data is srgb->linear
WriteSPI1D('luts/srgb.spi1d', RANGE[0], RANGE[1], data)

cs = OCIO.ColorSpace(name='sRGB')
cs.setDescription("Standard RGB Display Space")
cs.setBitDepth(OCIO.BIT_DEPTH_F32)
cs.setAllocation(OCIO.ALLOCATION_UNIFORM)
cs.setAllocationVars([RANGE[0], RANGE[1]])

t = OCIO.FileTransform('srgb.spi1d', interpolation=OCIO.INTERP_LINEAR)
cs.setTransform(t, OCIO.COLORSPACE_DIR_TO_REFERENCE)
config.addColorSpace(cs)

#Disabled pending matching root lut with extra range & name in Nuke.
NUM_SAMPLES = 2**16+25
RANGE = (-0.125, 4.875)
data = []
for i in range(NUM_SAMPLES):
   x = i/(NUM_SAMPLES-1.0)
   x = Fit(x, 0.0, 1.0, RANGE[0], RANGE[1])
   data.append(fromSRGB(x))

# Data is srgb->linear
WriteSPI1D('luts/srgbf.spi1d', RANGE[0], RANGE[1], data)

cs = OCIO.ColorSpace(name='sRGBf')
cs.setDescription("Standard RGB Display Space, but with additional range to preserve float highlights.")
cs.setBitDepth(OCIO.BIT_DEPTH_F32)
cs.setAllocation(OCIO.ALLOCATION_UNIFORM)
cs.setAllocationVars([RANGE[0], RANGE[1]])

t = OCIO.FileTransform('srgbf.spi1d', interpolation=OCIO.INTERP_LINEAR)
cs.setTransform(t, OCIO.COLORSPACE_DIR_TO_REFERENCE)
config.addColorSpace(cs)


###############################################################################

def toRec709(v):
    if v<0.018:
        return v*4.5
    return 1.099 * v**0.45 - 0.099

def fromRec709(v):
    if v<0.018*4.5:
        return v/4.5
    return ((v + .099) / 1.099) ** (1.0/0.45)

# These samples and range have been chosen to write out this colorspace with
# a limited over/undershoot range, which also exactly samples the 0.0,1.0
# crossings

NUM_SAMPLES = 2**12+5
RANGE = (-0.125, 1.125)
data = []
for i in range(NUM_SAMPLES):
    x = i/(NUM_SAMPLES-1.0)
    x = Fit(x, 0.0, 1.0, RANGE[0], RANGE[1])
    data.append(fromRec709(x))

# Data is srgb->linear
WriteSPI1D('luts/rec709.spi1d', RANGE[0], RANGE[1], data)

cs = OCIO.ColorSpace(name='rec709')
cs.setDescription("Rec. 709 (Full Range) Display Space")
cs.setBitDepth(OCIO.BIT_DEPTH_F32)
cs.setAllocation(OCIO.ALLOCATION_UNIFORM)
cs.setAllocationVars([RANGE[0], RANGE[1]])

t = OCIO.FileTransform('rec709.spi1d', interpolation=OCIO.INTERP_LINEAR)
cs.setTransform(t, OCIO.COLORSPACE_DIR_TO_REFERENCE)
config.addColorSpace(cs)


###############################################################################

cineonBlackOffset = 10.0 ** ((95.0 - 685.0)/300.0)

def fromCineon(x):
    return (10.0**((1023.0 * x - 685.0) / 300.0) - cineonBlackOffset) / (1.0 - cineonBlackOffset)

# These samples and range have been chosen to write out this colorspace with
# a limited over/undershoot range, which also exactly samples the 0.0,1.0
# crossings

NUM_SAMPLES = 2**12+5
RANGE = (-0.125, 1.125)
data = []
for i in range(NUM_SAMPLES):
    x = i/(NUM_SAMPLES-1.0)
    x = Fit(x, 0.0, 1.0, RANGE[0], RANGE[1])
    data.append(fromCineon(x))

# Data is srgb->linear
WriteSPI1D('luts/cineon.spi1d', RANGE[0], RANGE[1], data)

cs = OCIO.ColorSpace(name='Cineon')
cs.setDescription("Cineon (Log Film Scan)")
cs.setBitDepth(OCIO.BIT_DEPTH_F32)
cs.setAllocation(OCIO.ALLOCATION_UNIFORM)
cs.setAllocationVars([RANGE[0], RANGE[1]])

t = OCIO.FileTransform('cineon.spi1d', interpolation=OCIO.INTERP_LINEAR)
cs.setTransform(t, OCIO.COLORSPACE_DIR_TO_REFERENCE)
config.addColorSpace(cs)


###############################################################################

cs = OCIO.ColorSpace(name='Gamma1.8')
cs.setDescription("Emulates an idealized Gamma 1.8 display device.")
cs.setBitDepth(OCIO.BIT_DEPTH_F32)
cs.setAllocation(OCIO.ALLOCATION_UNIFORM)
cs.setAllocationVars([0.0, 1.0])

t = OCIO.ExponentTransform(value=(1.8,1.8,1.8,1.0))
cs.setTransform(t, OCIO.COLORSPACE_DIR_TO_REFERENCE)
config.addColorSpace(cs)

cs = OCIO.ColorSpace(name='Gamma2.2')
cs.setDescription("Emulates an idealized Gamma 2.2 display device.")
cs.setBitDepth(OCIO.BIT_DEPTH_F32)
cs.setAllocation(OCIO.ALLOCATION_UNIFORM)
cs.setAllocationVars([0.0, 1.0])

t = OCIO.ExponentTransform(value=(2.2,2.2,2.2,1.0))
cs.setTransform(t, OCIO.COLORSPACE_DIR_TO_REFERENCE)
config.addColorSpace(cs)

cs = OCIO.ColorSpace(name='Gamma2.4')
cs.setDescription("Emulates an idealized Gamma 2.4 display device.")
cs.setBitDepth(OCIO.BIT_DEPTH_F32)
cs.setAllocation(OCIO.ALLOCATION_UNIFORM)
cs.setAllocationVars([0.0, 1.0])

t = OCIO.ExponentTransform(value=(2.4,2.4,2.4,1.0))
cs.setTransform(t, OCIO.COLORSPACE_DIR_TO_REFERENCE)
config.addColorSpace(cs)

cs = OCIO.ColorSpace(name='Gamma2.6')
cs.setDescription("Emulates an idealized Gamma 2.6 display device.")
cs.setBitDepth(OCIO.BIT_DEPTH_F32)
cs.setAllocation(OCIO.ALLOCATION_UNIFORM)
cs.setAllocationVars([0.0, 1.0])

t = OCIO.ExponentTransform(value=(2.6,2.6,2.6,1.0))
cs.setTransform(t, OCIO.COLORSPACE_DIR_TO_REFERENCE)
config.addColorSpace(cs)


###############################################################################

# Log to Linear light conversions for Panalog
# WARNING: these are estimations known to be close enough.
# The actual transfer functions are not published

panalogBlackOffset = 10.0 ** ((64.0 - 681.0) / 444.0)

def fromPanalog(x):
    return (10.0**((1023 * x - 681.0) / 444.0) - panalogBlackOffset) / (1.0 - panalogBlackOffset)

# These samples and range have been chosen to write out this colorspace with
# a limited over/undershoot range, which also exactly samples the 0.0,1.0
# crossings

NUM_SAMPLES = 2**12+5
RANGE = (-0.125, 1.125)
data = []
for i in range(NUM_SAMPLES):
    x = i/(NUM_SAMPLES-1.0)
    x = Fit(x, 0.0, 1.0, RANGE[0], RANGE[1])
    data.append(fromPanalog(x))

# Data is srgb->linear
WriteSPI1D('luts/panalog.spi1d', RANGE[0], RANGE[1], data)

cs = OCIO.ColorSpace(name='Panalog')
cs.setDescription("Sony/Panavision Genesis Log Space")
cs.setBitDepth(OCIO.BIT_DEPTH_F32)
cs.setAllocation(OCIO.ALLOCATION_UNIFORM)
cs.setAllocationVars([RANGE[0], RANGE[1]])

t = OCIO.FileTransform('panalog.spi1d', interpolation=OCIO.INTERP_LINEAR)
cs.setTransform(t, OCIO.COLORSPACE_DIR_TO_REFERENCE)
config.addColorSpace(cs)


###############################################################################

redBlackOffset = 10.0 ** ((0.0 - 1023.0) / 511.0)

def fromREDLog(x):
    return ((10.0 ** ((1023.0 * x - 1023.0) / 511.0)) - redBlackOffset) / (1.0 - redBlackOffset)

# These samples and range have been chosen to write out this colorspace with
# a limited over/undershoot range, which also exactly samples the 0.0,1.0
# crossings


NUM_SAMPLES = 2**12+5
RANGE = (-0.125, 1.125)
data = []
for i in range(NUM_SAMPLES):
    x = i/(NUM_SAMPLES-1.0)
    x = Fit(x, 0.0, 1.0, RANGE[0], RANGE[1])
    data.append(fromREDLog(x))

# Data is srgb->linear
WriteSPI1D('luts/redlog.spi1d', RANGE[0], RANGE[1], data)

cs = OCIO.ColorSpace(name='REDLog')
cs.setDescription("RED Log Space")
cs.setBitDepth(OCIO.BIT_DEPTH_F32)
cs.setAllocation(OCIO.ALLOCATION_UNIFORM)
cs.setAllocationVars([RANGE[0], RANGE[1]])

t = OCIO.FileTransform('redlog.spi1d', interpolation=OCIO.INTERP_LINEAR)
cs.setTransform(t, OCIO.COLORSPACE_DIR_TO_REFERENCE)
config.addColorSpace(cs)


###############################################################################

def fromViperLog(x):
    return 10.0**((1023.0 * x - 1023.0) / 500.0)

# These samples and range have been chosen to write out this colorspace with
# a limited over/undershoot range, which also exactly samples the 0.0,1.0
# crossings


NUM_SAMPLES = 2**12+5
RANGE = (-0.125, 1.125)
data = []
for i in range(NUM_SAMPLES):
    x = i/(NUM_SAMPLES-1.0)
    x = Fit(x, 0.0, 1.0, RANGE[0], RANGE[1])
    data.append(fromViperLog(x))

# Data is srgb->linear
WriteSPI1D('luts/viperlog.spi1d', RANGE[0], RANGE[1], data)

cs = OCIO.ColorSpace(name='ViperLog')
cs.setDescription("Viper Log Space")
cs.setBitDepth(OCIO.BIT_DEPTH_F32)
cs.setAllocation(OCIO.ALLOCATION_UNIFORM)
cs.setAllocationVars([RANGE[0], RANGE[1]])

t = OCIO.FileTransform('viperlog.spi1d', interpolation=OCIO.INTERP_LINEAR)
cs.setTransform(t, OCIO.COLORSPACE_DIR_TO_REFERENCE)
config.addColorSpace(cs)


###############################################################################

alexav3logc_a = 5.555556
alexav3logc_b = 0.052272
alexav3logc_c = 0.247190
alexav3logc_d = 0.385537
alexav3logc_e = 5.367655
alexav3logc_f = 0.092809
alexav3logc_cut = 0.010591
alexav3logc_eCutF = alexav3logc_e*alexav3logc_cut + alexav3logc_f

# This corresponds to EI800 per Arri Doc
# http://www.arridigital.com/forum/index.php?topic=6372.0
# http://www.arri.com/?eID=registration&file_uid=7775
def fromAlexaV3LogC(x):
    if x > alexav3logc_eCutF:
        return (10.0 **((x - alexav3logc_d) / alexav3logc_c) - alexav3logc_b) / alexav3logc_a
    else:
        return (x - alexav3logc_f) / alexav3logc_e


# These samples and range have been chosen to write out this colorspace with
# a limited over/undershoot range, which also exactly samples the 0.0,1.0
# crossings


NUM_SAMPLES = 2**12+5
RANGE = (-0.125, 1.125)
data = []
for i in range(NUM_SAMPLES):
    x = i/(NUM_SAMPLES-1.0)
    x = Fit(x, 0.0, 1.0, RANGE[0], RANGE[1])
    data.append(fromAlexaV3LogC(x))

# Data is srgb->linear
WriteSPI1D('luts/alexalogc.spi1d', RANGE[0], RANGE[1], data)

cs = OCIO.ColorSpace(name='AlexaV3LogC')
cs.setDescription("Alexa Log C")
cs.setBitDepth(OCIO.BIT_DEPTH_F32)
cs.setAllocation(OCIO.ALLOCATION_UNIFORM)
cs.setAllocationVars([RANGE[0], RANGE[1]])

t = OCIO.FileTransform('alexalogc.spi1d', interpolation=OCIO.INTERP_LINEAR)
cs.setTransform(t, OCIO.COLORSPACE_DIR_TO_REFERENCE)
config.addColorSpace(cs)


###############################################################################

'PLogLin'

# Josh Pines style pivoted log/lin conversion
minLinValue = 1e-10
linReference = 0.18
logReference = 445.0
negativeGamma = 0.6
densityPerCodeValue = 0.002
ngOverDpcv = negativeGamma/densityPerCodeValue
dpcvOverNg = densityPerCodeValue/negativeGamma

def fromPLogLin(x):
    return (10.0**((x*1023.0 - logReference)*dpcvOverNg ) * linReference)


# These samples and range have been chosen to write out this colorspace with
# a limited over/undershoot range, which also exactly samples the 0.0,1.0
# crossings

NUM_SAMPLES = 2**12+5
RANGE = (-0.125, 1.125)
data = []
for i in range(NUM_SAMPLES):
    x = i/(NUM_SAMPLES-1.0)
    x = Fit(x, 0.0, 1.0, RANGE[0], RANGE[1])
    data.append(fromPLogLin(x))

# Data is srgb->linear
WriteSPI1D('luts/ploglin.spi1d', RANGE[0], RANGE[1], data)

cs = OCIO.ColorSpace(name='PLogLin')
cs.setDescription("Josh Pines style pivoted log/lin conversion. 445->0.18")
cs.setBitDepth(OCIO.BIT_DEPTH_F32)
cs.setAllocation(OCIO.ALLOCATION_UNIFORM)
cs.setAllocationVars([RANGE[0], RANGE[1]])

t = OCIO.FileTransform('ploglin.spi1d', interpolation=OCIO.INTERP_LINEAR)
cs.setTransform(t, OCIO.COLORSPACE_DIR_TO_REFERENCE)
config.addColorSpace(cs)


###############################################################################

'SLog'

#Note this is an old version that doesn't take black offsets into account.
#SLog1 should be used instead, this is here for back compat.

def fromSLog(x):
    return (10.0 ** (((x - 0.616596 - 0.03) / 0.432699)) - 0.037584)

# These samples and range have been chosen to write out this colorspace with
# a limited over/undershoot range, which also exactly samples the 0.0,1.0
# crossings

NUM_SAMPLES = 2**12+5
RANGE = (-0.125, 1.125)
data = []
for i in range(NUM_SAMPLES):
    x = i/(NUM_SAMPLES-1.0)
    x = Fit(x, 0.0, 1.0, RANGE[0], RANGE[1])
    data.append(fromSLog(x))

# Data is srgb->linear
WriteSPI1D('luts/slog.spi1d', RANGE[0], RANGE[1], data)

cs = OCIO.ColorSpace(name='SLog')
cs.setDescription("Sony SLog")
cs.setBitDepth(OCIO.BIT_DEPTH_F32)
cs.setAllocation(OCIO.ALLOCATION_UNIFORM)
cs.setAllocationVars([RANGE[0], RANGE[1]])

t = OCIO.FileTransform('slog.spi1d', interpolation=OCIO.INTERP_LINEAR)
cs.setTransform(t, OCIO.COLORSPACE_DIR_TO_REFERENCE)
config.addColorSpace(cs)


###############################################################################

'SLog1'

#The original SLog is actually incorrect (see note above), this should be used
#in its place.

def fromSLog1(x):
  return (((10.0 ** (((((x*1023.0)/4.0-16.0)/219.0)-0.616596-0.03)/0.432699))-0.037584)*0.9)

# These samples and range have been chosen to write out this colorspace with
# a limited over/undershoot range, which also exactly samples the 0.0,1.0
# crossings

NUM_SAMPLES = 2**12+5
RANGE = (-0.125, 1.125)
data = []
for i in range(NUM_SAMPLES):
  x = i/(NUM_SAMPLES-1.0)
  x = Fit(x, 0.0, 1.0, RANGE[0], RANGE[1])
  data.append(fromSLog(x))

# Data is srgb->linear
WriteSPI1D('luts/slog1.spi1d', RANGE[0], RANGE[1], data)

cs = OCIO.ColorSpace(name='SLog1')
cs.setDescription("Sony SLog1")
cs.setBitDepth(OCIO.BIT_DEPTH_F32)
cs.setAllocation(OCIO.ALLOCATION_UNIFORM)
cs.setAllocationVars([RANGE[0], RANGE[1]])

t = OCIO.FileTransform('slog1.spi1d', interpolation=OCIO.INTERP_LINEAR)
cs.setTransform(t, OCIO.COLORSPACE_DIR_TO_REFERENCE)
config.addColorSpace(cs)


###############################################################################

'SLog2'

def fromSLog2(x):
  i = (x - 0.06256) / 0.8563;
  if i >= 0.030001222851889303:
    return ((219.0*((10.0 ** ((i-0.616596-0.03)/0.432699))-0.037584)/155.0)*0.9)
  return ((i-0.030001222851889303)*0.28258064516129*0.9)

# These samples and range have been chosen to write out this colorspace with
# a limited over/undershoot range, which also exactly samples the 0.0,1.0
# crossings

NUM_SAMPLES = 2**12+5
RANGE = (-0.125, 1.125)
data = []
for i in range(NUM_SAMPLES):
  x = i/(NUM_SAMPLES-1.0)
  x = Fit(x, 0.0, 1.0, RANGE[0], RANGE[1])
  data.append(fromSLog2(x))

# Data is srgb->linear
WriteSPI1D('luts/slog2.spi1d', RANGE[0], RANGE[1], data)

cs = OCIO.ColorSpace(name='SLog2')
cs.setDescription("Sony SLog2")
cs.setBitDepth(OCIO.BIT_DEPTH_F32)
cs.setAllocation(OCIO.ALLOCATION_UNIFORM)
cs.setAllocationVars([RANGE[0], RANGE[1]])

t = OCIO.FileTransform('slog2.spi1d', interpolation=OCIO.INTERP_LINEAR)
cs.setTransform(t, OCIO.COLORSPACE_DIR_TO_REFERENCE)
config.addColorSpace(cs)


###############################################################################

'SLog3'

def fromSLog3(x):
  if x >= 171.2102946929/1023.0:
    return (((10.0 ** ((x*1023.0-420.0)/261.5))*(0.18+0.01))-0.01)
  return ((x*1023.0-95.0)*0.01125/(171.2102946929-95.0))

# These samples and range have been chosen to write out this colorspace with
# a limited over/undershoot range, which also exactly samples the 0.0,1.0
# crossings

NUM_SAMPLES = 2**12+5
RANGE = (-0.125, 1.125)
data = []
for i in range(NUM_SAMPLES):
  x = i/(NUM_SAMPLES-1.0)
  x = Fit(x, 0.0, 1.0, RANGE[0], RANGE[1])
  data.append(fromSLog3(x))

# Data is srgb->linear
WriteSPI1D('luts/slog3.spi1d', RANGE[0], RANGE[1], data)

cs = OCIO.ColorSpace(name='SLog3')
cs.setDescription("Sony SLog3")
cs.setBitDepth(OCIO.BIT_DEPTH_F32)
cs.setAllocation(OCIO.ALLOCATION_UNIFORM)
cs.setAllocationVars([RANGE[0], RANGE[1]])

t = OCIO.FileTransform('slog3.spi1d', interpolation=OCIO.INTERP_LINEAR)
cs.setTransform(t, OCIO.COLORSPACE_DIR_TO_REFERENCE)
config.addColorSpace(cs)


###############################################################################

'CLog'

def fromCLog(x):
  return (((10.0 ** (x - 0.0730597)/0.529136) - 1.0)/10.1596)

# These samples and range have been chosen to write out this colorspace with
# a limited over/undershoot range, which also exactly samples the 0.0,1.0
# crossings

NUM_SAMPLES = 2**12+5
RANGE = (-0.0684932, 1.08676)
data = []
for i in range(NUM_SAMPLES):
  x = i/(NUM_SAMPLES-1.0)
  x = Fit(x, 0.0, 1.0, RANGE[0], RANGE[1])
  data.append(fromCLog(x))

# Data is srgb->linear
WriteSPI1D('luts/clog.spi1d', RANGE[0], RANGE[1], data)

cs = OCIO.ColorSpace(name='CLog')
cs.setDescription("Canon CLog")
cs.setBitDepth(OCIO.BIT_DEPTH_F32)
cs.setAllocation(OCIO.ALLOCATION_UNIFORM)
cs.setAllocationVars([RANGE[0], RANGE[1]])

t = OCIO.FileTransform('clog.spi1d', interpolation=OCIO.INTERP_LINEAR)
cs.setTransform(t, OCIO.COLORSPACE_DIR_TO_REFERENCE)
config.addColorSpace(cs)


###############################################################################

'Log3G10'

def fromLog3G10(x):
    if x < 0:
      return (x / 15.1927) - 0.01
    else:
      return ((10 ** (x / 0.224282) - 1.0) / 155.975327) - 0.01

# These samples and range have been chosen to write out this colorspace with
# a limited over/undershoot range, which also exactly samples the 0.0,1.0
# crossings

NUM_SAMPLES = 2**12+5
RANGE = (-0.125, 1.125)
data = []
for i in range(NUM_SAMPLES):
  x = i/(NUM_SAMPLES-1.0)
  x = Fit(x, 0.0, 1.0, RANGE[0], RANGE[1])
  data.append(fromLog3G10(x))

WriteSPI1D('luts/log3g10.spi1d', RANGE[0], RANGE[1], data)

cs = OCIO.ColorSpace(name='Log3G10')
cs.setDescription("Log3G10")
cs.setBitDepth(OCIO.BIT_DEPTH_F32)
cs.setAllocation(OCIO.ALLOCATION_UNIFORM)
cs.setAllocationVars([RANGE[0], RANGE[1]])

t = OCIO.FileTransform('log3g10.spi1d', interpolation=OCIO.INTERP_LINEAR)
cs.setTransform(t, OCIO.COLORSPACE_DIR_TO_REFERENCE)
config.addColorSpace(cs)


###############################################################################

'Log3G12'

def fromLog3G12(x):
    sign = math.copysign(1.0, x)
    return sign * (10 ** (abs(x) / 0.184904) - 1.0) / 347.189667

# These samples and range have been chosen to write out this colorspace with
# a limited over/undershoot range, which also exactly samples the 0.0,1.0
# crossings

NUM_SAMPLES = 2**12+5
RANGE = (-0.125, 1.125)
data = []
for i in range(NUM_SAMPLES):
  x = i/(NUM_SAMPLES-1.0)
  x = Fit(x, 0.0, 1.0, RANGE[0], RANGE[1])
  data.append(fromLog3G12(x))

WriteSPI1D('luts/log3g12.spi1d', RANGE[0], RANGE[1], data)

cs = OCIO.ColorSpace(name='Log3G12')
cs.setDescription("Log3G12")
cs.setBitDepth(OCIO.BIT_DEPTH_F32)
cs.setAllocation(OCIO.ALLOCATION_UNIFORM)
cs.setAllocationVars([RANGE[0], RANGE[1]])

t = OCIO.FileTransform('log3g12.spi1d', interpolation=OCIO.INTERP_LINEAR)
cs.setTransform(t, OCIO.COLORSPACE_DIR_TO_REFERENCE)
config.addColorSpace(cs)


###############################################################################

'HybridLogGamma'

def fromHybridLogGamma(x):
    if x < 0.5:
      return x ** 2.0 / 3.0
    else:
      return math.exp((x - 1.00429347) / 0.17883277) + 0.02372241;

# These samples and range have been chosen to write out this colorspace with
# a limited over/undershoot range, which also exactly samples the 0.0,1.0
# crossings

NUM_SAMPLES = 2**12+5
RANGE = (-0.125, 1.125)
data = []
for i in range(NUM_SAMPLES):
  x = i/(NUM_SAMPLES-1.0)
  x = Fit(x, 0.0, 1.0, RANGE[0], RANGE[1])
  data.append(fromHybridLogGamma(x))

WriteSPI1D('luts/hybdridloggamma.spi1d', RANGE[0], RANGE[1], data)

cs = OCIO.ColorSpace(name='HybridLogGamma')
cs.setDescription("HybdridLogGamma")
cs.setBitDepth(OCIO.BIT_DEPTH_F32)
cs.setAllocation(OCIO.ALLOCATION_UNIFORM)
cs.setAllocationVars([RANGE[0], RANGE[1]])

t = OCIO.FileTransform('hybdridloggamma.spi1d', interpolation=OCIO.INTERP_LINEAR)
cs.setTransform(t, OCIO.COLORSPACE_DIR_TO_REFERENCE)
config.addColorSpace(cs)


###############################################################################

'Protune'

def fromProtune(x):
  return (((113.0 ** x) - 1.0)/(112.0))

# These samples and range have been chosen to write out this colorspace with
# a limited over/undershoot range, which also exactly samples the 0.0,1.0
# crossings

NUM_SAMPLES = 2**12+5
RANGE = (-0.125, 1.125)
data = []
for i in range(NUM_SAMPLES):
  x = i/(NUM_SAMPLES-1.0)
  x = Fit(x, 0.0, 1.0, RANGE[0], RANGE[1])
  data.append(fromProtune(x))

# Data is srgb->linear
WriteSPI1D('luts/protune.spi1d', RANGE[0], RANGE[1], data)

cs = OCIO.ColorSpace(name='Protune')
cs.setDescription("GoPro Protune")
cs.setBitDepth(OCIO.BIT_DEPTH_F32)
cs.setAllocation(OCIO.ALLOCATION_UNIFORM)
cs.setAllocationVars([RANGE[0], RANGE[1]])

t = OCIO.FileTransform('protune.spi1d', interpolation=OCIO.INTERP_LINEAR)
cs.setTransform(t, OCIO.COLORSPACE_DIR_TO_REFERENCE)
config.addColorSpace(cs)

###############################################################################

'BT1886'

cs = OCIO.ColorSpace(name='BT1886')
cs.setDescription("Emulates an idealized Gamma 2.4 display device.")
cs.setBitDepth(OCIO.BIT_DEPTH_F32)
cs.setAllocation(OCIO.ALLOCATION_UNIFORM)
cs.setAllocationVars([0.0, 1.0])

t = OCIO.ExponentTransform(value=(2.4,2.4,2.4,1.0))
cs.setTransform(t, OCIO.COLORSPACE_DIR_TO_REFERENCE)
config.addColorSpace(cs)


###############################################################################

'st2084'

def fromST2084(x):
  sign = math.copysign(1.0, x)
  v = max(abs(x) ** (1.0 / 78.84375), 0.8359375)
  L = (v - 0.8359375) / (18.8515625 - v * 18.6875)
  return sign * 10000.0 * (L ** 6.277394636)

# These samples and range have been chosen to write out this colorspace with
# a limited over/undershoot range, which also exactly samples the 0.0,1.0
# crossings

NUM_SAMPLES = 2**12+5
RANGE = (-0.125, 1.125)
data = []
for i in range(NUM_SAMPLES):
  x = i/(NUM_SAMPLES-1.0)
  x = Fit(x, 0.0, 1.0, RANGE[0], RANGE[1])
  data.append(fromST2084(x))

WriteSPI1D('luts/st2084.spi1d', RANGE[0], RANGE[1], data)

cs = OCIO.ColorSpace(name='st2084')
cs.setDescription("st2084")
cs.setBitDepth(OCIO.BIT_DEPTH_F32)
cs.setAllocation(OCIO.ALLOCATION_UNIFORM)
cs.setAllocationVars([RANGE[0], RANGE[1]])

t = OCIO.FileTransform('st2084.spi1d', interpolation=OCIO.INTERP_LINEAR)
cs.setTransform(t, OCIO.COLORSPACE_DIR_TO_REFERENCE)
config.addColorSpace(cs)


##############################################################################

'REDSpace'


###############################################################################

'Blackmagic Film Generation 5'
# Constants
A = 0.08692876065491224
B = 0.005494072432257808
C = 0.5300133392291939
D = 8.283605932402494
E = 0.09246575342465753
LIN_CUT = 0.005
LOG_CUT = D * LIN_CUT + E

def blackmagic_film_gen5_oetf(x):
	if x < LIN_CUT:
		y = D * x + E
	elif x >= LIN_CUT:
		y = A * (math.log(x + B)) + C
	return y

def blackmagic_film_gen5_inv_oetf(y):
	if y < LOG_CUT:
		x = (y - E) / D
	elif y >= LOG_CUT:
		x = math.exp((y - C) / A) - B
	return x

# These samples and range have been chosen to write out this colorspace with
# a limited over/undershoot range, which also exactly samples the 0.0,1.0
# crossings

NUM_SAMPLES = 2**12+5
RANGE = (-0.125, 1.125)
data = []
for i in range(NUM_SAMPLES):
    x = i/(NUM_SAMPLES-1.0)
    x = Fit(x, 0.0, 1.0, RANGE[0], RANGE[1])
    data.append(blackmagic_film_gen5_inv_oetf(x))

# Data is srgb->linear
WriteSPI1D('luts/blackmagicfilmgen5.spi1d', RANGE[0], RANGE[1], data)

cs = OCIO.ColorSpace(name='Blackmagic Film Generation 5')
cs.setDescription("Blackmagic Film Generation 5")
cs.setBitDepth(OCIO.BIT_DEPTH_F32)
cs.setAllocation(OCIO.ALLOCATION_UNIFORM)
cs.setAllocationVars([RANGE[0], RANGE[1]])

t = OCIO.FileTransform('blackmagicfilmgen5.spi1d', interpolation=OCIO.INTERP_LINEAR)
cs.setTransform(t, OCIO.COLORSPACE_DIR_TO_REFERENCE)
config.addColorSpace(cs)

###############################################################################

'ARRI Log C4'
def fromArriLogC4(x):
    a = (2.0**18.0 - 16.0) / 117.45
    b = (1023.0 - 95.0) / 1023.0
    c = 95.0 / 1023.0
    t = (2 ** (14 * (-c / b) + 6) - 64) / a
    p1 = 14.0 * (x - c) / b + 6.0
    p2 = 14.0 * (-x - c) / b + 6.0
    if x >= 0.0:
        return (2.0**p1 - 64.0) / a
    else:
        return -((2.0**p2 - 64.0) / a) + 2*t

# These samples and range have been chosen to write out this colorspace with
# a limited over/undershoot range, which also exactly samples the 0.0,1.0
# crossings

NUM_SAMPLES = 2**12+5
RANGE = (-0.125, 1.125)
data = []
for i in range(NUM_SAMPLES):
    x = i/(NUM_SAMPLES-1.0)
    x = Fit(x, 0.0, 1.0, RANGE[0], RANGE[1])
    data.append(fromArriLogC4(x))

# Data is srgb->linear
WriteSPI1D('luts/arrilogc4.spi1d', RANGE[0], RANGE[1], data)

cs = OCIO.ColorSpace(name='ARRILogC4')
cs.setDescription("ARRI Log C4")
cs.setBitDepth(OCIO.BIT_DEPTH_F32)
cs.setAllocation(OCIO.ALLOCATION_UNIFORM)
cs.setAllocationVars([RANGE[0], RANGE[1]])

t = OCIO.FileTransform('arrilogc4.spi1d', interpolation=OCIO.INTERP_LINEAR)
cs.setTransform(t, OCIO.COLORSPACE_DIR_TO_REFERENCE)
config.addColorSpace(cs)


###############################################################################

cs = OCIO.ColorSpace(name='raw')
cs.setDescription("Raw Data. Used for normals, points, etc.")
cs.setBitDepth(OCIO.BIT_DEPTH_F32)
cs.setIsData(True)
config.addColorSpace(cs)


###############################################################################

display = 'default'
config.addDisplayView(display, 'None', 'raw')
config.addDisplayView(display, 'sRGB', 'sRGB')
config.addDisplayView(display, 'sRGBf', 'sRGBf')
config.addDisplayView(display, 'rec709', 'rec709')
config.addDisplayView(display, 'rec1886', 'Gamma2.4')

config.setActiveDisplays('default')
config.setActiveViews('sRGB,sRGBf,rec709,rec1886,None')


###############################################################################



try:
    config.validate()
except Exception as e:
    print(e)

f = open(outputfilename,"w")
f.write(config.serialize())
f.close()
print("Wrote",outputfilename)

# Core/LUT/include/LUT/fnLUTConversions.h

