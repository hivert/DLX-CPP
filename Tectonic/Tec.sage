r"""
TecTiling by Exact Cover Problem

Example:

    +---+---+---+---+
    |     2     |   |
    +---+   +   +   +
    |   |       | 4 |
    +   +---+---+   +
    |       |       |
    +   +   +---+---+
    | 3     |       |
    +---+---+   +   +
    |       |     3 |
    +---+---+---+---+
"""

class TT(object):
    def __init__(self, blocks, hints):
        r"""
        sage: TT(["AAAB", "CAAB", "CCBB", "CCDD", "EEDD"], {})._sizeblocks
        {'A': 5, 'B': 4, 'C': 5, 'D': 4, 'E': 2}
        """
        if isinstance(blocks[0], str):
            blocks = [list(x) for x in blocks]
        self._blocks = blocks
        self._nrows = len(blocks)
        self._ncols = len(blocks[0])
        for b in self._blocks:
            assert(len(b) == self._ncols)

        bl = flatten(self._blocks)
        from collections import defaultdict
        sb = defaultdict(int)
        for l in bl:
            sb[l] += 1
        self._sizeblocks = dict(sb)

        self._hints = hints
        for ((r, c), l) in hints.items():
            assert(0 <= r < self._nrows)
            assert(0 <= c < self._ncols)
            assert(1 <= l <= self._sizeblocks[self._blocks[r][c]])
            # print(r, c, l, self._blocks[r][c])

    def update_hints(self, hints):
        r"""
        sage: T = T28_151.update_hints({})
        sage: print(T.to_string())
        +---+---+---+---+---+---+---+---+---+
        |   |           |       |           |
        +   +---+---+---+   +---+---+   +   +
        |       |   |       |       |       |
        +   +   +   +   +---+   +   +---+---+
        |       |   |   |   |           |   |
        +---+---+   +---+   +---+---+---+   +
        |   |       |       |       |       |
        +   +   +---+   +---+   +   +   +   +
        |   |   |       |           |       |
        +   +---+---+---+---+---+---+---+---+
        |   |           |       |       |   |
        +   +   +   +---+   +   +   +   +---+
        |   |       |           |       |   |
        +   +---+---+---+---+---+---+   +   +
        |   |       |               |   |   |
        +---+   +   +   +---+---+---+---+   +
        |   |       |   |   |           |   |
        +---+   +---+---+   +---+   +---+   +
        |   |   |               |   |       |
        +   +---+---+---+---+---+---+---+---+
        |               |                   |
        +---+---+---+---+---+---+---+---+---+
        """
        return TT(self._blocks, hints)

    def DLXoverlap_cols(self):
        res = []
        for r in range(self._nrows):
            for c in range(self._ncols):
                for rd, cd in [(+1, 0), (-1, +1), (0, +1), (+1, +1)]:
                    rn = r + rd; cn = c + cd
                    if 0 <= rn < self._nrows and 0 <= cn < self._ncols: 
                        bl = self._blocks[r][c]
                        bln = self._blocks[rn][cn]
                        if bl != bln:
                            overlap = min(self._sizeblocks[bl],
                                          self._sizeblocks[bln])
                            for i in range(1, overlap+1):
                                res.append("V_%i_%s%ix%i_%s%ix%i"%(
                                    i, bl, r, c, bln, rn, cn))
        return res

    @lazy_attribute
    def DLXcols(self):
        r"""
        sage: len(TT(["AAAB", "CAAB", "CCBB", "CCDD", "EEDD"], {}).DLXcols)
        146
        """
        res = []
        for r in range(self._nrows):
            for c in range(self._ncols):
                res.append("c_%i_%i"%(r, c))
        for b, sb in self._sizeblocks.items():
            for i in range(1, sb+1):
                res.append("b%s_%i"%(b, i))
        res += self.DLXoverlap_cols()
        for i in range(len(self._hints)):
            res.append("h_%i"%i)
        return res

    def block(self, r, c, default=None):
        try:
            return self._blocks[r][c]
        except IndexError:
            return default

    def to_string(self, fill=None):
        r"""
        sage: T = TT(["AAAB", "CAAB", "CCBB", "CCDD", "EEDD"],
        ....:        {(0, 1) : 2, (1, 3) : 4, (3, 0) : 3, (4, 3) : 3})
        sage: print(T.to_string())
        +---+---+---+---+
        |     2     |   |
        +---+   +   +   +
        |   |       | 4 |
        +   +---+---+   +
        |       |       |
        +   +   +---+---+
        | 3     |       |
        +---+---+   +   +
        |       |     3 |
        +---+---+---+---+
        sage: print(T.to_string({}))
        +---+---+---+---+
        |           |   |
        +---+   +   +   +
        |   |       |   |
        +   +---+---+   +
        |       |       |
        +   +   +---+---+
        |       |       |
        +---+---+   +   +
        |       |       |
        +---+---+---+---+
        sage: print(T.to_string({(1,2) : 'X', (1,3) : 'X'}))
        +---+---+---+---+
        |           |   |
        +---+   +   +   +
        |   |     X | X |
        +   +---+---+   +
        |       |       |
        +   +   +---+---+
        |       |       |
        +---+---+   +   +
        |       |       |
        +---+---+---+---+
        """
        if fill is None:
            fill = self._hints
        res = '+'
        for c in range(self._ncols):
            res += '---+'
        res += '\n'
        for r in range(self._nrows):
            res += '|'
            for c in range(self._ncols):
                res += ' ' + str(fill.get((r,c), ' ')) + ' '
                if self.block(r,c) == self.block(r, c+1):
                    res += ' '
                else:
                    res += '|'
            res += '\n+'
            for c in range(self._ncols):
                if self.block(r,c) == self.block(r+1, c):
                    res += '   +'
                else:
                    res += '---+'
            res += '\n'
        return res[:-1]

    def __repr__(self):
        return self.to_string(self._hints)

    def place_row(self, r, c, l, hint=None):
        r"""
        sage: T = TT(["AAAB", "CAAB", "CCBB", "CCDD", "EEDD"], {})
        sage: T.place_row(0, 0 ,3)
        ['c_0_0', 'bA_3', 'V_3_A0x0_C1x0']
        sage: T.place_row(2, 1, 5)
        ['c_2_1', 'bC_5', 'V_5_A1x1_C2x1', 'V_5_C2x1_A1x2']
        sage: T.place_row(2, 1, 3)
        ['c_2_1',
         'bC_3',
         'V_3_A1x1_C2x1',
         'V_3_C2x1_A1x2',
         'V_3_C2x1_B2x2',
         'V_3_C2x1_D3x2']
        """
        assert(0 <= r < self._nrows)
        assert(0 <= c < self._ncols)
        bl = self._blocks[r][c]
        assert(1 <= l <= self._sizeblocks[bl])
        if hint is not None:
            assert(0 <= hint < len(self._hints))

        res = []
        res.append("c_%i_%i"%(r, c))
        res.append("b%s_%i"%(bl, l))

        for rd, cd in [(+1, 0), (-1, +1), (0, +1), (+1, +1)]:
            rn = r + rd; cn = c + cd
            if 0 <= rn < self._nrows and 0 <= cn < self._ncols:
                bln = self._blocks[rn][cn]
                if bl != bln and l <= self._sizeblocks[bln]:
                    res.append("V_%i_%s%ix%i_%s%ix%i"%(
                        l, bl, r, c, bln, rn, cn))
            rn = r - rd; cn = c - cd
            if 0 <= rn < self._nrows and 0 <= cn < self._ncols:
                bln = self._blocks[rn][cn]
                if bl != bln and l <= self._sizeblocks[bln]:
                   res.append("V_%i_%s%ix%i_%s%ix%i"%(
                        l, bln, rn, cn, bl, r, c))
        if hint is not None:
            res.append("h_%i"%hint)
        return res

    def DLXrow2vect(self, row):
        r"""
        sage: T = TT(["AAAB"], {(0,1) : 2})
        sage: T.DLXrow2vect(['c_0_1', 'bA_3'])
        (0, 1, 0, 0, 0, 0, 1, 0, 0, 0)
        sage: T.DLXrow2vect(['c_0_3', 'bB_1', 'V_1_A0x2_B0x3'])
        (0, 0, 0, 1, 0, 0, 0, 1, 1, 0)
        sage: T.DLXrow2vect(['c_0_1', 'bA_2', 'h_0'])
        (0, 1, 0, 0, 0, 1, 0, 0, 0, 1)
        """
        assert(all(x in self.DLXcols for x in row))
        return tuple(int(cl in row) for cl in self.DLXcols)

    @lazy_attribute
    def DLXrows(self):
        res = []
        order = []
        for r in range(self._nrows):
            for c in range(self._ncols):
                for l in range(1, self._sizeblocks[self._blocks[r][c]]+1):
                    res.append(self.place_row(r, c, l, None))
                    order.append((r, c, l, None))
        for (i, hint) in enumerate(self._hints.items()):
            ((r, c), l) = hint
            res.append(self.place_row(r, c, l, i))
            order.append((r,c,l,i))
        res += [[x] for x in self.DLXoverlap_cols()]
        order += [None]*len(self.DLXoverlap_cols())
        return res, order

    @lazy_attribute
    def DLXmatrix(self):
        r"""
        sage: T = TT(["AAAB"], {(0,1) : 2})
        sage: T.DLXmatrix[0]
        [1 0 0 0 1 0 0 0 0 0]
        [1 0 0 0 0 1 0 0 0 0]
        [1 0 0 0 0 0 1 0 0 0]
        [0 1 0 0 1 0 0 0 0 0]
        [0 1 0 0 0 1 0 0 0 0]
        [0 1 0 0 0 0 1 0 0 0]
        [0 0 1 0 1 0 0 0 1 0]
        [0 0 1 0 0 1 0 0 0 0]
        [0 0 1 0 0 0 1 0 0 0]
        [0 0 0 1 0 0 0 1 1 0]
        [0 1 0 0 0 1 0 0 0 1]
        [0 0 0 0 0 0 0 0 1 0]
        sage: T.DLXmatrix[1]
        [(0, 0, 1, None),
         (0, 0, 2, None),
         (0, 0, 3, None),
         (0, 1, 1, None),
         (0, 1, 2, None),
         (0, 1, 3, None),
         (0, 2, 1, None),
         (0, 2, 2, None),
         (0, 2, 3, None),
         (0, 3, 1, None),
         (0, 1, 2, 0),
         None]
        """
        res, order = self.DLXrows
        return Matrix([self.DLXrow2vect(r) for r in res]), order

    def save_dance(self, filename):
        outfile=open(filename, "w")
        print(' '.join(self.DLXcols), file=outfile)
        for r in self.DLXrows[0]:
            print(' '.join(r), file=outfile)
        outfile.close()

    def call_external(self, opts = ["-2"]):
        r"""
        sage: T = TT(["AAAB"], {(0, 0) : 1})
        sage: S = T.call_external()
        Number of solutions: 2
        Number of choices: 6, Number of dances: 16
        Timings: parse = ... ns, solve = ... ns, output = ... ns, total = ... ns
        sage: for s in S: print(T.to_string(s))
        +---+---+---+---+
        | 1   2   3 | 1 |
        +---+---+---+---+
        +---+---+---+---+
        | 1   3   2 | 1 |
        +---+---+---+---+

        sage: S = T28_151.call_external()
        Number of solutions: 1
        Number of choices: 1723, Number of dances: 12242
        Timings: parse = ... ns, solve = ... ns, output = ... ns, total = ... ns
        sage: print(T28_151.to_string(S[0]))
        +---+---+---+---+---+---+---+---+---+
        | 3 | 1   3   2 | 1   2 | 1   2   5 |
        +   +---+---+---+   +---+---+   +   +
        | 4   2 | 4 | 5   4 | 5   4 | 3   4 |
        +   +   +   +   +---+   +   +---+---+
        | 1   5 | 1 | 3 | 2 | 3   1   2 | 1 |
        +---+---+   +---+   +---+---+---+   +
        | 2 | 3   2 | 4   5 | 4   5 | 4   5 |
        +   +   +---+   +---+   +   +   +   +
        | 1 | 5 | 1   3 | 1   3   2 | 3   2 |
        +   +---+---+---+---+---+---+---+---+
        | 4 | 3   4   5 | 2   5 | 1   5 | 1 |
        +   +   +   +---+   +   +   +   +---+
        | 5 | 1   2 | 3   1   4 | 3   2 | 3 |
        +   +---+---+---+---+---+---+   +   +
        | 3 | 4   5 | 4   2   5   1 | 4 | 1 |
        +---+   +   +   +---+---+---+---+   +
        | 1 | 2   1 | 3 | 1 | 4   2   3 | 5 |
        +---+   +---+---+   +---+   +---+   +
        | 5 | 3 | 5   4   2   3 | 1 | 4   2 |
        +   +---+---+---+---+---+---+---+---+
        | 2   4   1   3 | 1   4   2   3   5 |
        +---+---+---+---+---+---+---+---+---+
        """
        from subprocess import Popen, PIPE
        pipe = Popen(['../dancing'] + opts, stdout=PIPE, stdin=PIPE)
        pipe.stdin.write((' '.join(self.DLXcols)+'\n').encode())
        for r in self.DLXrows[0]:
            pipe.stdin.write((' '.join(r)+'\n').encode())
        pipe.stdin.close()

        M, L = self.DLXmatrix
        M = list(M)
        respipe = pipe.stdout
        rdsol = False
        sol = []
        res = []
        for line in respipe.readlines():
            line = line.decode()
            if line.startswith("%C"):
                continue
            if line.startswith("%T"):
                print(line[3 : -1])
                if line.startswith("%T Number of solutions: "):
                    nsol = int(line[24 : -1])
            elif line == "Solution:\n":
                rdsol = True
                sol = {}
            elif rdsol:
                if line == "End\n":
                    rdsol = False
                    res.append(sol)
                else:
                    row = [s for s in line.split() if not s.startswith("V_")]
                    if row:
                        r = c = l = None
                        for s in row:
                            if s.startswith('b'):
                                l = int(s.split('_')[1])
                            elif s.startswith('c'):
                                 r, c = (int(v) for v in s.split('_')[1:])
                        sol[(r, c)] = l
        respipe.close()
        return nsol, res

    def solve(self):
        r"""
        sage: T = TT(["AAAB"], {(0, 1) : 2})
        sage: T.solve()
        [{(0, 0): 1, (0, 1): 2, (0, 2): 3, (0, 3): 1}]
        """
        from sage.combinat.matrices.dlxcpp import AllExactCovers as AllCovers
        M, L = self.DLXmatrix
        res = list(AllCovers(M))
        return [self.DLXsol(sol) for sol in res]

    def DLXsol(self, sol):
        r"""
        sage: T = TT(["AAAB"], {(0, 1) : 2})
        sage: S = T.solve(); len(S)
        1
        sage: print(T.to_string(S[0]))
        +---+---+---+---+
        | 1   2   3 | 1 |
        +---+---+---+---+

        sage: T = TT(["AAAB"], {(0, 0) : 3})
        sage: S = T.solve(); len(S)
        1
        sage: print(T.to_string(S[0]))
        +---+---+---+---+
        | 3   1   2 | 1 |
        +---+---+---+---+

        sage: T = TT(["AAAB"], {(0, 2) : 1})
        sage: S = T.solve(); len(S)
        0

        sage: T = TT(["AAAA", "BBBB"], {(0, 0) : 1, (0, 1) : 2})
        sage: S = T.solve(); len(S)
        2
        sage: print(T.to_string(S[0]))
        +---+---+---+---+
        | 1   2   3   4 |
        +---+---+---+---+
        | 3   4   1   2 |
        +---+---+---+---+
        sage: print(T.to_string(S[1]))
        +---+---+---+---+
        | 1   2   4   3 |
        +---+---+---+---+
        | 4   3   1   2 |
        +---+---+---+---+

        sage: T = TT(["AAAB", "CAAB", "CCBB", "CCDD", "EEDD"],
        ....:        {(0, 1) : 2, (1, 3) : 4, (3, 0) : 3, (4, 3) : 3})
        sage: S = T.solve(); len(S)
        1
        sage: print(T.to_string(S[0]))
        +---+---+---+---+
        | 4   2   1 | 2 |
        +---+   +   +   +
        | 1 | 3   5 | 4 |
        +   +---+---+   +
        | 4   2 | 1   3 |
        +   +   +---+---+
        | 3   5 | 4   2 |
        +---+---+   +   +
        | 1   2 | 1   3 |
        +---+---+---+---+

        sage: S = T28_151.solve(); len(S)
        1
        sage: print(T28_151.to_string(S[0]))
        +---+---+---+---+---+---+---+---+---+
        | 3 | 1   3   2 | 1   2 | 1   2   5 |
        +   +---+---+---+   +---+---+   +   +
        | 4   2 | 4 | 5   4 | 5   4 | 3   4 |
        +   +   +   +   +---+   +   +---+---+
        | 1   5 | 1 | 3 | 2 | 3   1   2 | 1 |
        +---+---+   +---+   +---+---+---+   +
        | 2 | 3   2 | 4   5 | 4   5 | 4   5 |
        +   +   +---+   +---+   +   +   +   +
        | 1 | 5 | 1   3 | 1   3   2 | 3   2 |
        +   +---+---+---+---+---+---+---+---+
        | 4 | 3   4   5 | 2   5 | 1   5 | 1 |
        +   +   +   +---+   +   +   +   +---+
        | 5 | 1   2 | 3   1   4 | 3   2 | 3 |
        +   +---+---+---+---+---+---+   +   +
        | 3 | 4   5 | 4   2   5   1 | 4 | 1 |
        +---+   +   +   +---+---+---+---+   +
        | 1 | 2   1 | 3 | 1 | 4   2   3 | 5 |
        +---+   +---+---+   +---+   +---+   +
        | 5 | 3 | 5   4   2   3 | 1 | 4   2 |
        +   +---+---+---+---+---+---+---+---+
        | 2   4   1   3 | 1   4   2   3   5 |
        +---+---+---+---+---+---+---+---+---+
        """
        M, L = self.DLXmatrix
        M = list(M)
        res = [L[M.index(r)] for r in sol]
        res = [x for x in res if x is not None]
        return {(r, c) : l for (r, c, l, _) in res}

    def rand_sol(self):
        r"""
        sage: Tes = TT(["AAAB", "CAAB", "CCBB", "CCDD", "EEDD"], {})
        sage: P, S = Tes.rand_sol()
        """
        nsol, Sols = self.call_external(["-R", "-f"])
        if nsol == 0:
            raise ValueError
        Sol = Sols[0]
        print("Found one !!!")
        print(self.update_hints(Sol))
        def up_place(hints):
            return self.update_hints({(i, j) : Sol[(i, j)] for (i, j) in hints})
        places = Permutations(Sol.keys()).random_element()

        nhints = -1
        for i in range(len(places)-1, -1, -1):
            Tnew = up_place(places[:i])
            nsol = Tnew.call_external(["-0"])[0]
            if (nsol != 1):
                nhints = i
                break

        print("==================================== %i"%nhints)
        i = 0
        hints = places[:nhints+1]
        while (i < len(hints)):
            new_hints = hints[:]
            print(hints[i])
            del new_hints[i]
            Tnew = up_place(new_hints)
            print(Tnew)
            nsol = Tnew.call_external(["-0"])[0]
            if nsol == 1:
                print("Removed hints %i"%i)
                hints = new_hints
            else:
                i += 1
        return up_place(hints), up_place(Sol)

