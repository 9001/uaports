import threading
import sys

def fun(i):
	try:
		fun(i+1)
	except:
		sys.exit(0)

