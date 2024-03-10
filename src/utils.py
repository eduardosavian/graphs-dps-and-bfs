def read_file(filepath: str) -> str:
    with open(filepath, 'r') as f:
        return f.read()

def transform_matrix(input: str) -> list:
    input = input.removeprefix('<').removesuffix('>')

    matrix = []
    for i in range(len(input)):
        if input[i] == '{':
            row = []
            for j in range(i, len(input)):
                if input[j] == '}':
                    sla = input[i+1:j].split(',')
                    print(sla)
                    row.append(input[i+1:j].split(','))
                    i = j
                    break
            matrix.append(row)
    return matrix

def print_matrix(matrix: list) -> None:
    for row in matrix:
        print(row)