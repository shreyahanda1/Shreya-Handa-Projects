from sequenceAnalysis import NucParams, FastAreader
import sys

def main (fileName=None):
    ''' Finds the sequence length, GC content, and relative codon usage for the genome sequence. '''
    myReader = FastAreader(fileName) 
    myNuc = NucParams()
    for head, seq in myReader.readFasta() :
        myNuc.addSequence(seq)

    # get the total number of nucleotides in sequence
    totalNuc = myNuc.nucCount()
    # divide by one million because we are looking in the units Mb
    seqLength = totalNuc/1_000_000
    print(f'sequence length = {seqLength:.2f} Mb')
    print()

    # find the percentage of GC content in sequence by calculating its average  
    gc = ((myNuc.nucComp["G"] + myNuc.nucComp["C"]) / totalNuc) * 100
    print(f'GC content = {gc:.1f}%')
    print()
    
    # sort codons in alpha order, by Amino Acid    
    # sort by amino acid, then sort alphabetically
    # if codon is not found in table, will return "-"
    sort = sorted(myNuc.codonComp.items(), key = lambda x: (myNuc.rnaCodonTable.get(x[0], "-"), x[0]))
    
    # calculate relative codon usage for each codon and print
    for i, j in sort:
        if j > 0:
            # find the amino acid in the table
            aa = myNuc.rnaCodonTable[i]
            # find the codon frequency by dividing the count of the codon by the count of its amino acid
            # if amino acid is not found, then just divide by 1
            val = (j / myNuc.aaComp.get(aa, 1))

            # print in the correct format of spaces and significant figures
            print ('{:s} : {:s} {:5.1f} ({:6d})'.format(i, aa, val*100, j))
    
if __name__ == "__main__":
    main() # changed in order to use stdin