def crc16_update1(crc, a):
    crc ^= a
    for i in xrange(8):
        if crc & 1:
            crc = (crc >> 1) ^ 0x1021 # reversed, 0x8408 for CCITT (normal 0x1021)
        else:
            crc = (crc >> 1)
    return crc

polynomial = 0x1021
def crc16_update(crc, a):
    crc ^= a << 8;
    for i in xrange(8):
        if crc & 0x8000:
            crc = (crc << 1) ^ polynomial
        else:
            crc = crc << 1
    return crc & 0xffff


crc = 0x7ef8
#for a in [0x55, 0x2f, 0x7d, 0x87, 0x26, 0x49, 0xe9, 0x08, 0x7c, 0x6b, 0x62, 0xb7, 0x28, 0x29, 0xc9, 0xf9, 0xdf, 0xfc, 0xc2, 0x97, 0xd5]:
for a in [0, 0, 0, 0x80, 0]:
    crc = crc16_update(crc, a)

print "0x%04x" % (crc ^ 0x1b05)
