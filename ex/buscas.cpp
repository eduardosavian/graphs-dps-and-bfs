#include <iostream>
#include <vector>
#include <sstream>
#include <regex>
#include <stack>
#include <queue>

/*
    A=0
    B=1
    C=2
    D=3
    E=4
    F=5
    G=6
    H=7
    ...
*/

class matriz {
private:
    int not_finished(std::vector<int> const& visitados)
    {
        for (int i = 0; i < this->grau; i++) {
            // retornar a posicao ao invés de um boleano é intencional
            if (visitados[i] == 0) { return i; }
        }
        return -1;
    };
        

public:
    std::vector<std::vector<int>> at; 
    int grau;
    char direcao;

    matriz(int vertices=0, char dire='Y') : at(vertices, std::vector<int>(vertices)), grau(vertices), direcao(dire) {}

    static matriz preencher(int vertices) {
        char direcao;
        do {
            std::cout << "[Y/N] Direcionado? ";
            std::cin >> direcao;
        } while (direcao != 'n' && direcao != 'N' && direcao != 'y' && direcao != 'Y');

        std::regex pattern("(\\{[0-9],[0-9]\\})*");
        //std::string input = "N{A,E}{A,F}{A,I}{A,G}{B,G}{C,H}{D,H}{E,A}{E,F}{E,I}{F,A}{F,E}{F,I}{G,A}{G,B}{H,C}{H,D}{I,A}{I,E}{I,F}";
        //std::string input = "N{A,B}{A,D}{B,A}{B,E}{C,E}{C,H}{D,A}{D,F}{E,B}{E,C}{E,I}{F,D}{F,G}{G,E}{G,F}{H,C}{H,I}{I,E}{I,H}";
        //std::string input = "N{A,D}{A,E}{B,E}{B,C}{C,I}{C,H}{D,F}{E,G}{F,G}{G,I}{I,H}";
        //std::string input = "D{A,B}{A,E}{B,D}{C,A}{D,C}{D,E}{E,B}";
        //std::string input = "D{A,D}{D,E}{E,A}{C,G}{G,B}{G,F}{B,C}{F,C}";
        //std::string input = "{A,B}{A,D}{B,C}{C,G}{D,E}{D,F}{E,A}{E,B}{E,C}{E,G}{F,C}{G,B}{G,F}";
        
        std::cout << "EX: {A,B}{B,C}{C,A}\n>: ";
        std::string input;
        std::cin >> input;

        if (!std::regex_search(input, pattern)) {
            std::cout << ("Entrada invalida.\n");
            std::exit(1);
        }

        std::stringstream ss(input);
        char temp;
        matriz grafo(vertices, direcao);


        char i, j;
        int row, col;
        while (ss >> temp >> i >> temp >> j >> temp) {
            std::cout << "Adj: " << i << " e " << j << std::endl;
            row = i - 'A';
            col = j - 'A';
            if (row < 0 || row >= vertices || col < 0 || col >= vertices) {
                std::cout << ("Adjacentes invalidos.\n");
                exit(1);
            }
            grafo.at[row][col] = 1;
            if (direcao == 'N') {
                grafo.at[col][row] = 1; // Nao direcionado
            }
        }
        std::cout << std::endl;
        return grafo;
    }

