#!/usr/bin/env python

import signal
import sys
import subprocess
import time

max_rss = 0
latest_rss = 0

def print_quit():
	print('Max RSS:' + str(max_rss))
	print('Latest RSS:' + str(max_rss))
	sys.exit(0)
    
def signal_handler(signal, frame):
	print_quit()    
        
        
if __name__ == "__main__":    
	signal.signal(signal.SIGINT, signal_handler)

	while True:
		try:
			rss_out = subprocess.check_output(['ps', '-C', sys.argv[1], '-o', 'rss']).splitlines()
			if(len(rss_out) > 1):
				rss = int(rss_out[1])
				max_rss = max(max_rss, rss)
				latest_rss = rss
			else:
				print_quit()
		except Exception:
			print_quit()		
		time.sleep(0.1)
