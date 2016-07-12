#!/bin/env python

import sys, os, argparse, struct

import decode_signal

min_packet_length = 216 # bits
zero_gap_len = 8
pattern = 0xC710F55
#pattern = 0xFFFFFFF
pattern_bit_len = 28

def find(f_in, f_out, start, stride, count, zero):
    f_in.seek(start, os.SEEK_SET)
    buf_file_pos = start
    byte = 0
    bitcnt = 0
    bytecnt = 0
    eof = False
    packet_starts = []
    corr_pattern = []
    for i in xrange(pattern_bit_len):
        if (pattern >> (pattern_bit_len-i-1)) & 1:
            corr_pattern.append(1)
        else:
            corr_pattern.append(-1)
#    print "Using pattern", corr_pattern
    alpha = .3 / stride
    abs_avg = 0.
    max_abs_avg = 0.
    max_corr = 0.
    max_rel_corr = 0.
    min_corr = 0.
    # length in samples of pattern plus 1-bit extra for
    # fine adjustment
    half_buf_len = pattern_win_len = stride * (pattern_bit_len+1)
    buf_len = half_buf_len * 2
    sys.stderr.write("Using buffer of length %d\n" % buf_len)
    # The first half of the buffer is last cycle data, the last
    # half is just read data
    buf = [0] * buf_len
    # The beginning of just read data
    # number of consequtive zeros up to current sample
    zero_run = 0
    prev_peak_corr = 0
    prev_peak_pos = -1
    while not eof:
        b = f_in.read(half_buf_len)
        read_len = len(b)
        if len(b) == 0:
#            print "\nEOF"
            eof = True
            break
        # Copy previous active part (usually, second half)
        # to the beginning of the buffer
        for j in xrange(buf_len-read_len):
            buf[j] = buf[j+read_len]
        # Read in new data in the buffer second part (usually, half)
        k = 0
        for j in xrange(buf_len-read_len, buf_len):
            val = ord(b[k])
            if val > 127: val -= 256
            buf[j] = val
            k += 1
        # Multiply pattern read_len times with stride
        for j in xrange(read_len):
            corr = 0
            l = 0 # preamble index
            buf_end   = buf_len - read_len + j
            cur = buf[buf_end]
            abs_cur = abs(cur)
            if abs_cur < 3:
                zero_run += 1
            else:
                zero_run = 0
            abs_avg = (1 - alpha) * abs_avg + alpha * abs_cur
            # Early termination of average if too long zero run
            if zero_run >= stride:
                abs_avg = 0
            max_abs_avg = max(max_abs_avg, abs_avg)
            buf_start = buf_end - pattern_bit_len*stride

#            for k in xrange(buf_start, buf_end, stride):
#                corr += buf[k] * corr_pattern[l]
#                l += 1
            for k in xrange(buf_start, buf_end):
                l = (k-buf_start) / stride
                corr += buf[k] * corr_pattern[l]


            if abs_avg > 5:
                rel_corr = corr / abs_avg
            else:
               rel_corr = 0.
#            rel_corr = corr * abs_avg
            min_corr = min(min_corr, corr)
            max_corr = max(max_corr, corr)
            max_rel_corr = max(max_rel_corr, rel_corr)
            peak_corr = 0
            if rel_corr > pattern_bit_len*12: peak_corr = rel_corr

            if peak_corr > prev_peak_corr:
                prev_peak_corr = peak_corr
                prev_peak_pos = buf_file_pos + j
            elif prev_peak_pos >= 0:
                packet_starts.append(prev_peak_pos + stride/2)
                sys.stderr.write("packet found at %d\n" % (prev_peak_pos + stride/2))
                prev_peak_pos = -1
            elif peak_corr == 0:
                prev_peak_corr = 0

            if f_out:
                f_out.write(struct.pack('<h', 40 * int(peak_corr)))
                f_out.write(struct.pack('<h', 40 * int(rel_corr)))
#                f_out.write(struct.pack('<h', buf[buf_end]))
#            else:
#                print "%6d, %6d, %6f, %6f" % (buf[buf_end], corr, abs_avg, rel_corr)
        buf_file_pos += read_len
    sys.stderr.write("max_abs_avg %f, max_corr %d, max_rel_corr %f, min_corr %d\n" % (max_abs_avg, max_corr, max_rel_corr, min_corr))

    return packet_starts

def main(argv):
    parser = argparse.ArgumentParser(description='Find packet in demodulated signal')
    parser.add_argument('f_in', type=argparse.FileType('rb'), metavar='INFILE', help='Input file name')
    parser.add_argument('-b', dest='f_out', type=argparse.FileType('wb'), metavar='OUTFILE', nargs='?', help='Binary output file name')
    parser.add_argument('-c', dest='count', type=int, default=0, help="Packet size to cut in bits")
    parser.add_argument('-s', dest='start', type=int, metavar='START', nargs='?', default=0, help='Start position')
    parser.add_argument('-t', dest='stride', type=int, metavar='STRIDE', nargs='?', default=16, help='Stride')
    parser.add_argument('-z', dest='zero', type=int, metavar='ZERO', nargs='?', default=0, help='Zero level')
    args = parser.parse_args(argv)
    packet_starts = find(args.f_in, args.f_out, args.start, args.stride, args.count, args.zero)
    if args.count > 0:
        for packet_start in packet_starts:
            packet = decode_signal.cut(args.f_in, packet_start, args.stride, args.count, args.zero)
#            print ", ".join(map(lambda x: "0x%02x" %x, packet))
            print "".join(map(lambda x: "%02x" %x, packet))
    return 0

if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
