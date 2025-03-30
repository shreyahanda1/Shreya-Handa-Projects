#!/usr/bin/env python3
# Name: Shreya Handa (shanda1)
# Group Members: Architha Dhavala (adhavala), Arya Ashok (aashok)

class ProteinParam :
# These tables are for calculating:
#     molecular weight (aa2mw), along with the mol. weight of H2O (mwH2O)
#     absorbance at 280 nm (aa2abs280)
#     pKa of positively charged Amino Acids (aa2chargePos)
#     pKa of negatively charged Amino acids (aa2chargeNeg)
#     and the constants aaNterm and aaCterm for pKa of the respective termini
#  Feel free to move these to appropriate methods as you like

# As written, these are accessed as class attributes, for example:
# ProteinParam.aa2mw['A'] or ProteinParam.mwH2O

    aa2mw = {
        'A': 89.093,  'G': 75.067,  'M': 149.211, 'S': 105.093, 'C': 121.158,
        'H': 155.155, 'N': 132.118, 'T': 119.119, 'D': 133.103, 'I': 131.173, 
        'P': 115.131, 'V': 117.146, 'E': 147.129, 'K': 146.188, 'Q': 146.145,
        'W': 204.225,  'F': 165.189, 'L': 131.173, 'R': 174.201, 'Y': 181.189
        }

    mwH2O = 18.015
    aa2abs280= {'Y':1490, 'W': 5500, 'C': 125}

    aa2chargePos = {'K': 10.5, 'R':12.4, 'H':6}
    aa2chargeNeg = {'D': 3.86, 'E': 4.25, 'C': 8.33, 'Y': 10}
    aaNterm = 9.69
    aaCterm = 2.34

    def __init__ (self, protein):
        # ignore any spaces in the inputted protein and transform any lowercase letters to uppercase
        self.protein = protein.strip().upper()
        # initialze a dictionary with all valid amino acids and their values to 0
        # i use this dictionary later on to store the count of each amino acid in the protein sequence
        self.aa_dict = {"A" : 0, "C" : 0, "D" : 0, "E" : 0, "F" : 0, "G" : 0, "H" : 0, "I" : 0, "L" : 0, "K" : 0, 
                        "M" : 0, "N" : 0, "P" : 0, "Q" : 0, "R" : 0, "S" : 0, "T" : 0, "V" : 0, "Y" : 0, "W" : 0}
        # initialize the count of each amino acid here so that I can access it when doing calculations in other functions
        self.init_aaComposition()
        
    def init_aaComposition (self) :
        ''' This method finds the count of each valid amino acid in the inputted protein sequence.

        Example:
            input: VLSPADKTNVKAAW
            output: {"A" : 3, "C" : 0, "D" : 1, "E" : 0, "F" : 0, "G" : 0, "H" : 0, "I" : 0, "L" : 1, "K" : 2, 
                        "M" : 0, "N" : 1, "P" : 1, "Q" : 0, "R" : 0, "S" : 1, "T" : 1, "V" : 2, "Y" : 0, "W" : 1}

        '''
        # call the function aaCount and get the total amount of valid amino acids in the protein sequence. store the value in total
        total = self.aaCount()

        # look through each amino acid in the inputted protein
        for i in self.protein:
            # self.aa_dict contains all valid characters. if the amino acid is in there, that means it is a valid amino acid
            # if amino acid is not in self.aa_dict, then does nothing/ignores
            if i in self.aa_dict:
                # if a valid amino acid, increment that specific's amino acid's count by 1
                self.aa_dict[i] += 1

        # return self.aa_dict which now holds the count of each amino acid
        return self.aa_dict

    def aaCount (self):
        ''' This method will return a single integer count of valid amino acid characters found. Ignores spaces and invalid characters. 
        
        Example:
            input: VLSPADKTNVKAAW
            output: 14
        '''
        # intializes a counter that will hold the the count of all amino acid characters found
        count = 0
        # look through each amino acid in the inputted protein
        for i in self.protein:
            # self.aa_dict contains all valid characters. if the amino acid is in there, that means it is a valid amino acid
            # if amino acid is not in self.aa_dict, then does nothing/ignores
            if i in self.aa_dict:
                # incremenets the count because one more valid amino acid was found
                count += 1
        # returns the final count after the for loop is done looping through the entire sequence
        return count
        pass

    '''
    ORIGINAL PI (using a different approach)
    def pI (self):
        i = 0
        lst = []
        while i <= 14:
            lst.append(round(i, 2))
            i += 0.01

        # any charge calculated after will have to be smaller than infinity
        start = float('inf')
        # starting point, an initialization
        smallest_net = 0
        #smallest_net = self._charge_(start)
        for i in lst: 
            # it will have to be smaller than infinity
            if abs(self._charge_(i)) < abs(start):
                start = abs(self._charge_(i))
                smallest_net = i

        return round(smallest_net, 2)
        pass
    '''
    
    def pI(self, precision = 2):
        ''' This method finds the pH that yields a neutral net Charge that is closest to 0. It does so by the use of binary search. It has initial 
        low and high values and uses the _charge_ function to calculate their charges. After calculation of the midpoint, it checks if middle_charge is 
        equal to, greater than, or less than 0 in addition to whether the low charge is greater than or less than 0. It adjusts the boundaries of the 
        search tree accordingly. '''
        # set the lower and upper bounds for binary search
        low = 0.0
        high = 14.0
        # set the precision which is 10^-2 = 0.01
        prec = 10 ** (-precision)
    
        # calculate the charge for the lower and upper boundary
        # do this to check for zero crossing in the initial values. if it does, change the boundaries accordingly
        low_charge = self._charge_(low)
        high_charge = self._charge_(high)

        # ensures the difference between high and low is not less than 0.01
        while high - low > prec:
            # find the middle point of the binary search and its charge
            # this middle point is used later on in the code to adjust boundaries
            middle = (low + high) / 2
            middle_charge = self._charge_(middle)

            # if the middle_charge is zero the first round, then immediately return value
            if middle_charge == 0:
                return round(middle, precision)  
    
            # if zero is not found first time, use binary search to adjust the high and low and bounderies of where to look
            if middle_charge > 0 and low_charge > 0:
                # crossing from negative to postive hasn't occured yet, so move lower boundary up to start searching from that new range
                low = middle 
            # since low_charge is negative and middle_charge is positive, this means zero crossing must be somewhere in the middle of these two values
            elif middle_charge > 0 and low_charge < 0:
                #crossing from negative to positive has occured, so move high down to middle and search from that new range
                high = middle
            elif middle_charge < 0 and low_charge < 0:
                # crossing from negative to postive hasn't occured yet. both middle and low are less than 0, so move lower boundary up to start searching from that new range
                low = middle
            # since middle_charge is negative and low_charge is positive, this means zero crossing must be somewhere in the middle of these two values
            elif middle_charge < 0 and low_charge > 0:
                high = middle

        # round the final pI to 2 decimal places
        return round((high + low) / 2, precision)


    def aaComposition (self) :
        ''' Returns the count of each amino acid in a input sequence. Ignores any invalid amino acid letters and white spaces when counting. These
        numbers are later used to find the percentage/abundance of each in the input sequence. 

        Example:
            input: VLSPADKTNVKAAW
            output: {"A" : 3, "C" : 0, "D" : 1, "E" : 0, "F" : 0, "G" : 0, "H" : 0, "I" : 0, "L" : 1, "K" : 2, 
                        "M" : 0, "N" : 1, "P" : 1, "Q" : 0, "R" : 0, "S" : 1, "T" : 1, "V" : 2, "Y" : 0, "W" : 1}

        '''
        # returns self.aa_dict that is initialized in the init_aaComposition function above
        return self.aa_dict
        pass

    def _charge_ (self, pH):
        ''' Calculates the net charge on the inputted protein at a specific pH. _charge_ is used to calculate the pI. I used the function given above and implemented the math in Python
        coding language. '''
        # initialize varaibles to pos and neg to 0 to later add the formaula to them
        pos = 0
        neg = 0

        # go through and read each amino acid 
        for i in self.aa_dict:
            # if the amino is in self.aa2chargePos, apply the formaula to it
            # i want to find the sum of all of the amino acids and their required values, so do pos+= to add previous caluclations to new ones
            if i in self.aa2chargePos:
                pos += self.aa_dict[i] * (10 ** self.aa2chargePos[i]) / (10 ** self.aa2chargePos[i] + 10 ** pH)
            # if the amino is in self.aa2chargeNeg, apply the formaula to it
            # i want to find the sum of all of the amino acids and their required values, so do neg+= to add previous caluclations to new ones
            elif i in self.aa2chargeNeg: 
                neg += self.aa_dict[i] * (10 ** pH) / (10 ** self.aa2chargeNeg[i] + 10 ** pH)

        # apply Nterminus to the pos/basic value calculated with the formula above
        pos += 1 * (10 ** self.aaNterm) / (10 ** self.aaNterm + 10 ** pH)
        # apply Cterminus to the neg/acidic value calculated with the formula above
        neg += 1 * (10 ** pH) / (10 ** self.aaCterm + 10 ** pH)

        # find the net charge by subtracting pos - neg and return the value
        net = pos - neg
        return net
        pass

    # assigned Cystine == True because the code will otherwise produce a "missing 1 required positional argument" error
    def molarExtinction (self, Cystine = True):
        ''' This method finds the extinction coefficient which indicates how much light a protein absorbs at a certain wavelength. I use the 
        dictionary aa2abs280= {'Y':1490, 'W': 5500, 'C': 125} and formula given above to do so. I find the E_Y, E_W, and E_C by accessing their
        values in the dictionary aa2abs280. I find the N_Y, N_W, and N_C by accessign their values/counts in the self.aa_dict I created. I then 
        plug in their respective values in the formula (N_Y * E_Y) + (N_W * E_W) + (N_C * E_C). 
        
        Example:
            input: VLSPADKTNVKAAW
            output: 5500.00
        '''

        # find E_Y, E_W, and E_C by accessign their values in the dictionary aa2abs280
        E_Y = self.aa2abs280["Y"]
        E_W = self.aa2abs280["W"]
        # checks if Cystine is True/present, if it is get it's E_C value from the dictionary. if it's False/not present, set it to 0 so it doesn't have an effect
        if Cystine == True:
            E_C = self.aa2abs280["C"]
        else:
            E_C = 0

        # find the N_Y, N_W, and N_C by accessign their values/counts in the self.aa_dict I created above
        N_Y = self.aa_dict["Y"]
        N_W = self.aa_dict["W"]
        # checks if Cystine is True/present, if it is get it's N_C value from the dictionary. if it's False/not present, set it to 0 so it doesn't have an effect
        if Cystine == True:
            N_C = self.aa_dict["C"]
        else:
            N_C = 0

        # plug in calculated values in formula below. return the result
        molar = (N_Y * E_Y) + (N_W * E_W) + (N_C * E_C)
        return molar     
        pass

    # assigned Cystine == True because the code will otherwise produce a "missing 1 required positional argument" error
    def massExtinction (self, Cystine = True):
        ''' Calculate the Mass Extinction by taking the Molar Extinction coefficient and dividing by the molecularWeight of the corresponding protein.

        Example:
            input: VLSPADKTNVKAAW
            output: 3.67
        '''
        # calls the self.molecularWeight() to get the calculated molecular weight and store it into myMW
        myMW =  self.molecularWeight()
        # calls self.molarExtinction(), but checks if Cystine is True or False and takes that into consideration of the calculation
        return self.molarExtinction(Cystine) / myMW if myMW else 0.0

    def molecularWeight (self):
        '''
        This method calculates the molecular weight (MW) of the protein sequence. This is done by summing the weights of the individual amino acids 
        and excluding the waters that are released with peptide bond formation. I access my self.aa_dict to get the count of each amino acid as well as
        the self.aa2mw to get the molecular weight of each amino acid. I also access the self.mwH2O to subtract the waters that are released with peptide
        bond formation.

        Example:
            input: VLSPADKTNVKAAW
            output: 1499.7
        '''
        # initialize a weight varaible to 0
        weight = 0
        # loop through self.aa_dict.items() and assign the i variable to the amino acid and the count varaible to the count of each amino acid
        for i, count in self.aa_dict.items():
            # apply the formula given in the directions above. multiply the moelcular weight of the amino acid to its count
            # sum every iteration together
            weight += self.aa2mw[i] * count

        # find the number of peptide bonds in the protein sequence. this will just be the count-1
        if self.aaCount() > 0:
            num_pb = self.aaCount() - 1

        # subtract the waters that are released with each peptide bond by multiplying 
        num_pb *= self.mwH2O
        mw = weight - num_pb
        return mw
        pass


