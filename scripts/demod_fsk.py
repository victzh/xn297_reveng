#!/usr/bin/env python
##################################################
# Gnuradio Python Flow Graph
# Title: Demod Fsk Gen
# Generated: Sun May 03 13:37:25 2015
##################################################

from gnuradio import analog
from gnuradio import blocks
from gnuradio import eng_notation
from gnuradio import filter
from gnuradio import gr
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from argparse import ArgumentParser, FileType
import math

class demod_fsk_gen(gr.top_block):

    def __init__(self, f_in, f_out, base_freq, freq_offset, samp_rate, bit_rate):
        gr.top_block.__init__(self, "Demod Fsk Gen")

        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate
        self.bit_rate = bit_rate
        self.samp_per_sym = samp_per_sym = int(samp_rate/bit_rate)
        self.fxff_decimation = fxff_decimation = 1
        self.fsk_deviation_hz = fsk_deviation_hz = 160000
        self.freq_offset = freq_offset
        self.base_freq = base_freq
        if not f_out:
            f_out = f_in + '.demod'

        ##################################################
        # Blocks
        ##################################################
        self.low_pass_filter_0 = filter.fir_filter_fff(1, firdes.low_pass(
        	1, samp_rate/fxff_decimation, bit_rate*0.8, bit_rate*.2, firdes.WIN_BLACKMAN, 6.76))
        self.freq_xlating_fir_filter_xxx_0 = filter.freq_xlating_fir_filter_ccc(fxff_decimation, (firdes.low_pass(1, samp_rate, bit_rate*1.1, bit_rate*.4,  firdes.WIN_BLACKMAN, 6.76)), freq_offset, samp_rate)
        self.blocks_uchar_to_float_0 = blocks.uchar_to_float()
        self.blocks_throttle_0 = blocks.throttle(gr.sizeof_gr_complex*1, samp_rate/32,True)
        self.blocks_float_to_complex_0 = blocks.float_to_complex(1)
        self.blocks_float_to_char_0 = blocks.float_to_char(1, 1)
        self.blocks_file_source_0 = blocks.file_source(gr.sizeof_char*1, f_in, False)
        self.blocks_file_sink_0 = blocks.file_sink(gr.sizeof_char*1, f_out, False)
        self.blocks_file_sink_0.set_unbuffered(False)
        self.blocks_deinterleave_0 = blocks.deinterleave(gr.sizeof_float*1)
        self.analog_quadrature_demod_cf_0 = analog.quadrature_demod_cf(samp_rate/(2*math.pi*fsk_deviation_hz/8.0)/fxff_decimation)
        self.analog_pwr_squelch_xx_0 = analog.pwr_squelch_cc(30, 0.3, 0, False)

        ##################################################
        # Connections
        ##################################################
        self.connect((self.blocks_file_source_0, 0), (self.blocks_uchar_to_float_0, 0))
        self.connect((self.blocks_uchar_to_float_0, 0), (self.blocks_deinterleave_0, 0))
        self.connect((self.blocks_deinterleave_0, 0), (self.blocks_float_to_complex_0, 0))
        self.connect((self.blocks_deinterleave_0, 1), (self.blocks_float_to_complex_0, 1))
        self.connect((self.blocks_float_to_complex_0, 0), (self.blocks_throttle_0, 0))
        self.connect((self.blocks_throttle_0, 0), (self.freq_xlating_fir_filter_xxx_0, 0))
        self.connect((self.low_pass_filter_0, 0), (self.blocks_float_to_char_0, 0))
        self.connect((self.analog_quadrature_demod_cf_0, 0), (self.low_pass_filter_0, 0))
        self.connect((self.blocks_float_to_char_0, 0), (self.blocks_file_sink_0, 0))
        self.connect((self.freq_xlating_fir_filter_xxx_0, 0), (self.analog_pwr_squelch_xx_0, 0))
        self.connect((self.analog_pwr_squelch_xx_0, 0), (self.analog_quadrature_demod_cf_0, 0))


