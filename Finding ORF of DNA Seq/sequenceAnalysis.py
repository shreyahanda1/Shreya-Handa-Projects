#!/usr/bin/env python3
# Name: Shreya Handa (shanda1)
# Group Members: Architha Dhavala (adhavala), Arya Ashok (aashok)

class CommandLine() :
    '''
    Handle the command line, usage and help requests.

    CommandLine uses argparse, now standard in 2.7 and beyond. 
    it implements a standard command line argument parser with various argument options,
    a standard usage and help.

    attributes:
    all arguments received from the commandline using .add_argument will be
    avalable within the .args attribute of object instantiated from CommandLine.
    For example, if myCommandLine is an object of the class, and requiredbool was
    set as an option using add_argument, then myCommandLine.args.requiredbool will
    name that option.
 
    '''
    
    def __init__(self, inOpts=None) :
        '''
        Implement a parser to interpret the command line argv string using argparse.
        '''
        
        import argparse
        self.parser = argparse.ArgumentParser(description = 'Program prolog - a brief description of what this thing does', 
                                             epilog = 'Program epilog - some other stuff you feel compelled to say', 
                                             add_help = True, #default is True 
                                             prefix_chars = '-', 
                                             usage = '%(prog)s [options] -option1[default] <input >output'
                                             )
        self.parser.add_argument('-lG', '--longestGene', action = 'store', nargs='?', const=True, default=False, help='longest Gene in an ORF')
        self.parser.add_argument('-mG', '--minGene', type=int, choices= (100,200,300,500,1000), default=100, action = 'store', help='minimum Gene length')
        self.parser.add_argument('-s', '--start', action = 'append', default = ['ATG'],nargs='?', 
                                 help='start Codon') #allows multiple list options
        self.parser.add_argument('-t', '--stop', action = 'append', default = ['TAG','TGA','TAA'],nargs='?', help='stop Codon') #allows multiple list options
        self.parser.add_argument('-v', '--version', action='version', version='%(prog)s 0.1')  
        if inOpts is None :
            self.args = self.parser.parse_args()
        else :
            self.args = self.parser.parse_args(inOpts)

import sys
class FastAreader :
    ''' 
    Define objects to read FastA files.
    
    instantiation: 
    thisReader = FastAreader ('testTiny.fa')
    usage:
    for head, seq in thisReader.readFasta():
        print (head,seq)
    '''
    def __init__ (self, fname=None):
        '''contructor: saves attribute fname '''
        self.fname = fname
            
    def doOpen (self):
        ''' Handle file opens, allowing STDIN.'''
        if self.fname is None:
            return sys.stdin
        else:
            return open(self.fname)
        
    def readFasta (self):
        ''' Read an entire FastA record and return the sequence header/sequence'''
        header = ''
        sequence = ''
        
        with self.doOpen() as fileH:
            
            header = ''
            sequence = ''
            
            # skip to first fasta header
            line = fileH.readline()
            while not line.startswith('>') :
                line = fileH.readline()
            header = line[1:].rstrip()

            for line in fileH:
                if line.startswith ('>'):
                    yield header,sequence
                    header = line[1:].rstrip()
                    sequence = ''
                else :
                    sequence += ''.join(line.rstrip().split()).upper()

        yield header,sequence