# Please do not modify any of the following.  This will produce a standard output that can be parsed
    
import sys
def main():
    inString = input('protein sequence?')
    while inString :
        myParamMaker = ProteinParam(inString)
        myAAnumber = myParamMaker.aaCount()
        print ("Number of Amino Acids: {aaNum}".format(aaNum = myAAnumber))
        print ("Molecular Weight: {:.1f}".format(myParamMaker.molecularWeight()))
        print ("molar Extinction coefficient: {:.2f}".format(myParamMaker.molarExtinction()))
        print ("mass Extinction coefficient: {:.2f}".format(myParamMaker.massExtinction()))
        print ("Theoretical pI: {:.2f}".format(myParamMaker.pI()))
        print ("Amino acid composition:")
        
        if myAAnumber == 0 : myAAnumber = 1  # handles the case where no AA are present 
        
        for aa,n in sorted(myParamMaker.aaComposition().items(), 
                           key= lambda item:item[0]):
            print ("\t{} = {:.2%}".format(aa, n/myAAnumber))
    
        inString = input('protein sequence?')

if __name__ == "__main__":
    main()

class NucParams:
    rnaCodonTable = {
    # RNA codon table
    # U
    'UUU': 'F', 'UCU': 'S', 'UAU': 'Y', 'UGU': 'C',  # UxU
    'UUC': 'F', 'UCC': 'S', 'UAC': 'Y', 'UGC': 'C',  # UxC
    'UUA': 'L', 'UCA': 'S', 'UAA': '-', 'UGA': '-',  # UxA
    'UUG': 'L', 'UCG': 'S', 'UAG': '-', 'UGG': 'W',  # UxG
    # C
    'CUU': 'L', 'CCU': 'P', 'CAU': 'H', 'CGU': 'R',  # CxU
    'CUC': 'L', 'CCC': 'P', 'CAC': 'H', 'CGC': 'R',  # CxC
    'CUA': 'L', 'CCA': 'P', 'CAA': 'Q', 'CGA': 'R',  # CxA
    'CUG': 'L', 'CCG': 'P', 'CAG': 'Q', 'CGG': 'R',  # CxG
    # A
    'AUU': 'I', 'ACU': 'T', 'AAU': 'N', 'AGU': 'S',  # AxU
    'AUC': 'I', 'ACC': 'T', 'AAC': 'N', 'AGC': 'S',  # AxC
    'AUA': 'I', 'ACA': 'T', 'AAA': 'K', 'AGA': 'R',  # AxA
    'AUG': 'M', 'ACG': 'T', 'AAG': 'K', 'AGG': 'R',  # AxG
    # G
    'GUU': 'V', 'GCU': 'A', 'GAU': 'D', 'GGU': 'G',  # GxU
    'GUC': 'V', 'GCC': 'A', 'GAC': 'D', 'GGC': 'G',  # GxC
    'GUA': 'V', 'GCA': 'A', 'GAA': 'E', 'GGA': 'G',  # GxA
    'GUG': 'V', 'GCG': 'A', 'GAG': 'E', 'GGG': 'G'  # GxG
    }
    dnaCodonTable = {key.replace('U','T'):value for key, value in rnaCodonTable.items()}

    def __init__ (self, inString=''):
        ''' Create empty dictionaries for the allowed nucleotides, codon keys (3-base), and codon values (1-letter). '''
        # initialize dictionary for allowed nucleotides. assign the values 0 to them as we will edit that value later on
        self.nucComp = {"A": 0, "C": 0, "G": 0, "T": 0, "U": 0, "N": 0}

        # add the rnaCodonTable values to an empty dictionary and assign the value 0 to them
        self.codonComp = {}
        for i in NucParams.rnaCodonTable.keys():
            self.codonComp[i] = 0

        # add the rnaCodonTable keys to an empty dictionary and assign the value 0 to them
        self.aaComp = {}
        for i in NucParams.rnaCodonTable.values():
            self.aaComp[i] = 0

    def addSequence (self, inSeq):
        ''' Find the counts of inSeq nucleotide bases, codons, and codons' AA composition. '''
        # loop through the nucleotide in inSeq. if nucleotide in self.nucleotide dictionary, increments its count by 1
        for i in inSeq:
            if i in self.nucComp:
                self.nucComp[i] += 1

        # we are looking at a RNA sequence so all T's need to be replaced by U's
        # turning all lower case letters into upper case to cross-match with the rnaCodonTable above
        inSeq = inSeq.upper().replace("T", "U")

        # ignore incomplete/lagging codon sequences in the end of the sequence. i will increment in counts of 3 after each iteration
        for i in range(0, len(inSeq), 3):
            # loop through inSeq and finds each 3-base codon. if codon in self.codon dictionary, increments its count by 1
            codon = inSeq[i:i+3]
            if codon in self.codonComp and "N" not in codon:
                self.codonComp[codon] += 1

            # uses the same parsed codon and finds its amino acid letter. if its aa in self.aa dictionary, increment its count by 1
            # the codon might not be in rnaCodonTable which is why I have put a parameter None. with this None, it does not assume that the codon is in rnaCodonTable and avoids errors
            aa = NucParams.rnaCodonTable.get(codon, None)
            if aa and aa in self.aaComp: # ignores if aa is None. avoids unecessary dictionary lookup
                self.aaComp[aa] += 1
        pass
        
    def aaComposition(self):
        ''' Returns the calculated AA composition from above. '''
        return self.aaComp
    def nucComposition(self):
        ''' Returns the calculated nucleotide composition from above. '''
        return self.nucComp
    def codonComposition(self):
        ''' Returns the calculated codon composition from above. '''
        return self.codonComp
    def nucCount(self):
        ''' Returns the sum of the count (values) of all valid nucleotides in inSeq. '''
        return sum(self.nucComp.values())
        
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
