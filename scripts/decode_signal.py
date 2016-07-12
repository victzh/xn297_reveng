#!/bin/env python

import sys, os, argparse

def cut(f, start, stride, bitcount, zero):
    byte = 0
    bit_in_byte_cnt = 0
    bytecnt = 0
    score = 0
    packet = []
    for i in xrange(bitcount):
#        print "Seek to %d" % (start + i*stride)
        f.seek(start + i*stride, os.SEEK_SET)
        b = f.read(1)
        if len(b) == 0:
#            print "\nEOF"
            break
        val = ord(b)
        if val > 127: val -= 256
#        print "%02x - %d" % (ord(b), val)
        if val >= zero: bitval = 1
        else: bitval = 0
        score += abs(val)
        byte = (byte << 1) | bitval
        bit_in_byte_cnt += 1
        if bit_in_byte_cnt == 8:
            packet.append(byte)
            bit_in_byte_cnt = 0
            byte = 0
            bytecnt += 1
            if bytecnt == 8:
#                print
                bytecnt = 0
#    print " %d" % (score/count)
    return packet

def main(argv):
    parser = argparse.ArgumentParser(description='Cut packet from demodulated signal', fromfile_prefix_chars='@')
    parser.add_argument('f_in', type=argparse.FileType('rb'), metavar='INFILE', help='Input file name')
    parser.add_argument('-c', dest='count', type=int, default=256, help="Packet size to cut in bits")
    parser.add_argument('-s', dest='starts', type=int, metavar='START', nargs='*', default=0, help='Start positions')
    parser.add_argument('-t', dest='stride', type=int, metavar='STRIDE', nargs='?', default=16, help='Stride')
    parser.add_argument('-z', dest='zero', type=int, metavar='ZERO', nargs='?', default=0, help='Zero level')
    a = parser.parse_args(argv)
    for start in a.starts:
        packet = cut(a.f_in, start, a.stride, a.count, a.zero)
#        print ", ".join(map(lambda x: "0x%02x" %x, packet))
        print "".join(map(lambda x: "%02x" %x, packet))
    return 0

if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