class OrfFinder:        
    ''' Calculates the ORF of DNA sequences by finding the start and stop codons.

    Input: .fa file that contiains DNA sequence
    Output: Coding frame (+ for top strand and - for bottom strand). Length of ORF. 
    '''

    # initialize needed components of the code
    # initialize start codons, stop codons, minimum ORF length (100 nucleotides), and an empty ORF list which will store the found ORFs
    def __init__(self, seq, start = None, stop = None, minORF = 100):
        self.seq = seq
        self.start = ["ATG"]
        self.stop = ["TAA","TGA","TAG"]
        self.minORF = minORF
        self.ORF = []

    # finding the reverse complement of the sequence -- this is needed when finding negative frames since DNA is double stranded
    def reverseComp(self):
        # a dictionary that holds the complements of all bases
        comp = {"A":"T", "T":"A", "G":"C", "C":"G"}
        # store the reverse complement in an empty list and join them together so they create the 3'-5' bottom of double strand
        reverse_comp = []
        for i in reversed(self.seq):
            reverse_comp.append(comp[i])
        # return the reverse of the strand
        return "".join(reverse_comp)

    def ORF_find(self):
        # initializes list to keep track of the start codon positions for every reading frame
        lst = [[0], [0], [0]]

        # loops and looks through the sequence for every 3 codons
        for i in range(0, len(self.seq) - 2):
            # gets 3 nucleotides, 1 codon
            codon = self.seq[i:i+3]
            # finds the reading frame, either 1, 2, or 3
            frame = i % 3

            # checks if the codon found is in the start codon list initialized ["ATG"]
            # if it is ["ATG"], then store the index of the start codon and its frame
            if codon in self.start: 
                lst[frame].append(i)

            # checks if the codon found is in the stop codon list initialized ["TAA","TGA","TAG"]
            elif codon in self.stop:
                # look through all the found start codons in the same frame
                while lst[frame]:
                    # get the first start codon recorded
                    start = lst[frame].pop(0)
                    # calculate the ORF length
                    length = i + 3 - start
                    # check that the length is greater than or equal to 100 nucleotides
                    # if it is, store the ORF output requirements, like frame, start position, stop position, and length
                    if length >= self.minORF:
                        self.ORF.append((f'+{frame + 1}', start + 1, i + 3, length))

        # do the same as above, but now for the reverse sequence
        reverseSeq = self.reverseComp()
        # loops and looks through the sequence for every 3 codons
        for i in range(0, len(reverseSeq) - 2):
            # gets 3 nucleotides, 1 codon
            codon = reverseSeq[i:i+3]
            # finds the reading frame, either 1, 2, or 3
            frame = i % 3
            # checks if the codon found is in the start codon list initialized ["ATG"]
            # if it is ["ATG"], then store the index of the start codon and its frame
            if codon in self.start: 
                lst[frame].append(i)
            # checks if the codon found is in the stop codon list initialized ["TAA","TGA","TAG"]
            elif codon in self.stop:
                # look through all the found start codons in the same frame
                while lst[frame]:
                    # get the first start codon recorded
                    start = lst[frame].pop(0)
                    # calculate the ORF length
                    length = i + 3 - start
                    # check that the length is greater than or equal to 100 nucleotides
                    # if it is, store the ORF output requirements, like frame, start position, stop position, and length
                    if length >= self.minORF:
                        self.ORF.append((f'-{frame + 1}', len(self.seq) - (i + 3) + 1, len(self.seq) - start, length))

    def ORF_final(self):
        # sort the found ORF's length in decreasing order, then start position
        # x[3] is the where the length is stored. the negative before x[3] sorts in descending
        # x[1] is where the start position is stored
        ORF = self.ORF
        ORF.sort(key = lambda x:(-x[3], x[1]))

        # create empty set for start and stop codons to avoid duplicates
        # tracks stop codons in the + strand
        seen_end = set()
        # tracks start codons in the - strand
        seen_start = set()

        # iterates throught the sorted ORF list
        for frame, start, stop, length in ORF:
            # if frame on the + strand, check if stop codon is already seen to avoid duplicates
            if frame.startswith("+") and stop not in seen_end:
                # if not seen, now mark as seen
                seen_end.add(stop)
                # print in the correct format syntax
                print(f"{frame} {start:>5d}..{stop:>5d} {length:>5d}")
            # if frame on the - strand, check if start codon is already seen to avoid duplicates
            if frame.startswith("-") and start not in seen_start:
                # if not seen, now mark as seen
                seen_start.add(start)
                # print in the correct format syntax
                print(f"{frame} {start:>5d}..{stop:>5d} {length:>5d}")