    void exibir() {
        int r = 0;
        std::cout << "    ";
        for (int c = 0; c < this->grau; c++) {
            std::cout << char('A' + c) << " | ";
        }
        std::cout << std::endl;
        for (const auto& linha : at) {
            std::cout << char('A' + r++) << " | ";
            for (int elemento : linha) {
                std::cout << elemento << " | ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    /*
    input = "N{A,E}{A,F}{A,I}{A,G}{B,G}{C,H}{D,H}{E,A}{E,F}{E,I}{F,A}{F,E}{F,I}{G,A}{G,B}{H,C}{H,D}{I,A}{I,E}{I,F}"

    |    | A | B | C | D | E | F | G | H | I |
    |----|---|---|---|---|---|---|---|---|---|
    | A  |   |   |   | 1 | 1 | 1 |   |   | 1 |
    | B  |   |   |   |   |   |   | 1 |   |   |
    | C  |   |   |   |   |   |   |   | 1 |   |
    | D  |   |   |   |   |   |   |   | 1 |   |
    | E  | 1 |   |   |   |   | 1 |   |   | 1 |
    | F  | 1 |   |   |   | 1 |   |   |   | 1 |
    | G  | 1 | 1 |   |   |   |   |   |   |   |
    | H  |   |   | 1 | 1 |   |   |   |   |   |
    | I  | 1 |   |   | 1 |   |   |   | 1 |   |

    VISTA: A E F I G B C H D
    EMPILHA: A E F I G B C H D
    DESEMPILHA: I F E B G A D H C
    */
    void profundidade(char inicio) {
        auto visitados = std::vector<int>(this->grau);
        std::stack<int> stack;
        int letra = inicio - 'A';
        stack.push(letra); 
        visitados[letra] = 1; 
        std::cout << "Empilha: A\n";
        while (not_finished(visitados) != -1) {
            for (int adjacente = 0; adjacente < this->grau; adjacente++)
            {
                if (at[letra][adjacente] == 1 && visitados[adjacente] == 0) 
                {
                    std::cout << "Empilha: " << char('A' + adjacente) << std::endl;
                    visitados[adjacente] = 1;
                    stack.push(adjacente); 
                    letra = adjacente;
                    adjacente = 0;
                }
            }
            if (stack.size() != 0)
            {
                letra = stack.top();
                stack.pop();
                std::cout << "Desempilha: " << char('A' + letra) << std::endl;
            }
            else { 
                letra = not_finished(visitados); 
                visitados[letra] = 1;
                stack.push(letra);
                std::cout << "Empilha: " << char('A' + letra) << std::endl;
            }
        }
    }

 
    void amplitude(char inicio) {
        std::queue<int> fila;
        std::vector<int> visitados(this->grau);
        int contador = 1;
        int vertice = inicio - 'A';
        visitados[vertice] = 1;
        fila.push(vertice);
        std::cout << contador++ << ":" << char(vertice + 'A') << std::endl;
        
        while (not_finished(visitados) != -1) {
            for (int pos = 0; pos < this->grau; pos++) {
                if (at[vertice][pos] == 1 && visitados[pos] == 0) 
                {
                    visitados[pos] = 1;
                    fila.push(pos);
                    std::cout << contador++ << ":" << char(pos + 'A') << std::endl;
                }
            }
            if (fila.size() > 0) {
                vertice = fila.front();
                fila.pop();
            }
            else { // Se desconexo, pega o primeiro nao visitado
                vertice = not_finished(visitados);
                visitados[vertice] = 1;
                fila.push(vertice);
                std::cout << contador++ << ":" << char(vertice + 'A') << std::endl;
            }
        }
    }

    std::vector<int> fecho_transitivo(char inicio, bool direto = true) {
        if (direto) {
            std::cout << "Fecho transitivo direto\n\n";
        }
        else {
            std::cout << "Fecho transitivo inverso\n\n";
        }
        std::queue<int> fila;
        std::vector<int> visitados(this->grau);
        int vertice = inicio - 'A';
        visitados[vertice] = 1;
        fila.push(vertice);
        std::vector<int> niveis(this->grau, -1);
        niveis[vertice] = 0;
        std::vector<int> associacoes(this->grau, -1);
        associacoes[vertice] = 1;
        while (not_finished(visitados) != -1) {
            for (int pos = 0; pos < this->grau; pos++) {
                if ((direto ? at[vertice][pos] == 1 : at[pos][vertice] == 1) && visitados[pos] == 0)
                {
                    visitados[pos] = 1;
                    fila.push(pos);
                    niveis[pos] = associacoes[vertice]; // letra
                    associacoes[pos] = associacoes[vertice] + 1; // adjacentes
                }
            }
            if (fila.size() > 0) {
                vertice = fila.front();
                fila.pop();
            }
            else { // Se desconexo, pega o primeiro nao visitado
                break;
            }
        }
        for (int i = 0; i < niveis.size(); i++) {
            std::cout << char(i + 'A') << ": " << niveis[i];
            if (direto) {
                std::cout << std::endl;
            }
            else {
                std::cout << " ";
            }
        }
        std::cout << std::endl;
        return niveis;
    }

    void matriz_alancabilidade(bool direto = false) {
        std::vector<std::vector<int>> matriz(this->grau, std::vector<int>(this->grau));
        std::vector<int> temp;
        for (int i = 0; i < this->grau; i++) {
            temp = (fecho_transitivo(char(i + 'A'), direto));
            if (direto) {
                for (int j = 0; j < this->grau; j++) {
                    matriz[j][i] = temp[j];
                }
            }
            else {
                matriz[i] = temp;
            }
        }

        std::cout << "\n    ";
        for (int i = 0; i < this->grau; i++) {
            std::cout << char('A' + i) << " | ";
        }
        std::cout << std::endl;
        for (int i = 0; i < this->grau; i++) {
            std::cout << char('A' + i) << " | ";
            for (int j = 0; j < this->grau; j++) {
                std::cout << matriz[i][j] << " | ";
            }
            std::cout << std::endl;
        }
    }

    // fazer privado depois
    std::vector<int> fecho_transitivo_para_conectividade(char inicio, bool direto, std::vector<int> skip) {
        auto not_finished_lambda = [](std::vector<int> const& visitados, int grau)
        {
            // é diferente da implementacao anterior
            for (int i = 0; i < grau; i++) {
                if (visitados[i] != 1) { return true; }
            }
            return false;
        };

        if (direto) {
            std::cout << "Fecho transitivo direto\n\n";
        }
        else {
            std::cout << "Fecho transitivo inverso\n\n";
        }
        std::queue<int> fila;
        std::vector<int> visitados = skip;
        int vertice = inicio - 'A';
        visitados[vertice] = 1;
        fila.push(vertice);
        std::vector<int> niveis(this->grau, -1);
        niveis[vertice] = 0;
        std::vector<int> associacoes(this->grau, -1);
        associacoes[vertice] = 1;
        while (not_finished_lambda(visitados, this->grau) != -1) {
            for (int pos = 0; pos < this->grau; pos++) {
                if ((direto ? at[vertice][pos] == 1 : at[pos][vertice] == 1) && visitados[pos] == -1)
                {
                    visitados[pos] = 1;
                    fila.push(pos);
                    niveis[pos] = associacoes[vertice]; // letra
                    associacoes[pos] = associacoes[vertice] + 1; // adjacentes
                }
            }
            if (fila.size() > 0) {
                vertice = fila.front();
                fila.pop();
            }
            else { // Se desconexo, pega o primeiro nao visitado
                break;
            }
        }
        for (int i = 0; i < niveis.size(); i++) {
            std::cout << char(i + 'A') << ": " << niveis[i];
            if (direto) {
                std::cout << std::endl;
            }
            else {
                std::cout << " ";
            }
        }
        std::cout << std::endl;
        if (direto == false) { std::cout << std::endl; }
        return niveis;
    }

    void conexo_recursao(char inicio, std::vector<int> & skips) {
        // é confuso, mas assim, conjuntos sempre rezeta, skips nunca reseta, assim eu consigo ter o controler de quando acapar o loop
        auto fecho_direto = this->fecho_transitivo_para_conectividade(inicio, true, skips);
        auto fecho_direto_inverso = this->fecho_transitivo_para_conectividade(inicio, false, skips);

        std::vector<int> conjunto;

        for (int i = 0; i < this->grau; i++) {
            if (fecho_direto[i] != -1 && fecho_direto_inverso[i] != -1) {
                conjunto.push_back(i);
            }
        }
        if (conjunto.size() != 0) {
            std::cout << "Nao conexo. Subconjunto: ";

            for (auto e : conjunto) {
                std::cout << char(e + 'A') << " ";
                skips[e] = 1;
            }
            std::cout << std::endl << std::endl;

            for (int i = 0; i < this->grau; i++) {
                if (skips[i] == -1) {
                    conexo_recursao(char(i + 'A'), skips);
                }
            }
        }
        else {
            std::cout << "conexo\n";
        }        
    }
};


//fecho inverso se calcula por coluna no for

// verificar se ~e conexo: fecho transitivo direto de um vertice e fecho transitivo inverso. gerou 2 conjuntos, faz intercecção. Se o resultado for todos os vertices do grafico ~e conexo.

// quais conjuntos conexoes? pega os vertices que nao apareceram e faz o mesmo procedimento acima. At~e que tenha todos os vertices contidos em algum subgrafico

void interface() {
    auto isNumber = [](const std::string& str) {
        for (char c : str) {
            if (!std::isdigit(c)) {
                std::cout << " INVALIDO\n";
                return false;
            }
        }
        return true;
    };

    std::string entrada;
    do {
        std::cout << "N vertices: ";
        std::getline(std::cin, entrada);
    } while (!isNumber(entrada));

    int grau = std::stoi(entrada);

    matriz grafo = matriz::preencher(grau);
    std::cout << std::endl;
    grafo.exibir();
    /*
    enum opcoes {
        exibir = 0,
        adicionar_vertices = 1,
        remover_vertices = 2,
        adicionar_aresta = 3,
        remover_aresta = 4,
        busca_profundidade = 5,
        busca_largura = 6,
        fecho_direto = 7,
        fecho_inverso = 8,
        alcancabilidade = 9,
        conexividade = 10
    };
    */
    while(true) {
        std::cout << "-------------------------------\n";
        std::cout << "[ 0]: exibir\n";
        std::cout << "[ 1]: adicionar vertice\n";
        std::cout << "[ 2]: remover vertice\n";
        std::cout << "[ 3]: adicionar aresta\n";
        std::cout << "[ 4]: remover aresta\n";
        std::cout << "[ 5]: busca em profundidade\n";
        std::cout << "[ 6]: busca em amplitude\n";
        std::cout << "[ 7]: fecho transitivo direto\n";
        std::cout << "[ 8]: fecho transitivo inverso\n";
        std::cout << "[ 9]: matriz de alcancabilidade\n";
        std::cout << "[10]: Verificar conexividade\n";
        std::cout << "-------------------------------\n";
        std::cout << ">: ";
        std::cin >> entrada;

        if (!isNumber(entrada) && (std::stoi(entrada) > 0 && std::stoi(entrada) < 10)) { // aumentar o limite se aumentar as opcoes
            continue;
        }
        std::cout << std::endl;
        
        char input;
        switch (std::stoi(entrada)) { // fazer um ponteiro de funcao
        case 0: grafo.exibir();
        break;

        case 1: std::cout << "AINDA NAO IMPLEMENTADO\n"; break;

        case 2: std::cout << "AINDA NAO IMPLEMENTADO\n"; break;

        case 3:
            char vertice1, vertice2;
            while (true) {
                std::cout << "[A-" << char(grau + 'A' - 1) << "] Vertice 1: ";
                std::cin >> input;
                if (std::isupper(input) && input >= 'A' && input <= 'Z') {
                    vertice1 = input;
                    while (true) {
                        std::cout << "[A-" << char(grau + 'A' - 1) << "] Vertice 2: ";
                        std::cin >> input;
                        if (std::isupper(input) && input >= 'A' && input <= 'Z') {
                            vertice2 = input;
                            break;
                        }
                        std::cout << " INVALIDO";
                    }
                    break;
                }
                std::cout << " INVALIDO";
            }
            grafo.at[vertice1 - 'A'][vertice2 - 'A'] = 1;
            if (grafo.direcao == 'N' || grafo.direcao == 'n') {
                grafo.at[vertice2 - 'A'][vertice1 - 'A'] = 1;
            }


        break;

        case 4: std::cout << "AINDA NAO IMPLEMENTADO\n"; break;

        case 5: // colocar em funcao ou lambda
            while (true) {
                std::cout << "[A-" << char(grau + 'A' - 1) << "] Inicio: ";
                std::cin >> input;
                if (std::isupper(input) && input >= 'A' && input <= 'Z') {
                    grafo.profundidade(input); // so muda aqui
                    break;
                }
                std::cout << " INVALIDO";
            }
        break;

        case 6: // colocar em funcao ou lambda
            while (true) {
                std::cout << "[A-" << char(grau + 'A' - 1) << "] Inicio: ";
                std::cin >> input;
                if (std::isupper(input) && input >= 'A' && input <= 'Z') {
                    grafo.amplitude(input); // so muda aqui
                    break;
                }
                std::cout << " INVALIDO";
            }
        break;

        case 7: // colocar em funcao ou lambda
            while (true) {
                std::cout << "[A-" << char(grau + 'A' - 1) << "] Inicio: ";
                std::cin >> input;
                if (std::isupper(input) && input >= 'A' && input <= 'Z') {
                    grafo.fecho_transitivo(input, true); // so muda aqui
                    break;
                }
                std::cout << " INVALIDO";
            }
        break;

        case 8: // colocar em funcao ou lambda
            while (true) {
                std::cout << "[A-" << char(grau + 'A' - 1) << "] Inicio: ";
                std::cin >> input;
                if (std::isupper(input) && input >= 'A' && input <= 'Z') {
                    grafo.fecho_transitivo(input, false); // so muda aqui
                    break;
                }
                std::cout << " INVALIDO";
            }
        break;

        case 9: grafo.matriz_alancabilidade(); break;

        case 10: 
            std::vector<int> conjuntos_ja_visitados(grau, -1);
            while (true) {
                std::cout << "[A-" << char(grau + 'A' - 1) << "] Inicio: ";
                std::cin >> input;
                if (std::isupper(input) && input >= 'A' && input <= 'Z') {
                    grafo.conexo_recursao(input, conjuntos_ja_visitados); // so muda aqui
                    break;
                }
                std::cout << " INVALIDO";
            }
        break;
        }
    } 
}

int main() {    
    interface();
    //matriz grafo = matriz::preencher(4);
    return 0;
} 