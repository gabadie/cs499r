
#
# This script computes for an octree node, the sub-nodes access list depending
# the direction of ray
#

def main(bitfield):
    for d in range(8):
        mask = 0

        for i in range(8):
            subnodeId = 0x7 & (i ^ (~d))

            if not bitfield:
                print '{},'.format(subnodeId),

            mask = mask | (subnodeId << (i * 4))

        if bitfield:
            print '0x{:08x},'.format(mask)

        else:
            print ''


if __name__ == '__main__':
    main(True)
