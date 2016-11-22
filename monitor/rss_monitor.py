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

	while true:
		rss_out = subprocess.check_output(['ps','-C',sys.argv[1],'-o','rss'])
		max_rss = max(max_rss,int(rss_out.split('\n')[1]))
		time.sleep(0.1)