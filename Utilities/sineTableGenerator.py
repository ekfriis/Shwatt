
import math

#256 divitions in pi/2

#nDivisions = 256
nDivisions = 32

f=open('lookupTable.h', 'w')

f.write("#ifndef AVR_TRIGLOOKUPTABLE_H\n")
f.write("#define AVR_TRIGLOOKUPTABLE_H\n")

f.write("prog_int16_t sineQuarterWaveLookupTableByteAddressed[] PROGMEM = {")

stepSize = (math.pi/(2.0*(nDivisions-1)))

outputSize = pow(2,15)-1

for x in range(0, nDivisions):
   if not x % 8:
      f.write("\n")
   pointInRadians = stepSize*x
   sineValue = int(round(math.sin(pointInRadians)*outputSize))
   print pointInRadians
   print sineValue
   print "0x%06x" % sineValue
   f.write("\t0x%04x" % sineValue)
   if x != nDivisions-1:
      f.write(",")
   else:
      f.write("\n};\n")

f.write("#endif\n")
f.close()
   
