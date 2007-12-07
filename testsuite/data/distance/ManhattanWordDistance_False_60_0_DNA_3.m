name = 'ManhattanWordDistance'
accuracy = 0
dm_train = [0, 62, 66, 54, 72, 58, 66, 50, 56, 54, 60;62, 0, 60, 60, 68, 46, 50, 70, 60, 40, 62;66, 60, 0, 58, 82, 48, 62, 60, 60, 54, 68;54, 60, 58, 0, 76, 58, 70, 54, 66, 60, 64;72, 68, 82, 76, 0, 72, 72, 74, 60, 62, 80;58, 46, 48, 58, 72, 0, 40, 64, 60, 42, 70;66, 50, 62, 70, 72, 40, 0, 72, 68, 48, 78;50, 70, 60, 54, 74, 64, 72, 0, 54, 64, 74;56, 60, 60, 66, 60, 60, 68, 54, 0, 60, 74;54, 40, 54, 60, 62, 42, 48, 64, 60, 0, 62;60, 62, 68, 64, 80, 70, 78, 74, 74, 62, 0]
reverse = 'False'
data_type = ''
dm_test = [56, 58;68, 64;74, 76;70, 66;62, 58;74, 60;76, 58;66, 68;66, 66;74, 44;64, 68]
data_train = ['ATCACTACTGCCCCACAGCTAGTTGCTTTGGCTGGCCGTCACCGTTCTGTACGATCGGAA', 'TTGTAGGGTGCTCGAGCATGTTCTTGGCATCGCTACATCGAGTCCTTAGATTCAAGCCTC', 'GTCCGAGAACGCGCGCTGGCATCCGAGTCAACAGAAGGCCTGCGTGAGCGGAGTTATGCG', 'AGATCTGCTAGTACGAGAAGGATTGCCACGCCACCTGGAAAACTTCTTTATAGCGCGCTT', 'TCCCCTCTGGGGGCCCCAGGTAAAACAATAAACCCACAATCCTTGTAATTCGTGTGGGTC', 'ATGCAACACTCCCGATCGACGCTGAGCCTAGAAGCAATTGTCGAGTCACTGCGTATGTGT', 'TCCGGGATGCTGTCGACCGAGTACTAAGTGCATTCTCATATCTCTGCAATGTCGACAATT', 'TTGGTTACTAAAAACGACAGATACCAGTGCCCGGAACTTGAAGCTTGCGGAACCGTCTAC', 'TTGTGGTTCCCAAGATACGTTGACAGGCGCAAGCGGTAAGCGTACTACCATCCCTGCTAC', 'GGCATCTGTCCTCGCTTGTATATAGGTACTCAGCTCGATGGTGGAACTGAGTGCCGACAT', 'CCATGAGCCTTCACGTGTGGCTTTCTTTAGCTCAGCCAGCTATGCCCGGAGGCTCTTTAT']
feature_class = 'string_complex'
seqlen = 60
data_test = ['AATACAAAAGCCGCCCCCACTCAGTCAGGGCCGGGCTTTCACGGTTCCCCACCGGCAGCT', 'CACTGGAGGGACTGGGCTAGTTCCGTGGAGGTCTCGTACTGACGTATAAATATCTTGTGT']
gap = 0
feature_obtain = 'Char'
alphabet = 'DNA'
data_class = 'dna'
order = 3
feature_type = 'Word'