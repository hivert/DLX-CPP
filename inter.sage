import cppyy
std = cppyy.gbl.std
vct = std.vector[int]
V = lambda v : vct([int(i) for i in v])
cppyy.include('dlx_matrix.hpp')
cppyy.load_library('libdlx_matrix')
DLX = cppyy.gbl.DLX_backtrack
"""
sage: vv = vct([int(3),int(0),int(1),int(2)])
sage: list(DLX.inverse_perm(vv))
[1, 2, 3, 0]

sage: M = DLX.DLXMatrix(int(4))
sage: M.add_row(V([0,2]))
0
sage: M.add_row(V([1,2]))
1
sage: M.add_row(V([1,3]))
2
sage: matrix([[int(x) for x in M.row_dense(int(i))] for i in range(M.height())])
[1 0 1 0]
[0 1 1 0]
[0 1 0 1]
sage: M.search_iter()
True
sage: list(M.get_solution())
[0, 2]
sage: M.search_iter()
False
"""
