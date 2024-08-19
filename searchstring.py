#!/usr/bin python

import sys, re, random, argparse, math
import time

def parse_vcd(vcd_file, searchstring):
	# regular expression 
	re_time = re.compile(r"^#(\d+)")				# re obj of '#(time)'
	re_1b_val = re.compile(r"^([01zxZX])(.+)")		# re obj of e.g '0%'
	re_Nb_val = re.compile(r"^[br](\S+)\s+(.+)")	# re obj of e.g 'b0 @', 'b111 #'
	re_info = re.compile(r"^(\s+\S+)+")			# re obj of e.g '	Jun, ...'
	re_t_val = re.compile(r"^([01])([spn]+)")
	re_len = re.compile("^(\[\d+\])*(\[\d+\:\d+\])")
	#######################################

	# variables
	data = {}		# var structure values
	hier = []		# scope hierachy
	time = 0		# each '#(time)'

	check = False
	Code = ""
	MaxTime = 0
	elsetime = [0,0,0,0]
	temp = 0
	Sum = 0
	count = 0
	#######################################
	
	fh = open(vcd_file, 'r')
	print "%s, Search String: %s\n"%(vcd_file, searchstring)
	
	# READLINE
	# DIVIDE TOKEN INTO DIFFERENT STRUCTURE
	while True:
		
		line = fh.readline()

		if not line:		# EOF
			break
		line = line.strip()	# discard \s of head and tail
	
		if "$var" in line:

		#$var reg      32 #    a [31:0] $end
		#$var reg 1 * data $end
		
			ls = line.split()						# assume all on one line
			vCode = ls[3]				# code
			vName = " ".join(ls[4:-1])	# name connect with ' []'
			vName_2 = "".join(ls[4:-1])	# name connect with '[]'
			
			if searchstring == vName:
				Code = vCode
				if vCode not in data:
					data[vCode] = 0
	
		elif line.startswith('#'):
			re_time_match = re_time.match(line)
			time = re_time_match.group(1)		# recording #(time)

		elif line.startswith(('0', '1', 'x', 'z', 'b', 'r','X', 'Z')):

			if line.startswith(('0', '1', 'x', 'z', 'X', 'Z')):
				m = re_1b_val.match(line)
			elif line.startswith(('b', 'r')):
				m = re_Nb_val.match(line)
			value = m.group(1)
			code = m.group(2)
			if 'x' in value or 'z' in value:
				value = '0'
			
			if code in data:					# record time and value (once a time)
				
				if value == '1':				
					data[code] = time
					count += 1
				elif value == '0':
					temp = (int)(time) - (int)(data[code])
					Sum += temp
					if temp > MaxTime:
						MaxTime = temp
					i=0
					while i < len(elsetime):
						if temp > elsetime[i]:
							elsetime[i] = temp
							break
						i+=1
				
					
	

	print "Max: %f ns\n"%(MaxTime)
	for i in range(len(elsetime)):
		print "%d: %f ns\n"%(i,elsetime[i])
	print "Avg: %f ns\n"%(Sum/count)
######################################

'''
Main function
	for command line option
'''
def main():

	msg = "myParser.py [-h] [vcd_file] \n\t\t\t[-s]"
	parser = argparse.ArgumentParser(description='myVCDParser Script ver_0.1', usage=msg)
	parser.add_argument('vcd', nargs='?', action='store', help='The vcd file you want to parse.')
	parser.add_argument('-s', nargs='?', action='store', dest='searchstring', help='Show only the name of signals begin with string you input. NOTE: vcd file and rc file will not be generated.', metavar=('string'))
	parser.add_argument('-c', nargs='?', action='store', dest='comment')

	args = parser.parse_args()
	if args.vcd is None:
		print parser.print_help()
		parser.error("You must choose a vcd file .")
		return 
	if args.searchstring!=None:
		searchstring = args.searchstring
		tempGroup = searchstring.split('/')
		for tempString in tempGroup:
			s = re.match("^[a-zA-Z0-9_]*$",tempString)
			if s is None:
				parser.error("The string must be composed of letters and numbers and underscore")
				return 
		if searchstring.endswith('/'):
			parser.error("The end of string can't be slash.")
			return 
	else:
		searchstring = ''
		print parser.print_help()
		parser.error("You must enter a string.")
		return 
	if args.comment!=None:
		print args.comment
	if args.vcd!=None:
		vcd_file = args.vcd

		
		vcdGroup = vcd_file.split('.')
		if (vcdGroup is not None and vcdGroup[len(vcdGroup)-1] != "vcd") or (vcdGroup is None):
			parser.error("Only support vcd file input.")
			return
		
		parse_vcd(vcd_file, searchstring)		# vcd = record all time's signals
		


	return 
if __name__ == "__main__":
	main()