# QT sink close method reimplementation

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.set_samp_per_sym(int(self.samp_rate/self.bit_rate))
        self.blocks_throttle_0.set_sample_rate(self.samp_rate/32)
        self.low_pass_filter_0.set_taps(firdes.low_pass(1, self.samp_rate/self.fxff_decimation, self.bit_rate*0.8, self.bit_rate*.2, firdes.WIN_BLACKMAN, 6.76))
        self.freq_xlating_fir_filter_xxx_0.set_taps((firdes.low_pass(1, self.samp_rate, self.bit_rate*1.1, self.bit_rate*.4,  firdes.WIN_BLACKMAN, 6.76)))
        self.analog_quadrature_demod_cf_0.set_gain(self.samp_rate/(2*math.pi*self.fsk_deviation_hz/8.0)/self.fxff_decimation)

    def get_bit_rate(self):
        return self.bit_rate

    def set_bit_rate(self, bit_rate):
        self.bit_rate = bit_rate
        self.set_samp_per_sym(int(self.samp_rate/self.bit_rate))
        self.low_pass_filter_0.set_taps(firdes.low_pass(1, self.samp_rate/self.fxff_decimation, self.bit_rate*0.8, self.bit_rate*.2, firdes.WIN_BLACKMAN, 6.76))
        self.freq_xlating_fir_filter_xxx_0.set_taps((firdes.low_pass(1, self.samp_rate, self.bit_rate*1.1, self.bit_rate*.4,  firdes.WIN_BLACKMAN, 6.76)))

    def get_variable_config_0(self):
        return self.variable_config_0

    def set_variable_config_0(self, variable_config_0):
        self.variable_config_0 = variable_config_0

    def get_samp_per_sym(self):
        return self.samp_per_sym

    def set_samp_per_sym(self, samp_per_sym):
        self.samp_per_sym = samp_per_sym

    def get_fxff_decimation(self):
        return self.fxff_decimation

    def set_fxff_decimation(self, fxff_decimation):
        self.fxff_decimation = fxff_decimation
        self.low_pass_filter_0.set_taps(firdes.low_pass(1, self.samp_rate/self.fxff_decimation, self.bit_rate*0.8, self.bit_rate*.2, firdes.WIN_BLACKMAN, 6.76))
        self.analog_quadrature_demod_cf_0.set_gain(self.samp_rate/(2*math.pi*self.fsk_deviation_hz/8.0)/self.fxff_decimation)

    def get_fsk_deviation_hz(self):
        return self.fsk_deviation_hz

    def set_fsk_deviation_hz(self, fsk_deviation_hz):
        self.fsk_deviation_hz = fsk_deviation_hz
        self.analog_quadrature_demod_cf_0.set_gain(self.samp_rate/(2*math.pi*self.fsk_deviation_hz/8.0)/self.fxff_decimation)

    def get_freq_offset(self):
        return self.freq_offset

    def set_freq_offset(self, freq_offset):
        self.freq_offset = freq_offset
        self.freq_xlating_fir_filter_xxx_0.set_center_freq(self.freq_offset)

    def get_file_prefix(self):
        return self.file_prefix

    def set_file_prefix(self, file_prefix):
        self.file_prefix = file_prefix

    def get_base_freq(self):
        return self.base_freq

    def set_base_freq(self, base_freq):
        self.base_freq = base_freq

if __name__ == '__main__':
#    parser = OptionParser(option_class=eng_option, usage="%prog: [options]")
#    (options, args) = parser.parse_args()
    parser = ArgumentParser(description='Demodulate (G)FSK signal')
    parser.add_argument('f_in', metavar='INFILE', help='Input file name')
    parser.add_argument('f_out', metavar='OUTFILE', nargs='?', default='', help='Output file name')
    parser.add_argument('-f', dest='freq', type=int, default=2400000000, help="Frequency of recording")
    parser.add_argument('-o', dest='offset', type=int, nargs='?', default=2000000, help='Offset of signal center frequency')
    parser.add_argument('-s', dest='samprate', type=int, nargs='?', default=16000000, help='Sample rate')
    parser.add_argument('-b', dest='bitrate', type=int, nargs='?', default=1000000, help='Data rate in bps')
    args = parser.parse_args()
    tb = demod_fsk_gen(args.f_in, args.f_out, args.freq, args.offset, args.samprate, args.bitrate)
    tb.start()
    tb.wait()

