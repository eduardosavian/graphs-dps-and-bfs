import sys

import utils as ut

def main(arg1, arg2):
    input = ut.read_file(arg1)
    matrix = ut.transform_matrix(input)
    ut.print_matrix(matrix)


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python main.py <arg1> <arg2>")
        sys.exit(1)
    else:
        main(sys.argv[1], sys.argv[2])