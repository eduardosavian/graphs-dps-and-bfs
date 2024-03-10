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
                    matrix.append(input[i+1:j].split(','))
                    i = j
                    break
    return matrix


def print_matrix(matrix: list) -> None:
    print("Matrix:")
    print(matrix)
    print("-------")
    for row in matrix:
        print(row)


def create_graph_matrix(matrix: list) -> list:
    graph = []
    charss = []

    for row in matrix:
        for col in row:
            if col not in charss:
                charss.append(col)

    charss = sorted(set(charss))
    for i in range(len(charss)):
        row = []
        for j in range(len(charss)):
            if i == j:
                row.append('X')
            else:
                row.append('0')
        graph.append(row)

    for row in matrix:
        for i in range(len(row)):
            for j in range(len(row)):
                if row[i] != row[j]:
                    graph[charss.index(row[i])][charss.index(row[j])] = '1'
    return graph