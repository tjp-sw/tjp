#!/bin/env python

import numpy as np
from scipy.io.wavfile import read as wavread
from scipy.ndimage.filters import maximum_filter1d
import sys

#filein='/Users/raysmets/Downloads/tjp_songs/38_DAY1_MID_rain-on-window-sill-#2.wav'
filein = sys.argv[1]
print "anayalzing " + str(sys.argv[1])

def max_filter1d_valid(a, W):
    hW = (W-1)//2 # Half window size
    return maximum_filter1d(a,size=W)[hW:-hW]

def down_avg(arr, n):
    end =  n * int(len(arr)/n)
    return np.mean(np.array( arr[:end] ).reshape(-1, n), 1)

def find_env_outliers( filein,
						stdevs = 4.5,
						histrs = 1,
						dwn_samp = 1,
						env_buf_size = .05 # 50 ms buffer for envelope
	 				):
	# Load the file
	(sample_rate, data) = wavread( filein )

	# Amplitude for peak detection: max of absolute values
	amp   = [ float( max( abs(l), abs(r) ) ) for (l,r) in data ]

	# Envelope detection
	buf_sz= int( sample_rate * env_buf_size )
	env   = max_filter1d_valid( amp, buf_sz )

	env   = down_avg( env, dwn_samp )
	# env   = np.ediff1d( env, to_begin=0 )

	# Statisticall analysis of envelope
	menv  = np.mean( env )
	sdenv = np.std( env )
	z     = [ (x-menv)/sdenv for x in env ]
	cands = [ (float(i*dwn_samp+buf_sz/2.0)/sample_rate,x) for (i,x) in enumerate(z) if x>stdevs ]
	last  = -100
	events= []
	for (t,x) in cands:
		if t-last>histrs:
			last=t
			events.append( (t,x) )
			print 'EVENT: %.3f seconds %.2f stdevs from the mean' % ( t, x )

find_env_outliers(filein)
