from sequenceAnalysis import CommandLine, FastAreader, OrfFinder

def main(inFile = None, options = None):
    '''
    Find some genes.  
    '''
    thisCommandLine = CommandLine(options)
    reader = FastAreader(inFile)    

    # loop through readFasta file where i is the header and j is the sequence
    for i, j in reader.readFasta():
        print(i)
        # thisCommandLine.args.longestGene is True if only the longest Gene is desired
        # thisCommandLine.args.start is a list of start codons
        # thisCommandLine.args.stop is a list of stop codons
        # thisCommandLine.args.minGene is the minimum Gene length to include
        ORF = OrfFinder(j, thisCommandLine.args.start, thisCommandLine.args.stop, thisCommandLine.args.minGene)
        # call function ORF_find() and ORF_final() to perform its functions
        ORF.ORF_find()
        ORF.ORF_final()

    ###### replace the code between comments.
    print (thisCommandLine.args)
    #######
    
if __name__ == "__main__":
    main(options = ['-mG=300', '-lG']) # delete this stuff if running from commandline