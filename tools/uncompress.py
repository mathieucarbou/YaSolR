#!/usr/bin/python3

# Un-compress files compressed with https://file2raw.labrat.one/

import sys, getopt

def main(argv):
   inputFile = ''
   inputFile = ''
   opts, args = getopt.getopt(argv,"hi:o:")
   for opt, arg in opts:
      if opt == '-h':
         print ('uncompress.py -i <inputFile> -o <outputFile>')
         sys.exit()
      elif opt in ("-i"):
         inputFile = arg
      elif opt in ("-o"):
         outputFile = arg
   if inputFile == '' or outputFile == '':
      print ('uncompress.py -i <inputFile> -o <outputFile>')
      sys.exit(2)
   print ('Input: ', inputFile)
   print ('Output:', outputFile)
   with open(outputFile, 'wb') as fOut:
      with open(inputFile, 'r') as fIn:
         for line in fIn:
            if len(line) > 0 and line[0].isdigit():
               for word in line.strip().split(','):
                  if word != '':
                     fOut.write(int(word).to_bytes(1, byteorder='big'))

if __name__ == '__main__':
   main(sys.argv[1:])
