#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <chrono>

using namespace std;

struct Tarefa {
    string descricao;
    int prioridade;
    int prazo; // Em segundos
};

// Lista de tarefas e um mutex para sincronização
vector<shared_ptr<Tarefa>> listaTarefas;
mutex mtx;

// Função para adicionar uma nova tarefa
void adicionarTarefa(const string& descricao, int prioridade, int prazo) {
    lock_guard<mutex> lock(mtx);
    listaTarefas.push_back(make_shared<Tarefa>(Tarefa{descricao, prioridade, prazo}));
    cout << "Tarefa adicionada: " << descricao << " (Prioridade: " << prioridade << ")\n";
}

// Função para listar tarefas
void listarTarefas() {
    lock_guard<mutex> lock(mtx);
    if (listaTarefas.empty()) {
        cout << "Nenhuma tarefa pendente.\n";
        return;
    }

    cout << "\n--- Lista de Tarefas ---\n";
    for (const auto& tarefa : listaTarefas) {
        cout << "- " << tarefa->descricao << " | Prioridade: " << tarefa->prioridade 
             << " | Prazo: " << tarefa->prazo << "s\n";
    }
}

// Função para salvar tarefas em um arquivo
void salvarTarefas(const string& nomeArquivo) {
    lock_guard<mutex> lock(mtx);
    ofstream arquivo(nomeArquivo);
    if (!arquivo) {
        cerr << "Erro ao abrir o arquivo para salvar.\n";
        return;
    }

    for (const auto& tarefa : listaTarefas) {
        arquivo << tarefa->descricao << ";" << tarefa->prioridade << ";" << tarefa->prazo << "\n";
    }
    arquivo.close();
    cout << "Tarefas salvas em " << nomeArquivo << "\n";
}

// Função para carregar tarefas de um arquivo
void carregarTarefas(const string& nomeArquivo) {
    lock_guard<mutex> lock(mtx);
    ifstream arquivo(nomeArquivo);
    if (!arquivo) {
        cerr << "Erro ao abrir o arquivo para carregar.\n";
        return;
    }

    listaTarefas.clear();
    string descricao;
    int prioridade, prazo;

    while (getline(arquivo, descricao, ';') && arquivo >> prioridade && arquivo.ignore() && arquivo >> prazo) {
        listaTarefas.push_back(make_shared<Tarefa>(Tarefa{descricao, prioridade, prazo}));
        arquivo.ignore();
    }
    arquivo.close();
    cout << "Tarefas carregadas do arquivo " << nomeArquivo << "\n";
}

// Função para simular a execução de uma tarefa
void executarTarefa(shared_ptr<Tarefa> tarefa) {
    cout << "Executando: " << tarefa->descricao << " (Tempo estimado: " << tarefa->prazo << "s)...\n";
    this_thread::sleep_for(chrono::seconds(tarefa->prazo));
    cout << "Tarefa concluída: " << tarefa->descricao << "!\n";
}

// Função que gerencia a execução das tarefas em threads
void iniciarExecucao() {
    lock_guard<mutex> lock(mtx);
    vector<thread> threads;

    for (auto& tarefa : listaTarefas) {
        threads.emplace_back(executarTarefa, tarefa);
    }

    for (auto& t : threads) {
        if (t.joinable())
            t.join();
    }

    listaTarefas.clear(); // Limpa a lista após a execução
    cout << "Todas as tarefas foram concluídas.\n";
}

int main() {
    int opcao;
    string descricao;
    int prioridade, prazo;
    string arquivo = "tarefas.txt";

    carregarTarefas(arquivo);

    do {
        cout << "\n=== Gerenciador de Tarefas ===\n";
        cout << "1. Adicionar Tarefa\n2. Listar Tarefas\n3. Executar Tarefas\n4. Salvar e Sair\nEscolha: ";
        cin >> opcao;
        cin.ignore();

        switch (opcao) {
            case 1:
                cout << "Descrição da tarefa: ";
                getline(cin, descricao);
                cout << "Prioridade (1-5): ";
                cin >> prioridade;
                cout << "Prazo (segundos): ";
                cin >> prazo;
                adicionarTarefa(descricao, prioridade, prazo);
                break;

            case 2:
                listarTarefas();
                break;

            case 3:
                iniciarExecucao();
                break;

            case 4:
                salvarTarefas(arquivo);
                cout << "Saindo...\n";
                break;

            default:
                cout << "Opção inválida.\n";
        }

    } while (opcao != 4);

    return 0;
}