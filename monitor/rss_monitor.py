#!/usr/bin/env python

import signal
import sys
import subprocess
import time

max_rss = 0

def signal_handler(signal, frame):
        print('Max RSS:'+str(max_rss))
        sys.exit(0)
        
        
if __name__ == "__main__":    
	signal.signal(signal.SIGINT, signal_handler)

	while True:
		try:
			rss_out = subprocess.check_output(['ps','-C',sys.argv[1],'-o','rss']).splitlines()
			if(len(rss_out)>1):
				max_rss = max(max_rss,int(rss_out[1]))
		except Exception:
			pass
		time.sleep(0.1)
