from sequenceAnalysis import NucParams, FastAreader
import sys

def main (halophileFileName = None, hyperthermophileFileName = None):
    ''' Program compares GC content, aaComposition and relative codon bias of 2 genomes. In this assignment, I will be comparing a halophile genome and a hyperthermophile genome. '''
      
    # call FastAreader function to read both the halophileFileName and hyperthermophileFileName contents
    halophileReader = FastAreader(halophileFileName)
    hyperthermophileReader = FastAreader(hyperthermophileFileName)

    # call the NucParams function to get all of the needed information like aaComposition, nucComposition, and codonComposition that I can use later on to compare the 2 genomes together
    halophileNuc = NucParams()
    hyperthermophileNuc = NucParams()

    # process the 2 genomes
    # find the counts of inSeq nucleotide bases, codons, and codons' AA composition
    for i, seq in halophileReader.readFasta() :
        halophileNuc.addSequence(seq)

    for i, seq in hyperthermophileReader.readFasta() :
        hyperthermophileNuc.addSequence(seq)

    # use the helper function to calculate the GC content of the 2 genomes
    halophileGC = gc(halophileNuc)
    hyperthermophileGC = gc(hyperthermophileNuc)

    # print out the found GC content of the Halophile genome and Hyperthermophile genome
    print(f'GC content of Halophile = {halophileGC:.1f}%')
    print(f'GC content of Hyperthermophile = {hyperthermophileGC:.1f}%')    
    # find and print out the difference in GC content for the Halophile and Hyperthermophile genomes. print out the absolute value as difffernce should not be negative
    print(f'Difference in GC content for Halophile and Hyperthermophile = {abs(halophileGC - hyperthermophileGC):.1f}%')
    print()

    # call the helper function aa which prints out the aa comparison between the 2 genomes
    aa(halophileNuc, hyperthermophileNuc)

    # call the helper function codon_bias which prints out the codon bias comparison between the 2 genomes
    codon_bias(halophileNuc, hyperthermophileNuc)

    
# helper function to calculate GC content
def gc (nuc):
    ''' Calculates the GC content for any genome sequence. '''
    # get the total count of nucleotides
    totalNuc = nuc.nucCount()

    # find the percentage of GC content in sequence by calculating its average  
    gc = ((nuc.nucComp["G"] + nuc.nucComp["C"]) / totalNuc) * 100
    return gc

# helper function to calculate aa comparison
def aa (nuc1, nuc2):
    ''' Computes the amino acid comparison for any 2 genomes. '''
    # get the aa composition for the halophileNuc and hyperthermophileNuc
    aa1 = nuc1.aaComposition()
    aa2 = nuc2.aaComposition()

    aa1Total = sum(aa1.values())
    aa2Total = sum(aa2.values())

    for i in aa1:
        # get the aa composition from both genomes for the same amino acid
        aa1Get = aa1[i]
        aa2Get = aa2.get(i, 0)

        # get difference in percentage
        aa1Percent = (aa1Get / aa1Total * 100)
        aa2Percent = (aa2Get / aa2Total * 100)

        # check for which is higher and lower and in which scenario
        # explicitly state this with the percentages, making it clearer to read
        diff = aa2Percent - aa1Percent
        if diff > 0:
            result = f'{abs(diff):.2f}% higher in Halophile'
        elif diff < 0:
            result = f'{abs(diff):.2f}% lower in Hyperthermophile'
        else:
             result = "the same in both"
    
    
        # print Halophile and Hyperthermophile for each amino acid and their difference
        print(f'{i}: Halophile = {aa1Get}, Hyperthermophile = {aa2Get}, Difference = {abs(aa2Get - aa1Get)}') 
        print(f'{result}')
        
    # checking for amino acids that are in Hyperthermophile but not Halophile
    for i in aa2:
        if i not in aa1:
            # get the aa composition from both genomes. if not in genome1, then will be 0
            aa1Get = 0
            aa2Get = aa2[i]
            # print Halophile and Hyperthermophile for each amino acid and their difference
            print(f'{i}: Halophile = {aa1Get}, Hyperthermophile = {aa2Get}, Difference = {aa1Get - aa2Get}')         
    print()

def codon_bias(nuc1, nuc2):
    ''' Computes the comparison between relative codon biases for both genomes. '''
    # sort codons in in both genomes
    # sort in alpha order, by Amino Acid    
    # sort by amino acid, then sort alphabetically
    # if codon is not found in table, will return "-"
    sortNuc1 = sorted(nuc1.codonComp.keys(), key = lambda x: (nuc1.rnaCodonTable.get(x, "-"), x))
    sortNuc2 = sorted(nuc2.codonComp.keys(), key = lambda x: (nuc2.rnaCodonTable.get(x, "-"), x))

    for i in range(len(sortNuc1)):
        # get the codons from both genomes
        codon1 = sortNuc1[i]
        codon2 = sortNuc2[i]

        # get the count of the codon from Halophile and Hyperthermophile
        # if codon is not found, then count is just 0
        count1 = nuc1.codonComp.get(codon1, 0)
        count2 = nuc2.codonComp.get(codon2, 0)

        # print out the relative codon biases from Halophile and Hyperthermophile and compare/find their differences
        print(f'{codon1}: Halophile = {count1}, Hyperthermophile = {count2}, Difference = {abs(count2 - count1)}') 

if __name__ == "__main__":
    main("haloVolc1_1-genes.fa", "testGenome.fa")