T28_151 = TT(["ABBBCCDDD",
              "AAECCFFDD",
              "AAECGFFFH",
              "IEEGGJJHH",
              "IEGGJJJHH",
              "IKKKLLMMN",
              "IKKLLLMMO",
              "IPPQQQQMO",
              "RPPQSTTTO",
              "UPSSSSTOO",
              "UUUUVVVVV"],
             {(0, 4) : 1, (0, 8) : 5,
              (1, 0) : 4, (1, 1) : 2,
              (3, 0) : 2, (3, 7) : 4, (3, 8) : 5,
              (6, 2) : 2, (6, 3) : 3,
              (8, 8) : 5,
              (10, 1): 4, (10, 2):1, (10, 8) : 5})

TNEW    = TT(["AAABCCDDD",
              "AABBCCEDD",
              "IIHBCEEFF",
              "IIHHHEFFF",
              "KIHMMNNNG",
              "KKKMMMNNG"],
             {})

TNEW2   = TT(["AAABCC",
              "AABBCC",
              "IIHBCE",
              "IIHHHE",
              "KIHMME",
              "KKKMMM"],
             {})

TNEW3   = TT(["AAACCC",
              "AABBCC",
              "IIHBBE",
              "IIHHHE",
              "KIHMME",
              "KKKMMM"],
             {})

TNEW4   = TT(["AAACCCF",
              "AABBCCE",
              "IIHBBEE",
              "IIHHHEG",
              "KIHMMEG",
              "KKKMMMG",
              "KXXXYYY"],
             {})

T26_96  = TT(["AABB",
              "AAAB",
              "CCCC",
              "DCEE",
              "DEEE"],
             {(0, 0) : 2, (0, 2) : 2, (2, 3) : 5, (4, 3): 5})
