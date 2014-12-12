
#
# This script computes for an octree node, the sub-nodes access list depending
# the direction of ray
#

import math
import random
import sys


def gridMasterSize(grid):
    return int(math.sqrt(len(grid)))

def canPutInGroup(grid_size, grid, x, y, v):
    group_x = int(math.floor(x / grid_size)) * grid_size
    group_y = int(math.floor(y / grid_size)) * grid_size

    for iy in range(group_y, group_y + grid_size):
        for ix in range(group_x, group_x + grid_size):
            i = iy * grid_size * grid_size + ix

            if grid[i] == v:
                return False

            elif grid[i] == None:
                return True

    return True

def canPutInRow(grid_size, grid, y, v):
    i_start = y * grid_size * grid_size

    for i in range(i_start, i_start + grid_size * grid_size):
        if grid[i] == v:
            return False

        elif grid[i] == None:
            return True

    return True

def canPutInColumn(grid_size, grid, x, v):
    for iy in range(grid_size * grid_size):
        i = iy * grid_size * grid_size + x

        if grid[i] == v:
            return False

        elif grid[i] == None:
            return True

    return True

def canPut(grid_size, grid, x, y, v):
    return (
        canPutInGroup(grid_size, grid, x, y, v) and
        canPutInRow(grid_size, grid, y, v) and
        canPutInColumn(grid_size, grid, x, v)
    )

def emptyGrid(master_grid_size):
    return [None for n in range(master_grid_size * master_grid_size)]

def assertHyperCube(grid):
    master_grid_size = gridMasterSize(grid)
    grid_size = int(math.sqrt(master_grid_size))

    for y in range(master_grid_size):
        for x in range(master_grid_size):
            assert not canPut(grid_size, grid, x, y, grid[x + y * master_grid_size])

def genHyperCube(grid_size):
    master_grid_size = grid_size * grid_size

    grid = emptyGrid(master_grid_size)

    i = 0
    for y in range(master_grid_size):
        for x in range(master_grid_size):
            for v in range(master_grid_size):
                if canPut(grid_size, grid, x, y, v):
                    grid[i] = v
                    break

            i += 1

    assertHyperCube(grid)

    return grid

def randomPermutation(size):
    prem = list()
    available = [i for i in range(size)]

    for i in range(size):
        r = random.randrange(size - i)
        prem.append(available[r])
        available[r] = available[-1-i]

    return prem

def gridTranspose(grid):
    master_grid_size = gridMasterSize(grid)

    new_grid = emptyGrid(master_grid_size)

    for y in range(master_grid_size):
        for x in range(master_grid_size):
            i = x + y * master_grid_size
            i_src = y + x * master_grid_size
            new_grid[i] = grid[i_src]

    assertHyperCube(new_grid)

    return new_grid

def gridPermutationX(grid):
    master_grid_size = gridMasterSize(grid)
    grid_size = int(math.sqrt(master_grid_size))

    grid1 = emptyGrid(master_grid_size)

    for column_group in range(grid_size):
        perm = randomPermutation(grid_size)

        for y in range(master_grid_size):
            for x in range(grid_size):
                i = x + column_group * grid_size + y * master_grid_size
                i_src = perm[x] + column_group * grid_size + y * master_grid_size
                grid1[i] = grid[i_src]

    assertHyperCube(grid1)

    perm = randomPermutation(grid_size)

    grid2 = emptyGrid(master_grid_size)

    for y in range(master_grid_size):
        for column_group in range(grid_size):
            for x in range(grid_size):
                i = x + column_group * grid_size + y * master_grid_size
                i_src = x + perm[column_group] * grid_size + y * master_grid_size
                grid2[i] = grid1[i_src]

    assertHyperCube(grid2)

    return grid2

def gridPermutation(grid):
    return gridPermutationX(gridTranspose(gridPermutationX(grid)))

def gridToBijection(grid):
    master_grid_size = gridMasterSize(grid)

    bijection_grid = emptyGrid(master_grid_size)

    for y in range(master_grid_size):
        for x in range(master_grid_size):
            i_src = x + y * master_grid_size
            i_dest = y + grid[i_src] * master_grid_size
            bijection_grid[i_dest] = x

    assertHyperCube(bijection_grid)

    return bijection_grid

def bijection2Dto1D(grid):
    biject

def printGrid(grid,aligned):
    master_grid_size = gridMasterSize(grid)

    t = '{:02}, '

    if not aligned:
        t = '{}, '

    i = 0
    for y in range(master_grid_size):
        for x in range(master_grid_size):
            print t.format(grid[i]),
            i += 1

        print ''

def printCompressedGrid(grid):
    master_grid_size = gridMasterSize(grid)

    i = 0
    j = 0
    v = 0
    for y in range(master_grid_size):
        for x in range(master_grid_size):
            v = v | (grid[i] << 8 * j)
            j += 1
            if j == 4:
                print '0x{:08x},'.format(v),
                j = 0
                v = 0

            i += 1

        print ''


if __name__ == '__main__':
    assert len(sys.argv) == 2
    grid_size = int(sys.argv[1])
    prim = genHyperCube(grid_size)
    grid = gridPermutation(prim)
    bijection_grid = gridToBijection(grid)

    print 'grid size = {} -> warp size = {}'.format(grid_size, grid_size * grid_size)
    print 'orthogonal latin hypercube'
    printGrid(grid, True)
    print '(warp id, thread id) -(bijection)-> (x[y], y)'
    printGrid(bijection_grid, False)
    printCompressedGrid(bijection_grid)
