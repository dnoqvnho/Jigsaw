import sys
import random

class Query:
    def __init__(self, selection, projection, ranges):
        self.selection = selection
        self.projection = projection
        self.ranges = ranges
        self.closed = [False]*len(ranges)

    def to_string(self):
        anum = len(self.ranges) / 2 ##Number of attributes
        out_str = ""
        
        ####Output the projection
        s = ['0']*anum
        for i in self.projection:
            s[i] = '1'
        out_str += ''.join(s) + " "
        
        ####Output the selection
        s = ['0']*anum
        for i in self.selection:
            s[i] = '1'
        out_str += ''.join(s) + " "

        ####Output the ranges
        for i in self.ranges:
            out_str += str(i) + " "

        ####Output the closed
        s = ['0']*len(self.closed)
        for i in range(0, len(self.closed)):
            if self.closed[i]:
                s[i] = '1'
        out_str += ''.join(s)
        return out_str

        


class QTemplate:
    ##The selection and projection are lists of IDs of attributes 
    ##The ranges is a 2*ANUM array to represent the value ranges of tempalte
    def __init__(self, selection, projection, ranges):
        self.selection = selection
        self.projection = projection
        self.ranges = ranges

    def __init__(self, anum, select_a_num, proj_a_num, ranges):
        self.ranges = ranges
        ##Random select proj_a_num attributes
        self.projection = random.sample(range(0, anum), proj_a_num)
        self.selection = random.sample(self.projection, select_a_num)
        self.projection.sort()
        self.selection.sort()


    def make_one_query(self, selectivity):
        s_anum = len(self.selection)
        sel_per_attribute = selectivity**(1/float(s_anum))
        q_ranges = self.ranges[:]
        for i in self.selection:
            sample = self.__sample_one_attribute__(i, sel_per_attribute)
            q_ranges[2*i: (2*i + 2)] = sample
        return Query(self.selection, self.projection, q_ranges)


    def __sample_one_attribute__(self, aid, selectivity):
        r = self.ranges[2*aid:(2*aid + 2)]

        sample_size = (r[1] - r[0]) * selectivity
        sample = [0] * 2

        while True:
            sample[0] = int(r[0] + (r[1] - r[0] - sample_size) * random.random())
            sample[1] = int(sample[0] + sample_size)
            if sample[0] >= r[0] and sample[1] <= r[1]:
                break
        return sample



QNUM_train = 100
QNUM_test= 10

ANUM = int(sys.argv[1])
TNUM = int(sys.argv[2])

ranges = [0, TNUM - 1] * ANUM
selectivity = (0.01, 0.1, 0.4, 1)

project_a_num = 16
select_a_num = 1
qt_num = 2
qt = [QTemplate(ANUM, select_a_num, project_a_num, ranges) for _ in range(qt_num)]

for q in qt:
    print q.projection

for i in qt:
    for j in qt:
        inter = set(i.projection).intersection(j.projection)
        print("{}\t".format(len(inter))),
    print("\n"),
        

for s in selectivity:
    ofile = open("{}/{}/query_train".format(sys.argv[3], s), 'w')
    for i in range(0, QNUM_train):
        t = qt[random.randint(0, len(qt) - 1)]
        q = t.make_one_query(s)
        ofile.write(q.to_string() + "\n")
    ofile.close()
    
    ofile = open("{}/{}/query_test".format(sys.argv[3], s), 'w')
    for i in range(0, QNUM_test):
        t = qt[random.randint(0, len(qt) - 1)]
        q = t.make_one_query(s)
        ofile.write(q.to_string() + "\n")
    ofile.close()
