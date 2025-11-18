#include <iostream>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <string>
#include <algorithm>
#include <iomanip> 

using namespace std;

struct Produto {
    string nome;
    string categoria;
    double preco;
    int estoque;

    Produto() : nome(""), categoria(""), preco(0.0), estoque(0) {}
    Produto(string n, string c, double p, int e) : nome(std::move(n)), categoria(std::move(c)), preco(p), estoque(e) {}

    void exibir() const {
        cout << left << setw(20) << nome
             << " | " << left << setw(12) << categoria
             << " | R$ " << setw(8) << fixed << setprecision(2) << preco
             << " | Estoque: " << estoque << '\n';
    }
};

struct RegistroVenda {
    string produtoNome;
    int quantidade;
    double valorTotal;
    string nota; 

    RegistroVenda(string n, int q, double v, string note = "")
        : produtoNome(std::move(n)), quantidade(q), valorTotal(v), nota(std::move(note)) {}
};

class Inventario {
private:
    vector<Produto> produtos;
    set<string> categorias;
    map<string,int> contagemPorCategoria;
    map<string,double> valorPorCategoria;
    list<RegistroVenda> historicoVendas;

    int indiceDoProduto(const string& nome) const {
        for (size_t i = 0; i < produtos.size(); ++i) {
            if (produtos[i].nome == nome) return static_cast<int>(i);
        }
        return -1;
    }

    void atualizarMapsAoAdicionar(const Produto& p) {
        categorias.insert(p.categoria);
        contagemPorCategoria[p.categoria] += 1;
        valorPorCategoria[p.categoria] += p.preco * p.estoque;
    }

    void atualizarMapsAoRemoverEstoque(const Produto& p, int qtdRemovida) {
        valorPorCategoria[p.categoria] -= p.preco * qtdRemovida;
        if (valorPorCategoria[p.categoria] < 1e-9) valorPorCategoria[p.categoria] = 0.0;
    }

public:
    Inventario() = default;

    void adicionarProduto(const Produto& p) {
        int idx = indiceDoProduto(p.nome);
        if (idx >= 0) {
            Produto &ex = produtos[idx];
            valorPorCategoria[ex.categoria] -= ex.preco * ex.estoque;
            ex.estoque += p.estoque;
            ex.preco = p.preco;
            valorPorCategoria[ex.categoria] += ex.preco * ex.estoque;
            cout << "Produto existente atualizado: " << p.nome << " (novo estoque: " << ex.estoque << ")\n";
        } else {
            produtos.push_back(p);
            atualizarMapsAoAdicionar(p);
            cout << "Produto adicionado: " << p.nome << '\n';
        }
    }

    const Produto* buscarProduto(const string& nome) const {
        int idx = indiceDoProduto(nome);
        if (idx >= 0) return &produtos[idx];
        return nullptr;
    }

    vector<const Produto*> listarPorCategoria(const string& categoria) const {
        vector<const Produto*> resultado;
        for (const auto& p : produtos) {
            if (p.categoria == categoria) resultado.push_back(&p);
        }
        return resultado;
    }

    double calcularValorTotal() const {
        double total = 0.0;
        for (const auto& p : produtos) total += p.preco * p.estoque;
        return total;
    }

    bool registrarVenda(const string& nomeProduto, int quantidade, const string& nota = "") {
        if (quantidade <= 0) {
            cout << "Quantidade inválida para venda.\n";
            return false;
        }
        int idx = indiceDoProduto(nomeProduto);
        if (idx < 0) {
            cout << "Produto não encontrado: " << nomeProduto << '\n';
            return false;
        }
        Produto &p = produtos[idx];
        if (p.estoque < quantidade) {
            cout << "Estoque insuficiente para " << nomeProduto << ". Disponível: " << p.estoque << '\n';
            return false;
        }
        p.estoque -= quantidade;
        double valor = p.preco * quantidade;
        historicoVendas.emplace_back(p.nome, quantidade, valor, nota);
        atualizarMapsAoRemoverEstoque(p, quantidade);
        cout << "Venda registrada: " << quantidade << " x " << p.nome << " | Valor R$ " << fixed << setprecision(2) << valor << '\n';
        return true;
    }

    void exibirResumo() const {
        cout << "\n=== Resumo do Inventário ===\n";
        cout << "Produtos cadastrados: " << produtos.size() << '\n';
        cout << "Categorias únicas: " << categorias.size() << '\n';
        cout << "Valor total em estoque: R$ " << fixed << setprecision(2) << calcularValorTotal() << '\n';
        cout << "-------------------------------\n";
    }

    void listarTodosProdutos() const {
        cout << "\n--- Lista de Produtos ---\n";
        for (const auto& p : produtos) p.exibir();
    }

    void exibirEstatisticasPorCategoria() const {
        cout << "\n--- Estatísticas por Categoria ---\n";
        for (const auto& cat : categorias) {
            int cnt = 0;
            
            auto it = contagemPorCategoria.find(cat);
            if (it != contagemPorCategoria.end()) cnt = it->second;
            double val = 0.0;
            auto it2 = valorPorCategoria.find(cat);
            if (it2 != valorPorCategoria.end()) val = it2->second;
            cout << left << setw(12) << cat << " | Produtos distintos: " << setw(3) << cnt
                 << " | Valor em estoque: R$ " << fixed << setprecision(2) << val << '\n';
        }
    }

    void exibirHistoricoVendas() const {
        cout << "\n--- Histórico de Vendas ---\n";
        if (historicoVendas.empty()) { cout << "Nenhuma venda registrada.\n"; return; }
        for (const auto& r : historicoVendas) {
            cout << r.quantidade << " x " << left << setw(20) << r.produtoNome
                 << " | R$ " << setw(8) << fixed << setprecision(2) << r.valorTotal;
            if (!r.nota.empty()) cout << " | " << r.nota;
            cout << '\n';
        }
    }
};

int main() {
    Inventario inv;

    inv.adicionarProduto( Produto("Camiseta", "Roupas", 49.90, 20) );
    inv.adicionarProduto( Produto("Calça Jeans", "Roupas", 129.90, 10) );
    inv.adicionarProduto( Produto("Tênis Esportivo", "Calçados", 249.90, 5) );
    inv.adicionarProduto( Produto("Meias (par)", "Acessórios", 9.90, 50) );
    inv.adicionarProduto( Produto("Boné", "Acessórios", 39.90, 15) );

    inv.listarTodosProdutos();

    const Produto* p = inv.buscarProduto("Tênis Esportivo");
    if (p) {
        cout << "\nProduto encontrado:\n";
        p->exibir();
    }

    auto roupas = inv.listarPorCategoria("Roupas");
    cout << "\nProdutos na categoria 'Roupas':\n";
    for (const Produto* prod : roupas) prod->exibir();

    cout << "\nValor total do inventário: R$ " << fixed << setprecision(2) << inv.calcularValorTotal() << '\n';

    inv.registrarVenda("Camiseta", 3, "Cliente: João");
    inv.registrarVenda("Meias (par)", 10, "Promoção");
    inv.registrarVenda("Tênis Esportivo", 1);

    inv.exibirHistoricoVendas();
    inv.exibirResumo();
    inv.exibirEstatisticasPorCategoria();

    return 0;
}
